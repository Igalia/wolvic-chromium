// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_COMMON_CHROME_EXTENSIONS_API_PROVIDER_H_
#define COMPONENTS_EXTENSIONS_COMMON_CHROME_EXTENSIONS_API_PROVIDER_H_

#include <string_view>

#include "extensions/common/extensions_api_provider.h"

namespace components_extensions {

class ChromeExtensionsAPIProvider : public extensions::ExtensionsAPIProvider {
 public:
  ChromeExtensionsAPIProvider();

  ChromeExtensionsAPIProvider(const ChromeExtensionsAPIProvider&) = delete;
  ChromeExtensionsAPIProvider& operator=(const ChromeExtensionsAPIProvider&) =
      delete;

  ~ChromeExtensionsAPIProvider() override;

  // ExtensionsAPIProvider:
  void AddAPIFeatures(extensions::FeatureProvider* provider) override;
  void AddManifestFeatures(extensions::FeatureProvider* provider) override;
  void AddPermissionFeatures(extensions::FeatureProvider* provider) override;
  void AddBehaviorFeatures(extensions::FeatureProvider* provider) override;
  void AddAPIJSONSources(extensions::JSONFeatureProviderSource* json_source) override;
  bool IsAPISchemaGenerated(const std::string& name) override;
  std::string_view GetAPISchema(const std::string& name) override;
  void RegisterPermissions(extensions::PermissionsInfo* permissions_info) override;
  void RegisterManifestHandlers() override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_COMMON_CHROME_EXTENSIONS_API_PROVIDER_H_
