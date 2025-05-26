// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_INSTALL_VERIFIER_FACTORY_H_
#define COMPONENTS_EXTENSIONS_BROWSER_INSTALL_VERIFIER_FACTORY_H_

#include "base/no_destructor.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "content/public/browser/browser_context.h"

namespace components_extensions {

class InstallVerifier;

class InstallVerifierFactory : public BrowserContextKeyedServiceFactory {
 public:
  InstallVerifierFactory(const InstallVerifierFactory&) = delete;
  InstallVerifierFactory& operator=(const InstallVerifierFactory&) = delete;

  static InstallVerifier* GetForBrowserContext(
      content::BrowserContext* context);
  static InstallVerifierFactory* GetInstance();

 private:
  friend base::NoDestructor<InstallVerifierFactory>;

  InstallVerifierFactory();
  ~InstallVerifierFactory() override;

  // BrowserContextKeyedServiceFactory implementation
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_INSTALL_VERIFIER_FACTORY_H_
