// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSION_COMMON_PREF_NAMES_H_
#define COMPONENTS_EXTENSION_COMMON_PREF_NAMES_H_

#include <iterator>

#include "build/build_config.h"


namespace prefs {

// *************** PROFILE PREFS ***************
// These are migrated from //chrome/common to mapping to extensions pref,
// and attached to the user profile.

// Boolean that is true when Suggest support is enabled.
inline constexpr char kSearchSuggestEnabled[] = "search.suggest_enabled";

// Whether to enable hyperlink auditing ("<a ping>").
inline constexpr char kEnableHyperlinkAuditing[] = "enable_a_ping";

// Whether to enable sending referrers.
inline constexpr char kEnableReferrers[] = "enable_referrers";

// This references a default content setting value which we expose through the
// preferences extensions API and also used for migration of the old
// |kEnableDRM| preference.
inline constexpr char kProtectedContentDefault[] =
    "profile.default_content_setting_values.protected_media_identifier";

// Define the IP handling policy override that WebRTC should follow. When not
// set, it defaults to "default".
inline constexpr char kWebRTCIPHandlingPolicy[] = "webrtc.ip_handling_policy";
// Define range of UDP ports allowed to be used by WebRTC PeerConnections.
inline constexpr char kWebRTCUDPPortRange[] = "webrtc.udp_port_range";

// Whether Extensions are enabled.
inline constexpr char kDisableExtensions[] = "extensions.disabled";

// Boolean pref which persists whether the extensions_ui is in developer mode
// (showing developer packing tools and extensions details)
inline constexpr char kExtensionsUIDeveloperMode[] =
    "extensions.ui.developer_mode";

// Dictionary pref that tracks which command belongs to which
// extension + named command pair.
inline constexpr char kExtensionCommands[] = "extensions.commands";

// Boolean that indicates whether Chrome enterprise extension request is enabled
// or not.
inline constexpr char kCloudExtensionRequestEnabled[] =
    "enterprise_reporting.extension_request.enabled";

// A list of extension ids represents pending extension request. The ids are
// stored once user sent the request until the request is canceled, approved or
// denied.
inline constexpr char kCloudExtensionRequestIds[] =
    "enterprise_reporting.extension_request.ids";

// Policy that indicates how to handle animated images.
inline constexpr char kAnimationPolicy[] = "settings.a11y.animation_policy";

// A list of URLs (for U2F) or domains (for webauthn) that automatically permit
// direct attestation of a Security Key.
inline constexpr char kSecurityKeyPermitAttestation[] =
    "securitykey.permit_attestation";

#if BUILDFLAG(IS_MAC)
// Whether to create platform WebAuthn credentials in iCloud Keychain rather
// than the Chrome profile.
inline constexpr char kCreatePasskeysInICloudKeychain[] =
    "webauthn.create_in_icloud_keychain";
#endif

// Records the last time the CWS Info Service downloaded information about
// currently installed extensions from the Chrome Web Store, successfully
// compared it with the information stored in extension_prefs and updated the
// latter if necessary. The timestamp therefore represents the "freshness" of
// the CWS information saved.
inline constexpr char kCWSInfoTimestamp[] = "extensions.cws_info_timestamp";
inline constexpr char kCWSInfoFetchErrorTimestamp[] =
    "extensions.cws_info_fetch_error_timestamp";

// A bool value for running GarbageCollectStoragePartitionCommand.
inline constexpr char kShouldGarbageCollectStoragePartitions[] =
    "storage_partitions.should_garbage_collect";
}  // namespace prefs

#endif  // COMPONENTS_EXTENSION_COMMON_PREF_NAMES_H_
