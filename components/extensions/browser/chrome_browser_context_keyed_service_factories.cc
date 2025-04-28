// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/chrome_browser_context_keyed_service_factories.h"

namespace components_extensions {

void EnsureChromeBrowserContextKeyedServiceFactoriesBuilt() {
// TODO(mshin): Enable the below code whenever migrating each functionality
//   extensions::ActivityLog::GetFactoryInstance();
//   extensions::BookmarksApiWatcher::EnsureFactoryBuilt();
//   extensions::ChromeAppIconServiceFactory::GetInstance();
//   extensions::ChromeExtensionCookiesFactory::GetInstance();
//   extensions::CWSInfoServiceFactory::GetInstance();
//   extensions::ExtensionGarbageCollectorFactory::GetInstance();
//   extensions::ExtensionGCMAppHandler::GetFactoryInstance();
//   extensions::ExtensionManagementFactory::GetInstance();
//   extensions::ExtensionNotificationDisplayHelperFactory::GetInstance();
//   extensions::ExtensionSystemFactory::GetInstance();
//   extensions::ExtensionWebUIOverrideRegistrar::GetFactoryInstance();
//   extensions::IncognitoConnectability::EnsureFactoryBuilt();
//   extensions::InstallTrackerFactory::GetInstance();
//   extensions::InstallVerifierFactory::GetInstance();
//   extensions::MenuManagerFactory::GetInstance();
//   extensions::PermissionsUpdater::EnsureAssociatedFactoryBuilt();
// #if BUILDFLAG(ENABLE_PLUGINS)
//   extensions::PluginManager::GetFactoryInstance();
// #endif
//   extensions::WarningBadgeServiceFactory::GetInstance();
//   extensions::WebAuthenticationProxyRegistrarFactory::GetInstance();
//   extensions::WebAuthenticationProxyServiceFactory::GetInstance();
}

}  // namespace components_extensions
