// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_TELEMETRY_SERVICE_VERDICT_HANDLER_H_
#define COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_TELEMETRY_SERVICE_VERDICT_HANDLER_H_

#include "base/memory/raw_ptr.h"
#include "components/extensions/browser/blocklist.h"
#include "extensions/browser/extension_registry.h"

namespace extensions {
class ExtensionPrefs;
}

namespace components_extensions {
class ExtensionService;

// These values are logged to UMA. Entries should not be renumbered and
// numeric values should never be reused. Please keep in sync with
// "SBExtensionTelemetryDisableReason" in
// src/tools/metrics/histograms/enums.xml.
enum class ExtensionTelemetryDisableReason {
  kUnknown = 0,
  kMalware = 1,
  kMaxValue = kMalware
};

// Manages the Extension Telemetry service verdict states in extension pref.
class ExtensionTelemetryServiceVerdictHandler {
 public:
  ExtensionTelemetryServiceVerdictHandler(extensions::ExtensionPrefs* extension_prefs,
                                          extensions::ExtensionRegistry* registry,
                                          ExtensionService* extension_service);
  ExtensionTelemetryServiceVerdictHandler(
      const ExtensionTelemetryServiceVerdictHandler&) = delete;
  ExtensionTelemetryServiceVerdictHandler& operator=(
      const ExtensionTelemetryServiceVerdictHandler&) = delete;
  ~ExtensionTelemetryServiceVerdictHandler() = default;

  // Performs action based on verdicts received from the Extension Telemetry
  // server. Currently, the verdicts are limited to off-store extensions. It's
  // possible that the action is already performed for a verdict, in this case,
  // nothing is done.
  //
  // |state_map| represents the converted blocklist states from verdicts. For
  // each state, the following action is performed:
  // MALWARE - Unloads the extension and adds it to the Extension Telemetry
  // service malware blocklist.
  // NOT_BLOCKLISTED - Reloads the extension and removes it from the Extension
  // Telemetry service malware blocklist.
  void PerformActionBasedOnVerdicts(
      const Blocklist::BlocklistStateMap& state_map);

 private:
  raw_ptr<extensions::ExtensionPrefs> extension_prefs_ = nullptr;
  raw_ptr<extensions::ExtensionRegistry> registry_ = nullptr;
  raw_ptr<ExtensionService> extension_service_ = nullptr;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_TELEMETRY_SERVICE_VERDICT_HANDLER_H_
