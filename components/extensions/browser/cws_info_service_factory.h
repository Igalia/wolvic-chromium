// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_CWS_INFO_SERVICE_FACTORY_H_
#define COMPONENTS_EXTENSIONS_BROWSER_CWS_INFO_SERVICE_FACTORY_H_

#include "base/no_destructor.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class KeyedService;

namespace content {
class BrowserContext;
}

namespace components_extensions {

class CWSInfoService;

// Singleton that produces CWSInfoService objects, one for each active BrowserContext.
class CWSInfoServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static CWSInfoService* GetForBrowserContext(content::BrowserContext* browser_context);
  static CWSInfoServiceFactory* GetInstance();

  CWSInfoServiceFactory(const CWSInfoServiceFactory&) = delete;
  CWSInfoServiceFactory& operator=(const CWSInfoServiceFactory&) = delete;

 private:
  friend class base::NoDestructor<CWSInfoServiceFactory>;

  CWSInfoServiceFactory();
  ~CWSInfoServiceFactory() override = default;

  // BrowserContextKeyedServiceFactory:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
  bool ServiceIsCreatedWithBrowserContext() const override;
  bool ServiceIsNULLWhileTesting() const override;
  void RegisterProfilePrefs(
      user_prefs::PrefRegistrySyncable* registry) override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_CWS_INFO_SERVICE_FACTORY_H_
