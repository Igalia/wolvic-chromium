// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/install_verifier_factory.h"

#include "components/extensions/browser/install_verifier.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/browser/extension_registry_factory.h"
#include "extensions/browser/extensions_browser_client.h"

using content::BrowserContext;
using extensions::ExtensionPrefs;
using extensions::ExtensionPrefsFactory;
using extensions::ExtensionRegistryFactory;
using extensions::ExtensionsBrowserClient;

namespace components_extensions {

// static
InstallVerifier* InstallVerifierFactory::GetForBrowserContext(
    BrowserContext* context) {
  return static_cast<InstallVerifier*>(
      GetInstance()->GetServiceForBrowserContext(
          ExtensionsBrowserClient::Get()->GetOriginalContext(context), true));
}

// static
InstallVerifierFactory* InstallVerifierFactory::GetInstance() {
  static base::NoDestructor<InstallVerifierFactory> instance;
  return instance.get();
}

InstallVerifierFactory::InstallVerifierFactory()
    : BrowserContextKeyedServiceFactory(
          "InstallVerifier",
          BrowserContextDependencyManager::GetInstance()) {
  DependsOn(ExtensionPrefsFactory::GetInstance());
  DependsOn(ExtensionRegistryFactory::GetInstance());
}

InstallVerifierFactory::~InstallVerifierFactory() = default;

std::unique_ptr<KeyedService>
InstallVerifierFactory::BuildServiceInstanceForBrowserContext(
    BrowserContext* context) const {
  return std::make_unique<InstallVerifier>(ExtensionPrefs::Get(context),
                                           context);
}

}  // namespace components_extensions
