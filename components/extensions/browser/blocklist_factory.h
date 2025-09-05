// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_BLOCKLIST_FACTORY_H_
#define COMPONENTS_EXTENSIONS_BROWSER_BLOCKLIST_FACTORY_H_

#include "base/no_destructor.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace components_extensions {

class Blocklist;

class BlocklistFactory : public BrowserContextKeyedServiceFactory {
 public:
  static Blocklist* GetForBrowserContext(content::BrowserContext* context);

  BlocklistFactory(const BlocklistFactory&) = delete;
  BlocklistFactory& operator=(const BlocklistFactory&) = delete;

  static BlocklistFactory* GetInstance();

 private:
  friend base::NoDestructor<BlocklistFactory>;

  BlocklistFactory();
  ~BlocklistFactory() override;

  // BrowserContextKeyedServiceFactory
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_BLOCKLIST_FACTORY_H_
