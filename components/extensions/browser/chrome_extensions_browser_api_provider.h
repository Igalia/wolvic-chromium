// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_CHROME_EXTENSIONS_BROWSER_API_PROVIDER_H_
#define CHROME_BROWSER_EXTENSIONS_CHROME_EXTENSIONS_BROWSER_API_PROVIDER_H_

#include "extensions/browser/extensions_browser_api_provider.h"

namespace components_extensions {

class ChromeExtensionsBrowserAPIProvider : public extensions::ExtensionsBrowserAPIProvider {
 public:
  ChromeExtensionsBrowserAPIProvider();

  ChromeExtensionsBrowserAPIProvider(
      const ChromeExtensionsBrowserAPIProvider&) = delete;
  ChromeExtensionsBrowserAPIProvider& operator=(
      const ChromeExtensionsBrowserAPIProvider&) = delete;

  ~ChromeExtensionsBrowserAPIProvider() override;

  void RegisterExtensionFunctions(ExtensionFunctionRegistry* registry) override;
};

}  // namespace components_extensions

#endif  // CHROME_BROWSER_EXTENSIONS_CHROME_EXTENSIONS_BROWSER_API_PROVIDER_H_
