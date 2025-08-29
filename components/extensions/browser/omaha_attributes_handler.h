// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_BROWSER_EXTENSIONS_OMAHA_ATTRIBUTES_HANDLER_H_
#define COMPONENTS_BROWSER_EXTENSIONS_OMAHA_ATTRIBUTES_HANDLER_H_

#include "base/memory/raw_ptr.h"
#include "components/extensions/browser/blocklist.h"
#include "extensions/browser/blocklist_state.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/extension_id.h"

namespace base {
class Value;
}

namespace extensions {
class ExtensionPrefs;
}

namespace components_extensions {
class ExtensionService;

// These values are logged to UMA. Entries should not be renumbered and
// numeric values should never be reused. Please keep in sync with
// "ExtensionUpdateCheckDataKey" in
// src/tools/metrics/histograms/metadata/extensions/enums.xml.
enum class ExtensionUpdateCheckDataKey {
  // No update check data keys were found so no action was taken.
  kNoKey = 0,
  // The update check data keys had a "_malware" key resulting in the extension
  // being disabled.
  kMalware = 1,
  // The update check data keys had a "_potentially_uws" key resulting in the
  // extension being disabled.
  kPotentiallyUWS = 2,
  // The update check data keys had a "_policy_violation" key resulting in the
  // extension being disabled.
  kPolicyViolation = 3,
  kMaxValue = kPolicyViolation
};

// Manages the Omaha attributes blocklist/greylist states in extension pref.
class OmahaAttributesHandler {
 public:
  OmahaAttributesHandler(extensions::ExtensionPrefs* extension_prefs,
                         extensions::ExtensionRegistry* registry,
                         ExtensionService* extension_service);
  OmahaAttributesHandler(const OmahaAttributesHandler&) = delete;
  OmahaAttributesHandler& operator=(const OmahaAttributesHandler&) = delete;
  ~OmahaAttributesHandler() = default;

  // Performs action based on Omaha attributes for the extension.
  void PerformActionBasedOnOmahaAttributes(const extensions::ExtensionId& extension_id,
                                           const base::Value::Dict& attributes);

 private:
  // Performs action based on `attributes` for the `extension_id`. If the
  // extension does not have the _malware attribute, remove it from the Omaha
  // malware blocklist state and maybe reload it. Otherwise, add it to the Omaha
  // malware blocklist state and maybe unload it.
  void HandleMalwareOmahaAttribute(const extensions::ExtensionId& extension_id,
                                   const base::Value::Dict& attributes);
  // Performs action based on `attributes` for the `extension_id`. If the
  // extension is not in the `greylist_state`, remove it from the Omaha
  // blocklist state and maybe re-enable it. Otherwise, add it to the Omaha
  // blocklist state and maybe disable it. `reason` is used for logging UMA
  // metrics.
  void HandleGreylistOmahaAttribute(const extensions::ExtensionId& extension_id,
                                    const base::Value::Dict& attributes,
                                    extensions::BitMapBlocklistState greylist_state,
                                    ExtensionUpdateCheckDataKey reason);

  raw_ptr<extensions::ExtensionPrefs> extension_prefs_ = nullptr;
  raw_ptr<extensions::ExtensionRegistry> registry_ = nullptr;
  raw_ptr<ExtensionService> extension_service_ = nullptr;
};

}  // namespace components_extensions

#endif  // COMPONENTS_BROWSER_EXTENSIONS_OMAHA_ATTRIBUTES_HANDLER_H_
