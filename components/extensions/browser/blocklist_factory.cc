// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/blocklist_factory.h"
#include "components/extensions/browser/blocklist.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/browser/extensions_browser_client.h"

using content::BrowserContext;
using extensions::ExtensionPrefsFactory;

namespace components_extensions {

// static
Blocklist* BlocklistFactory::GetForBrowserContext(BrowserContext* context) {
  return static_cast<Blocklist*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
BlocklistFactory* BlocklistFactory::GetInstance() {
  static base::NoDestructor<BlocklistFactory> instance;
  return instance.get();
}

BlocklistFactory::BlocklistFactory()
    : BrowserContextKeyedServiceFactory(
          "Blocklist",
          BrowserContextDependencyManager::GetInstance()) {
  DependsOn(ExtensionPrefsFactory::GetInstance());
}

BlocklistFactory::~BlocklistFactory() = default;

std::unique_ptr<KeyedService>
BlocklistFactory::BuildServiceInstanceForBrowserContext(
    BrowserContext* context) const {
  return std::make_unique<Blocklist>();
}

}  // namespace components_extensions
