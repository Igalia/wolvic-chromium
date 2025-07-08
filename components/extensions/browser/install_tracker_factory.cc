// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/install_tracker_factory.h"

#include "base/no_destructor.h"
#include "components/extensions/browser/install_tracker.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/browser/extension_system_provider.h"
#include "extensions/browser/extensions_browser_client.h"

using extensions::ExtensionsBrowserClient;
using extensions::ExtensionPrefs;
using extensions::ExtensionPrefsFactory;

namespace components_extensions {

// static
InstallTracker* InstallTrackerFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<InstallTracker*>(
      GetInstance()->GetServiceForBrowserContext(
          ExtensionsBrowserClient::Get()->GetOriginalContext(context), true));
}

InstallTrackerFactory* InstallTrackerFactory::GetInstance() {
  static base::NoDestructor<InstallTrackerFactory> instance;
  return instance.get();
}

InstallTrackerFactory::InstallTrackerFactory()
    : BrowserContextKeyedServiceFactory(
          "InstallTracker",
          BrowserContextDependencyManager::GetInstance()) {
  DependsOn(ExtensionsBrowserClient::Get()->GetExtensionSystemFactory());
  DependsOn(ExtensionPrefsFactory::GetInstance());
}

InstallTrackerFactory::~InstallTrackerFactory() = default;

std::unique_ptr<KeyedService>
InstallTrackerFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return std::make_unique<InstallTracker>(context,
                                          ExtensionPrefs::Get(context));
}

}  // namespace components_extensions
