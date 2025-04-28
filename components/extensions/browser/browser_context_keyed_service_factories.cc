// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/browser_context_keyed_service_factories.h"

#include "components/extensions/browser/api/api_browser_context_keyed_service_factories.h"
#include "components/extensions/browser/chrome_browser_context_keyed_service_factories.h"

namespace components_extensions {

void EnsureBrowserContextKeyedServiceFactoriesBuilt() {
  EnsureChromeBrowserContextKeyedServiceFactoriesBuilt();
  EnsureApiBrowserContextKeyedServiceFactoriesBuilt();
}

}  // namespace chrome_extensions
