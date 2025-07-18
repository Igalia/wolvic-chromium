// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/test_extension_system.h"

#include <memory>
#include <utility>

#include "base/command_line.h"
#include "base/memory/ptr_util.h"
#include "components/extensions/browser/crx_installer.h"
#include "components/extensions/browser/extension_management.h"
#include "components/extensions/browser/extension_service.h"
#include "components/extensions/browser/shared_module_service.h"
#include "components/prefs/pref_service.h"
#include "components/services/unzip/content/unzip_service.h"
#include "components/services/unzip/in_process_unzipper.h"
#include "components/value_store/test_value_store_factory.h"
#include "components/value_store/testing_value_store.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/browser/management_policy.h"
#include "extensions/browser/quota_service.h"
#include "extensions/browser/state_store.h"
#include "extensions/browser/user_script_manager.h"
#include "services/data_decoder/data_decoder_service.h"

using content::BrowserContext;
using content::BrowserThread;

using extensions::AppSorting;
using extensions::ContentVerifier;
using extensions::Extension;
using extensions::ExtensionPrefs;
using extensions::ExtensionSet;
using extensions::ManagementPolicy;
using extensions::QuotaService;
using extensions::ServiceWorkerManager;
using extensions::StateStore;
using extensions::UserScriptManager;

namespace components_extensions {

namespace {

// A fake CWSInfoService for tests that utilize the test extension system and
// service infrastructure but do not depend on the actual functionality of the
// service.
// TODO(mshin): Enable the below code after migrating CWSInfoService
// class FakeCWSInfoService : public CWSInfoService {
//  public:
//   explicit FakeCWSInfoService(BrowserContext* browser_context) {}

//   explicit FakeCWSInfoService(const CWSInfoService&) = delete;
//   FakeCWSInfoService& operator=(const CWSInfoService&) = delete;
//   ~FakeCWSInfoService() override = default;

//   // CWSInfoServiceInterface:
//   std::optional<bool> IsLiveInCWS(const Extension& extension) const override;
//   std::optional<CWSInfo> GetCWSInfo(const Extension& extension) const override;
//   void CheckAndMaybeFetchInfo() override {}
//   void AddObserver(Observer* observer) override {}
//   void RemoveObserver(Observer* observer) override {}

//   // KeyedService:
//   // Ensure that the keyed service shutdown is a no-op.
//   void Shutdown() override {}
// };

// std::optional<bool> FakeCWSInfoService::IsLiveInCWS(
//     const Extension& extension) const {
//   return true;
// }

// std::optional<CWSInfoServiceInterface::CWSInfo> FakeCWSInfoService::GetCWSInfo(
//     const Extension& extension) const {
//   return CWSInfoServiceInterface::CWSInfo();
// }

// std::unique_ptr<KeyedService> BuildFakeCWSService(
//     content::BrowserContext* context) {
//   return std::make_unique<FakeCWSInfoService>(context);
// }

}  // namespace

TestExtensionSystem::TestExtensionSystem(BrowserContext* browser_context)
    : browser_context_(browser_context),
      store_factory_(new value_store::TestValueStoreFactory()),
      state_store_(new StateStore(browser_context_,
                                  store_factory_,
                                  StateStore::BackendType::RULES,
                                  false)),
      quota_service_(new QuotaService())
      // TODO(mshin): Enable the below code after migrating ChromeAppSorting
      /*,
      app_sorting_(new ChromeAppSorting(browser_context_))*/ {}

TestExtensionSystem::~TestExtensionSystem() = default;

void TestExtensionSystem::Shutdown() {
  if (extension_service_)
    extension_service_->Shutdown();
  in_process_data_decoder_.reset();
}

ExtensionService* TestExtensionSystem::CreateExtensionService(
    const base::CommandLine* command_line,
    const base::FilePath& install_directory,
    bool autoupdate_enabled,
    bool extensions_enabled) {
  return CreateExtensionService(command_line, install_directory,
                                base::FilePath(), autoupdate_enabled,
                                extensions_enabled);
}

ExtensionService* TestExtensionSystem::CreateExtensionService(
    const base::CommandLine* command_line,
    const base::FilePath& install_directory,
    const base::FilePath& unpacked_install_directory,
    bool autoupdate_enabled,
    bool extensions_enabled) {
  // TODO(mshin): Enable the below code after migrating CWSInfoServiceFactory
  // if (CWSInfoService::Get(browser_context_) == nullptr) {
  //   // Associate a dummy CWSInfoService with this browser context if necessary.
  //   CWSInfoServiceFactory::GetInstance()->SetTestingFactory(
  //       browser_context_, base::BindRepeating(&BuildFakeCWSService));
  // }
  management_policy_ = std::make_unique<ManagementPolicy>();
  management_policy_->RegisterProviders(
      ExtensionManagementFactory::GetForBrowserContext(browser_context_)
          ->GetProviders());
  extension_service_ = std::make_unique<ExtensionService>(
      browser_context_, command_line, install_directory, unpacked_install_directory,
      // TODO(mshin): Enable the below code after migrating Blocklist
      ExtensionPrefs::Get(browser_context_), /*Blocklist::Get(browser_context_),*/
      autoupdate_enabled, extensions_enabled, &ready_);

  unzip::SetUnzipperLaunchOverrideForTesting(
      base::BindRepeating(&unzip::LaunchInProcessUnzipper));
  in_process_data_decoder_ =
      std::make_unique<data_decoder::test::InProcessDataDecoder>();

  extension_service_->ClearProvidersForTesting();
  return extension_service_.get();
}

void TestExtensionSystem::CreateUserScriptManager() {
  user_script_manager_ = std::make_unique<UserScriptManager>(browser_context_);
}

ExtensionService* TestExtensionSystem::extension_service() {
  return extension_service_.get();
}

ManagementPolicy* TestExtensionSystem::management_policy() {
  return management_policy_.get();
}

void TestExtensionSystem::SetExtensionService(ExtensionService* service) {
  extension_service_.reset(service);
}

ServiceWorkerManager* TestExtensionSystem::service_worker_manager() {
  return nullptr;
}

UserScriptManager* TestExtensionSystem::user_script_manager() {
  return user_script_manager_.get();
}

StateStore* TestExtensionSystem::state_store() {
  return state_store_.get();
}

StateStore* TestExtensionSystem::rules_store() {
  return state_store_.get();
}

StateStore* TestExtensionSystem::dynamic_user_scripts_store() {
  return state_store_.get();
}

scoped_refptr<value_store::ValueStoreFactory>
TestExtensionSystem::store_factory() {
  return store_factory_;
}

QuotaService* TestExtensionSystem::quota_service() {
  return quota_service_.get();
}

AppSorting* TestExtensionSystem::app_sorting() {
  // TODO(mshin): Enable the below code after migrating ChromeAppSorting
  // return app_sorting_.get();
  return nullptr;
}

const base::OneShotEvent& TestExtensionSystem::ready() const {
  return ready_;
}

bool TestExtensionSystem::is_ready() const {
  return ready_.is_signaled();
}

ContentVerifier* TestExtensionSystem::content_verifier() {
  return content_verifier_.get();
}

std::unique_ptr<ExtensionSet> TestExtensionSystem::GetDependentExtensions(
    const Extension* extension) {
  return extension_service()->shared_module_service()->GetDependentExtensions(
      extension);
}

void TestExtensionSystem::InstallUpdate(
    const std::string& extension_id,
    const std::string& public_key,
    const base::FilePath& temp_dir,
    bool install_immediately,
    InstallUpdateCallback install_update_callback) {
  NOTREACHED();
}

void TestExtensionSystem::PerformActionBasedOnOmahaAttributes(
    const std::string& extension_id,
    const base::Value::Dict& attributes) {}

bool TestExtensionSystem::FinishDelayedInstallationIfReady(
    const std::string& extension_id,
    bool install_immediately) {
  NOTREACHED();
  return false;
}

value_store::TestingValueStore* TestExtensionSystem::value_store() {
  // These tests use TestingValueStore in a way that ensures it only ever mints
  // instances of TestingValueStore.
  return static_cast<value_store::TestingValueStore*>(
      store_factory_->LastCreatedStore());
}

// static
std::unique_ptr<KeyedService> TestExtensionSystem::Build(
    content::BrowserContext* browser_context) {
  return base::WrapUnique(new TestExtensionSystem(browser_context));
}

void TestExtensionSystem::RecreateAppSorting() {
  // TODO(mshin): Enable the below code after migrating ChromeAppSorting
  // app_sorting_ = std::make_unique<ChromeAppSorting>(browser_context_);
}

}  // namespace components_extensions
