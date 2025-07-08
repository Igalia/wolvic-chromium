// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/extension_system_impl.h"

#include <algorithm>
#include <memory>

#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string_tokenizer.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"
#include "components/extensions/browser/chrome_content_verifier_delegate.h"
#include "components/extensions/browser/component_loader.h"
#include "components/extensions/browser/crx_installer.h"
#include "components/extensions/browser/extension_management.h"
#include "components/extensions/browser/extension_service.h"
#include "components/extensions/browser/extension_system_factory.h"
#include "components/extensions/browser/install_verifier.h"
#include "components/extensions/browser/load_error_reporter.h"
#include "components/extensions/browser/shared_module_service.h"
#include "components/extensions/browser/unpacked_installer.h"
#include "components/value_store/value_store_factory_impl.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/url_data_source.h"
#include "extensions/browser/content_verifier/content_verifier.h"
#include "extensions/browser/extension_pref_store.h"
#include "extensions/browser/extension_pref_value_map.h"
#include "extensions/browser/extension_pref_value_map_factory.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_util.h"
#include "extensions/browser/quota_service.h"
#include "extensions/browser/service_worker_manager.h"
#include "extensions/browser/state_store.h"
#include "extensions/browser/updater/uninstall_ping_sender.h"
#include "extensions/browser/user_script_manager.h"
#include "extensions/common/constants.h"
#include "extensions/common/features/feature_channel.h"
#include "extensions/common/manifest_url_handlers.h"
#include "ui/message_center/public/cpp/notifier_id.h"

using extensions::AppSorting;
using extensions::ContentVerifier;
using extensions::Extension;
using extensions::ExtensionPrefs;
using extensions::ExtensionRegistry;
using extensions::ExtensionSet;
using extensions::ManagementPolicy;
using extensions::QuotaService;
using extensions::ServiceWorkerManager;
using extensions::StateStore;
using extensions::UninstallPingSender;
using extensions::UninstallReason;
using extensions::UserScriptManager;

namespace components_extensions {

namespace {

// Helper to serve as an UninstallPingSender::Filter callback.
UninstallPingSender::FilterResult ShouldSendUninstallPing(
    content::BrowserContext* context,
    const Extension* extension,
    UninstallReason reason) {
  ExtensionManagement* extension_management =
      ExtensionManagementFactory::GetForBrowserContext(context);
  if (extension && (extension->from_webstore() ||
                    extension_management->UpdatesFromWebstore(*extension))) {
    return UninstallPingSender::SEND_PING;
  }
  return UninstallPingSender::DO_NOT_SEND_PING;
}

}  // namespace

//
// ExtensionSystemImpl::Shared
//

ExtensionSystemImpl::Shared::Shared(content::BrowserContext* context) : browser_context_(context) {}

ExtensionSystemImpl::Shared::~Shared() = default;

void ExtensionSystemImpl::Shared::InitPrefs() {
  store_factory_ = base::MakeRefCounted<value_store::ValueStoreFactoryImpl>(
      browser_context_->GetPath());

  // Three state stores. Two stores, which contain declarative rules and dynamic
  // user scripts respectively, must be loaded immediately so that the
  // rules/scripts are ready before we issue network requests.
  state_store_ = std::make_unique<StateStore>(
      browser_context_, store_factory_, StateStore::BackendType::STATE, true);

  rules_store_ = std::make_unique<StateStore>(
      browser_context_, store_factory_, StateStore::BackendType::RULES, false);

  dynamic_user_scripts_store_ = std::make_unique<StateStore>(
      browser_context_, store_factory_, StateStore::BackendType::SCRIPTS, false);
}

void ExtensionSystemImpl::Shared::RegisterManagementPolicyProviders() {
  management_policy_->RegisterProviders(
      ExtensionManagementFactory::GetForBrowserContext(browser_context_)
          ->GetProviders());

  management_policy_->RegisterProvider(InstallVerifier::Get(browser_context_));
}

void ExtensionSystemImpl::Shared::InitInstallGates() {
  // TODO(mshin): Enable the below code after migrating UpdateInstallGate
  // update_install_gate_ = std::make_unique<UpdateInstallGate>(browser_context_);
  // extension_service_->RegisterInstallGate(
  //     ExtensionPrefs::DELAY_REASON_WAIT_FOR_IDLE, update_install_gate_.get());
  extension_service_->RegisterInstallGate(
      ExtensionPrefs::DELAY_REASON_WAIT_FOR_IMPORTS,
      extension_service_->shared_module_service());
}

void ExtensionSystemImpl::Shared::Init(bool extensions_enabled) {
  TRACE_EVENT0("browser,startup", "ExtensionSystemImpl::Shared::Init");
  const base::CommandLine* command_line =
      base::CommandLine::ForCurrentProcess();

  bool allow_noisy_errors =
      !command_line->HasSwitch(::switches::kNoErrorDialogs);
  LoadErrorReporter::Init(allow_noisy_errors);

  content_verifier_ = new ContentVerifier(
      browser_context_, std::make_unique<ChromeContentVerifierDelegate>(browser_context_));

  service_worker_manager_ = std::make_unique<ServiceWorkerManager>(browser_context_);

  user_script_manager_ = std::make_unique<UserScriptManager>(browser_context_);

  // Always auto update(not support guest/system profile()
  bool autoupdate_enabled = true;
  extension_service_ = std::make_unique<ExtensionService>(
      browser_context_, base::CommandLine::ForCurrentProcess(),
      browser_context_->GetPath().AppendASCII(extensions::kInstallDirectoryName),
      browser_context_->GetPath().AppendASCII(extensions::kUnpackedInstallDirectoryName),
      // TODO(mshin): Enable the below code after migrating Blocklist
      ExtensionPrefs::Get(browser_context_), /*Blocklist::Get(browser_context_),*/
      autoupdate_enabled, extensions_enabled, &ready_);

  uninstall_ping_sender_ = std::make_unique<UninstallPingSender>(
      ExtensionRegistry::Get(browser_context_),
      base::BindRepeating(&ShouldSendUninstallPing, browser_context_));

  // These services must be registered before the ExtensionService tries to
  // load any extensions.
  {
    InstallVerifier::Get(browser_context_)->Init();
    ChromeContentVerifierDelegate::VerifyInfo::Mode mode =
        ChromeContentVerifierDelegate::GetDefaultMode();
    if (mode >= ChromeContentVerifierDelegate::VerifyInfo::Mode::BOOTSTRAP) {
      content_verifier_->Start();
    }
    management_policy_ = std::make_unique<ManagementPolicy>();
    RegisterManagementPolicyProviders();
  }

  // Extension API calls require QuotaService, so create it before loading any
  // extensions.
  quota_service_ = std::make_unique<QuotaService>();

  bool skip_session_extensions = false;
  extension_service_->component_loader()->AddDefaultComponentExtensions(
      skip_session_extensions);

  // app_sorting_ = std::make_unique<ChromeAppSorting>(browser_context_);

  InitInstallGates();

  extension_service_->Init();

  // Make sure ExtensionSyncService is created.
  // TODO(mshin): Support sync
  // ExtensionSyncService::Get(browser_context_);

  // Make the chrome://extension-icon/ resource available.
  // TODO(mshin): Enable the below code after migrating ExtensionIconSource
  // content::URLDataSource::Add(browser_context_,
  //                             std::make_unique<ExtensionIconSource>(browser_context_));

  // Register the source for the chrome://extensions-internals page.
  // TODO(mshin): Enable the below code after migrating ExtensionsInternalsSource
  // content::URLDataSource::Add(
  //     browser_context_, std::make_unique<ExtensionsInternalsSource>(browser_context_));
}

void ExtensionSystemImpl::Shared::Shutdown() {
  if (content_verifier_.get()) {
    content_verifier_->Shutdown();
  }
  if (extension_service_) {
    extension_service_->Shutdown();
  }
}

ServiceWorkerManager* ExtensionSystemImpl::Shared::service_worker_manager() {
  return service_worker_manager_.get();
}

StateStore* ExtensionSystemImpl::Shared::state_store() {
  return state_store_.get();
}

StateStore* ExtensionSystemImpl::Shared::rules_store() {
  return rules_store_.get();
}

StateStore* ExtensionSystemImpl::Shared::dynamic_user_scripts_store() {
  return dynamic_user_scripts_store_.get();
}

scoped_refptr<value_store::ValueStoreFactory>
ExtensionSystemImpl::Shared::store_factory() const {
  return store_factory_;
}

ExtensionService* ExtensionSystemImpl::Shared::extension_service() {
  return extension_service_.get();
}

ManagementPolicy* ExtensionSystemImpl::Shared::management_policy() {
  return management_policy_.get();
}

UserScriptManager* ExtensionSystemImpl::Shared::user_script_manager() {
  return user_script_manager_.get();
}

QuotaService* ExtensionSystemImpl::Shared::quota_service() {
  return quota_service_.get();
}

AppSorting* ExtensionSystemImpl::Shared::app_sorting() {
  return app_sorting_.get();
}

ContentVerifier* ExtensionSystemImpl::Shared::content_verifier() {
  return content_verifier_.get();
}

//
// ExtensionSystemImpl
//

ExtensionSystemImpl::ExtensionSystemImpl(content::BrowserContext* context) : browser_context_(context) {
  shared_ = ExtensionSystemSharedFactory::GetForBrowserContext(context);

  if (!context->IsOffTheRecord()) {
    shared_->InitPrefs();
  }
}

ExtensionSystemImpl::~ExtensionSystemImpl() = default;

void ExtensionSystemImpl::Shutdown() {}

void ExtensionSystemImpl::InitForRegularProfile(bool extensions_enabled) {
  TRACE_EVENT0("browser,startup", "ExtensionSystemImpl::InitForRegularProfile");

  if (user_script_manager() || extension_service()) {
    return;  // Already initialized.
  }

  shared_->Init(extensions_enabled);
}

ExtensionService* ExtensionSystemImpl::extension_service() {
  return shared_->extension_service();
}

ManagementPolicy* ExtensionSystemImpl::management_policy() {
  return shared_->management_policy();
}

ServiceWorkerManager* ExtensionSystemImpl::service_worker_manager() {
  return shared_->service_worker_manager();
}

UserScriptManager* ExtensionSystemImpl::user_script_manager() {
  return shared_->user_script_manager();
}

StateStore* ExtensionSystemImpl::state_store() {
  return shared_->state_store();
}

StateStore* ExtensionSystemImpl::rules_store() {
  return shared_->rules_store();
}

StateStore* ExtensionSystemImpl::dynamic_user_scripts_store() {
  return shared_->dynamic_user_scripts_store();
}

scoped_refptr<value_store::ValueStoreFactory>
ExtensionSystemImpl::store_factory() {
  return shared_->store_factory();
}

const base::OneShotEvent& ExtensionSystemImpl::ready() const {
  return shared_->ready();
}

bool ExtensionSystemImpl::is_ready() const {
  return shared_->is_ready();
}

QuotaService* ExtensionSystemImpl::quota_service() {
  return shared_->quota_service();
}

AppSorting* ExtensionSystemImpl::app_sorting() {
  return shared_->app_sorting();
}

ContentVerifier* ExtensionSystemImpl::content_verifier() {
  return shared_->content_verifier();
}

std::unique_ptr<ExtensionSet> ExtensionSystemImpl::GetDependentExtensions(
    const Extension* extension) {
  return extension_service()->shared_module_service()->GetDependentExtensions(
      extension);
}

void ExtensionSystemImpl::InstallUpdate(
    const std::string& extension_id,
    const std::string& public_key,
    const base::FilePath& unpacked_dir,
    bool install_immediately,
    InstallUpdateCallback install_update_callback) {
  DCHECK(!install_update_callback.is_null());

  ExtensionService* service = extension_service();
  DCHECK(service);

  scoped_refptr<CrxInstaller> installer = CrxInstaller::CreateSilent(service);
  installer->set_delete_source(true);
  installer->AddInstallerCallback(std::move(install_update_callback));
  installer->set_install_immediately(install_immediately);
  installer->UpdateExtensionFromUnpackedCrx(extension_id, public_key,
                                            unpacked_dir);
}

void ExtensionSystemImpl::PerformActionBasedOnOmahaAttributes(
    const std::string& extension_id,
    const base::Value::Dict& attributes) {
  extension_service()->PerformActionBasedOnOmahaAttributes(extension_id,
                                                           attributes);
}

bool ExtensionSystemImpl::FinishDelayedInstallationIfReady(
    const std::string& extension_id,
    bool install_immediately) {
  ExtensionService* service = extension_service();
  DCHECK(service);
  return service->GetPendingExtensionUpdate(extension_id) &&
         service->FinishDelayedInstallationIfReady(extension_id,
                                                   install_immediately);
}

}  // namespace components_extensions
