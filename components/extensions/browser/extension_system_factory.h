// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_SYSTEM_FACTORY_H_
#define COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_SYSTEM_FACTORY_H_

#include "base/no_destructor.h"
#include "components/extensions/browser/extension_system_impl.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "extensions/browser/extension_system_provider.h"

namespace extensions {
class ExtensionSystem;
}

namespace components_extensions {

// BrowserContextKeyedServiceFactory for ExtensionSystemImpl::Shared.
// Should not be used except by ExtensionSystem(Factory).
class ExtensionSystemSharedFactory : public BrowserContextKeyedServiceFactory {
 public:
  ExtensionSystemSharedFactory(const ExtensionSystemSharedFactory&) = delete;
  ExtensionSystemSharedFactory& operator=(const ExtensionSystemSharedFactory&) =
      delete;

  static ExtensionSystemImpl::Shared* GetForBrowserContext(
      content::BrowserContext* context);

  static ExtensionSystemSharedFactory* GetInstance();

 private:
  friend base::NoDestructor<ExtensionSystemSharedFactory>;

  ExtensionSystemSharedFactory();
  ~ExtensionSystemSharedFactory() override;

  // BrowserContextKeyedServiceFactory implementation:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
};

// BrowserContextKeyedServiceFactory for ExtensionSystemImpl.
// TODO(yoz): Rename to ExtensionSystemImplFactory.
class ExtensionSystemFactory : public extensions::ExtensionSystemProvider {
 public:
  ExtensionSystemFactory(const ExtensionSystemFactory&) = delete;
  ExtensionSystemFactory& operator=(const ExtensionSystemFactory&) = delete;

  // ExtensionSystem provider implementation:
  extensions::ExtensionSystem* GetForBrowserContext(
      content::BrowserContext* context) override;

  static ExtensionSystemFactory* GetInstance();

 private:
  friend base::NoDestructor<ExtensionSystemFactory>;

  ExtensionSystemFactory();
  ~ExtensionSystemFactory() override;

  // BrowserContextKeyedServiceFactory implementation:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
  bool ServiceIsCreatedWithBrowserContext() const override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_SYSTEM_FACTORY_H_
