// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSION_COMMON_PREF_NAMES_H_
#define COMPONENTS_EXTENSION_COMMON_PREF_NAMES_H_

#include <iterator>


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

}  // namespace prefs

#endif  // COMPONENTS_EXTENSION_COMMON_PREF_NAMES_H_
