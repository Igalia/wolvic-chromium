// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_INSTALL_TRACKER_FACTORY_H_
#define COMPONENTS_EXTENSIONS_BROWSER_INSTALL_TRACKER_FACTORY_H_

#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "content/public/browser/browser_context.h"

namespace base {
template <typename T>
class NoDestructor;
}

namespace components_extensions {

class InstallTracker;

class InstallTrackerFactory : public BrowserContextKeyedServiceFactory {
 public:
  InstallTrackerFactory(const InstallTrackerFactory&) = delete;
  InstallTrackerFactory& operator=(const InstallTrackerFactory&) = delete;

  static InstallTracker* GetForBrowserContext(content::BrowserContext* context);
  static InstallTrackerFactory* GetInstance();

 private:
  friend base::NoDestructor<InstallTrackerFactory>;

  InstallTrackerFactory();
  ~InstallTrackerFactory() override;

  // BrowserContextKeyedServiceFactory overrides:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_INSTALL_TRACKER_FACTORY_H_
