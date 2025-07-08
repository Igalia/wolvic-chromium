// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/error_console/error_console_factory.h"

#include "components/extensions/browser/error_console/error_console.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "extensions/browser/extension_registry_factory.h"
#include "extensions/browser/extensions_browser_client.h"

using content::BrowserContext;
using extensions::ExtensionRegistryFactory;
using extensions::ExtensionsBrowserClient;

namespace components_extensions {

// static
ErrorConsole* ErrorConsoleFactory::GetForBrowserContext(
    BrowserContext* context) {
  return static_cast<ErrorConsole*>(
      GetInstance()->GetServiceForBrowserContext(
          ExtensionsBrowserClient::Get()->GetOriginalContext(context), true));
}

// static
ErrorConsoleFactory* ErrorConsoleFactory::GetInstance() {
  static base::NoDestructor<ErrorConsoleFactory> instance;
  return instance.get();
}

ErrorConsoleFactory::ErrorConsoleFactory()
    : BrowserContextKeyedServiceFactory(
          "ErrorConsoleFactory",
          BrowserContextDependencyManager::GetInstance()) {
  DependsOn(ExtensionRegistryFactory::GetInstance());
}

ErrorConsoleFactory::~ErrorConsoleFactory() = default;

std::unique_ptr<KeyedService>
ErrorConsoleFactory::BuildServiceInstanceForBrowserContext(
    BrowserContext* context) const {
  return std::make_unique<ErrorConsole>(context);
}

}  // namespace components_extensions
