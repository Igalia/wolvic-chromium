// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/common/chrome_extensions_api_provider.h"

#include <string_view>

#include "components/extensions/common/api/api_features.h"
#include "components/extensions/common/api/generated_schemas.h"
#include "components/extensions/common/api/manifest_features.h"
#include "components/extensions/common/api/permission_features.h"
#include "components/extensions/common/chrome_manifest_handlers.h"
#include "components/grit/components_resources.h"
#include "extensions/common/features/json_feature_provider_source.h"
#include "extensions/common/permissions/permissions_info.h"

using extensions::FeatureProvider;
using extensions::JSONFeatureProviderSource;
using extensions::PermissionsInfo;

namespace components_extensions {

ChromeExtensionsAPIProvider::ChromeExtensionsAPIProvider() {}
ChromeExtensionsAPIProvider::~ChromeExtensionsAPIProvider() = default;

void ChromeExtensionsAPIProvider::AddAPIFeatures(FeatureProvider* provider) {
  extensions::AddChromeAPIFeatures(provider);
}

void ChromeExtensionsAPIProvider::AddManifestFeatures(
    FeatureProvider* provider) {
  extensions::AddChromeManifestFeatures(provider);
}

void ChromeExtensionsAPIProvider::AddPermissionFeatures(
    FeatureProvider* provider) {
  extensions::AddChromePermissionFeatures(provider);
}

void ChromeExtensionsAPIProvider::AddBehaviorFeatures(
    FeatureProvider* provider) {
  // Note: No chrome-specific behavior features.
}

void ChromeExtensionsAPIProvider::AddAPIJSONSources(
    JSONFeatureProviderSource* json_source) {
  // TODO(mshin): Support Resources
  json_source->LoadJSON(IDR_CHROME_EXTENSION_API_FEATURES);
}

bool ChromeExtensionsAPIProvider::IsAPISchemaGenerated(
    const std::string& name) {
  return extensions::api::ChromeGeneratedSchemas::IsGenerated(name);
}

std::string_view ChromeExtensionsAPIProvider::GetAPISchema(
    const std::string& name) {
  return extensions::api::ChromeGeneratedSchemas::Get(name);
}

void ChromeExtensionsAPIProvider::RegisterPermissions(
    PermissionsInfo* permissions_info) {
  // TODO(mshin): Enable the below code after migrating chrome_api_permissions
  // permissions_info->RegisterPermissions(
  //     chrome_api_permissions::GetPermissionInfos(),
  //     chrome_api_permissions::GetPermissionAliases());
}

void ChromeExtensionsAPIProvider::RegisterManifestHandlers() {
  RegisterChromeManifestHandlers();
}

}  // namespace components_extensions
