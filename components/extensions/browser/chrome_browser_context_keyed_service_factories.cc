// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/chrome_browser_context_keyed_service_factories.h"

#include "components/extensions/browser/cws_info_service_factory.h"
#include "components/extensions/browser/extension_management.h"
#include "components/extensions/browser/extension_system_factory.h"
#include "components/extensions/browser/install_tracker_factory.h"
#include "components/extensions/browser/install_verifier_factory.h"
#include "components/extensions/browser/permissions_updater.h"

namespace components_extensions {

void EnsureChromeBrowserContextKeyedServiceFactoriesBuilt() {
// TODO(mshin): Enable the below code whenever migrating each functionality
//   extensions::ActivityLog::GetFactoryInstance();
//   extensions::BookmarksApiWatcher::EnsureFactoryBuilt();
//   extensions::ChromeAppIconServiceFactory::GetInstance();
//   extensions::ChromeExtensionCookiesFactory::GetInstance();
  CWSInfoServiceFactory::GetInstance();
//   extensions::ExtensionGarbageCollectorFactory::GetInstance();
//   extensions::ExtensionGCMAppHandler::GetFactoryInstance();
  ExtensionManagementFactory::GetInstance();
//   extensions::ExtensionNotificationDisplayHelperFactory::GetInstance();
  ExtensionSystemFactory::GetInstance();
//   extensions::ExtensionWebUIOverrideRegistrar::GetFactoryInstance();
//   extensions::IncognitoConnectability::EnsureFactoryBuilt();
  InstallTrackerFactory::GetInstance();
  InstallVerifierFactory::GetInstance();
//   extensions::MenuManagerFactory::GetInstance();
  PermissionsUpdater::EnsureAssociatedFactoryBuilt();
// #if BUILDFLAG(ENABLE_PLUGINS)
//   extensions::PluginManager::GetFactoryInstance();
// #endif
//   extensions::WarningBadgeServiceFactory::GetInstance();
//   extensions::WebAuthenticationProxyRegistrarFactory::GetInstance();
//   extensions::WebAuthenticationProxyServiceFactory::GetInstance();
}

}  // namespace components_extensions
