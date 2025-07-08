// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/wolvic_main_parts.h"

#include "base/files/file_util.h"
#include "base/path_service.h"
#include "content/public/common/result_codes.h"
#include "content/shell/browser/shell_devtools_manager_delegate.h"
#include "net/android/network_change_notifier_factory_android.h"
#include "net/base/network_change_notifier.h"
#include "wolvic/browser/mojo/wolvic_interface_registrar.h"
#include "wolvic/browser/webdata_services/web_data_service_factory.h"
#include "wolvic/wolvic_browser_context.h"
#include "wolvic/wolvic_browser_process.h"

#include "components/extensions/common/buildflags.h"
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
#include "components/extensions/browser/browser_context_keyed_service_factories.h"
#include "content/public/browser/child_process_security_policy.h"
#include "extensions/components/javascript_dialog_extensions_client/javascript_dialog_extension_client_impl.h"
#include "extensions/browser/browser_context_keyed_service_factories.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/constants.h"
#endif

namespace wolvic {

// TODO(jfernandez): Should define these constants in a separated file ?
namespace wolvic {
  const char kInitialProfile[] = "Default";
}

namespace {

base::FilePath GetInitialProfileDir() {
  base::FilePath profile_dir;
  base::PathService::Get(base::DIR_ANDROID_APP_DATA, &profile_dir);
  return profile_dir.AppendASCII(wolvic::kInitialProfile);
}

}

WolvicMainParts::WolvicMainParts() {}

WolvicMainParts::~WolvicMainParts() {}

int WolvicMainParts::PreEarlyInitialization() {
  net::NetworkChangeNotifier::SetFactory(
      new net::NetworkChangeNotifierFactoryAndroid());

  browser_process_ = std::make_unique<WolvicBrowserProcess>();

  return content::RESULT_CODE_NORMAL_EXIT;
}

int WolvicMainParts::PreCreateThreads() {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // chrome-extension:// URLs are safe to request anywhere, but may only
  // commit (including in iframes) in extension processes.
  content::ChildProcessSecurityPolicy::GetInstance()->RegisterWebSafeIsolatedScheme(
      extensions::kExtensionScheme, true);
#endif

  browser_process_->Init(browser_context_.get(), off_the_record_browser_context_.get());
  return content::RESULT_CODE_NORMAL_EXIT;
}

int WolvicMainParts::PreMainMessageLoopRun() {
  // Required before profile creation).
  PreProfileInit();

  set_browser_context(new WolvicBrowserContext(GetInitialProfileDir(), false));
  set_off_the_record_browser_context(new WolvicBrowserContext(GetInitialProfileDir(), true));
  content::ShellDevToolsManagerDelegate::StartHttpHandler(
      browser_context_.get());

  PostBrowserStart();

  return 0;
}

void WolvicMainParts::PreProfileInit() {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  javascript_dialog_extensions_client::InstallClient();
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)
}

void WolvicMainParts::EnsureBrowserContextKeyedServiceFactoriesBuilt() {
  WebDataServiceFactory::GetInstance();
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  components_extensions::EnsureBrowserContextKeyedServiceFactoriesBuilt();
  extensions::EnsureBrowserContextKeyedServiceFactoriesBuilt();
#endif
}

void WolvicMainParts::PostMainMessageLoopRun() {
  content::ShellDevToolsManagerDelegate::StopHttpHandler();
  browser_process_->StartTearDown();
}

void WolvicMainParts::PostBrowserStart() {
  LOG(WARNING) << "WolvicMainParts::PostBrowserStart --";

  RegisterWolvicJavaMojoInterfaces();
  EnsureBrowserContextKeyedServiceFactoriesBuilt();

#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  extensions::ExtensionSystem::Get(browser_context_.get())
      ->InitForRegularProfile(true);
#endif
}

void WolvicMainParts::set_browser_context(WolvicBrowserContext* context) {
  browser_context_.reset(context);
}

void WolvicMainParts::set_off_the_record_browser_context(
    WolvicBrowserContext* context) {
  off_the_record_browser_context_.reset(context);
}

}  // namespace wolvic
