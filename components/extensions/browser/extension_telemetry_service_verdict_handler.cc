// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/extension_telemetry_service_verdict_handler.h"

#include "base/metrics/histogram_functions.h"
#include "components/extensions/browser/extension_service.h"
#include "extensions/browser/blocklist_extension_prefs.h"
#include "extensions/browser/blocklist_state.h"

using extensions::BitMapBlocklistState;
using extensions::ExtensionIdSet;
using extensions::ExtensionPrefs;
using extensions::ExtensionRegistry;

namespace components_extensions {

namespace blocklist_prefs = extensions::blocklist_prefs;

namespace {

ExtensionTelemetryDisableReason GetExtensionTelemetryDisableReason(
    BitMapBlocklistState state) {
  switch (state) {
    case BitMapBlocklistState::BLOCKLISTED_MALWARE:
      return ExtensionTelemetryDisableReason::kMalware;
    default:
      return ExtensionTelemetryDisableReason::kUnknown;
  }
}

// Logs UMA metrics when an off-store extension is disabled.
void ReportOffstoreExtensionDisabled(ExtensionTelemetryDisableReason reason) {
  base::UmaHistogramEnumeration(
      "SafeBrowsing.ExtensionTelemetry.OffstoreExtensionDisabledReason",
      reason);
}

// Logs UMA metrics when an off-store extension is re-enabled.
void ReportOffstoreExtensionReenabled(BitMapBlocklistState state) {
  base::UmaHistogramEnumeration(
      "SafeBrowsing.ExtensionTelemetry.OffstoreExtensionReenabled_"
      "PastDisabledReason",
      GetExtensionTelemetryDisableReason(state));
}

}  // namespace

ExtensionTelemetryServiceVerdictHandler::
    ExtensionTelemetryServiceVerdictHandler(ExtensionPrefs* extension_prefs,
                                            ExtensionRegistry* registry,
                                            ExtensionService* extension_service)
    : extension_prefs_(extension_prefs),
      registry_(registry),
      extension_service_(extension_service) {}

void ExtensionTelemetryServiceVerdictHandler::PerformActionBasedOnVerdicts(
    const Blocklist::BlocklistStateMap& state_map) {
  ExtensionIdSet installed_ids =
      registry_->GenerateInstalledExtensionsSet().GetIDs();

  for (const auto& [extension_id, blocklist_state] : state_map) {
    // It's possible that an extension is already uninstalled. Ignore in this
    // case.
    if (!installed_ids.contains(extension_id)) {
      continue;
    }

    // If the blocklist state has not changed, do nothing.
    const BitMapBlocklistState& current_state =
        blocklist_prefs::GetExtensionTelemetryServiceBlocklistState(
            extension_id, extension_prefs_);
    if (static_cast<BitMapBlocklistState>(blocklist_state) == current_state) {
      continue;
    }

    switch (blocklist_state) {
      case extensions::NOT_BLOCKLISTED:
        blocklist_prefs::SetExtensionTelemetryServiceBlocklistState(
            extension_id, BitMapBlocklistState::NOT_BLOCKLISTED,
            extension_prefs_);
        extension_service_->OnBlocklistStateRemoved(extension_id);
        ReportOffstoreExtensionReenabled(current_state);
        break;
      case extensions::BLOCKLISTED_MALWARE:
        blocklist_prefs::SetExtensionTelemetryServiceBlocklistState(
            extension_id, BitMapBlocklistState::BLOCKLISTED_MALWARE,
            extension_prefs_);
        extension_service_->OnBlocklistStateAdded(extension_id);
        ReportOffstoreExtensionDisabled(
            ExtensionTelemetryDisableReason::kMalware);
        break;
      case extensions::BLOCKLISTED_SECURITY_VULNERABILITY:
      case extensions::BLOCKLISTED_CWS_POLICY_VIOLATION:
      case extensions::BLOCKLISTED_POTENTIALLY_UNWANTED:
      case extensions::BLOCKLISTED_UNKNOWN:
        break;
    }
  }
}

}  // namespace components_extensions
