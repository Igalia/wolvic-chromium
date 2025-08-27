// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/cws_info_service_factory.h"

#include "base/no_destructor.h"
#include "components/extensions/browser/cws_info_service.h"
#include "components/extensions/browser/extension_management.h"
#include "components/extensions/common/pref_names.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "content/public/browser/browser_context.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/browser/extension_registry_factory.h"

namespace components_extensions {

// static
CWSInfoService*
CWSInfoServiceFactory::GetForBrowserContext(
    content::BrowserContext* browser_context) {
  return static_cast<CWSInfoService*>(
      GetInstance()->GetServiceForBrowserContext(browser_context, /*create=*/true));
}

// static
CWSInfoServiceFactory* CWSInfoServiceFactory::GetInstance() {
  static base::NoDestructor<CWSInfoServiceFactory> instance;
  return instance.get();
}

CWSInfoServiceFactory::CWSInfoServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "CWSInfoService",
          BrowserContextDependencyManager::GetInstance()) {
  DependsOn(extensions::ExtensionPrefsFactory::GetInstance());
  DependsOn(extensions::ExtensionRegistryFactory::GetInstance());
}

std::unique_ptr<KeyedService>
CWSInfoServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  if (base::FeatureList::IsEnabled(kCWSInfoService) == false) {
    return nullptr;
  }
  return std::make_unique<CWSInfoService>(context);
}

bool CWSInfoServiceFactory::ServiceIsCreatedWithBrowserContext() const {
  return true;
}

bool CWSInfoServiceFactory::ServiceIsNULLWhileTesting() const {
  return true;
}

void CWSInfoServiceFactory::RegisterProfilePrefs(
    user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterTimePref(prefs::kCWSInfoTimestamp, base::Time());
  registry->RegisterTimePref(prefs::kCWSInfoFetchErrorTimestamp, base::Time());
}

}  // namespace components_extensions
