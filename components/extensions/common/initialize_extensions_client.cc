// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/common/initialize_extensions_client.h"

#include <memory>

#include "base/no_destructor.h"
#include "components/extensions/common/chrome_extensions_client.h"
#include "extensions/common/extensions_client.h"

namespace components_extensions {

namespace {

void EnsureExtensionsClientInitialized(
    extensions::Feature::FeatureDelegatedAvailabilityCheckMap
        delegated_availability_map) {
  static bool initialized = false;

  static base::NoDestructor<ChromeExtensionsClient> extensions_client;

  if (!initialized) {
    initialized = true;
    extensions_client->SetFeatureDelegatedAvailabilityCheckMap(
        std::move(delegated_availability_map));
    extensions::ExtensionsClient::Set(extensions_client.get());
  }

  // ExtensionsClient::Set() will early-out if the client was already set, so
  // this allows us to check that this was the only site setting it.
  DCHECK_EQ(extensions_client.get(), extensions::ExtensionsClient::Get())
      << "ExtensionsClient should only be initialized through "
      << "EnsureExtensionsClientInitialized() when using "
      << "ChromeExtensionsClient.";
}

}  // namespace

void EnsureExtensionsClientInitialized() {
  extensions::Feature::FeatureDelegatedAvailabilityCheckMap map;
  EnsureExtensionsClientInitialized(std::move(map));
}

}  // namespace components_extensions
