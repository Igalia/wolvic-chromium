// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/chrome_extensions_browser_api_provider.h"

#include "components/extensions/browser/api/generated_api_registration.h"
#include "extensions/browser/extension_function_registry.h"

namespace components_extensions {

ChromeExtensionsBrowserAPIProvider::ChromeExtensionsBrowserAPIProvider() =
    default;
ChromeExtensionsBrowserAPIProvider::~ChromeExtensionsBrowserAPIProvider() =
    default;

void ChromeExtensionsBrowserAPIProvider::RegisterExtensionFunctions(
    ExtensionFunctionRegistry* registry) {
  // Preferences.
  // TODO(mshin): Enable the below code after migrating Preference
  // registry->RegisterFunction<GetPreferenceFunction>();
  // registry->RegisterFunction<SetPreferenceFunction>();
  // registry->RegisterFunction<ClearPreferenceFunction>();

  // Generated APIs from Chrome.
  extensions::api::ChromeGeneratedFunctionRegistry::RegisterAll(registry);
}

}  // namespace components_extensions
