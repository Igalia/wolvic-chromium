// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/browser/session_settings.h"

#include "base/command_line.h"
#include "base/memory/ptr_util.h"
#include "base/no_destructor.h"
#include "base/strings/string_util.h"
#include "components/embedder_support/user_agent_utils.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/web_contents.h"
#include "third_party/blink/public/common/user_agent/user_agent_metadata.h"

namespace wolvic {

namespace {

const void* const kSessionSettingsKey = &kSessionSettingsKey;

const char kProductSwitch[] = "wolvic-product";

// The product token appended to every UA, passed as a command line switch.
const std::string& ProductToken() {
  static const base::NoDestructor<std::string> token(
      base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
          kProductSwitch));
  return *token;
}

// The client hints for `user_agent` in `mode`. They must agree with the UA, so
// sites do not see e.g. a desktop UA next to mobile hints.
blink::UserAgentMetadata GetUserAgentMetadata(
    SessionSettings::UserAgentMode mode,
    const std::string& user_agent) {
  // A UA we did not build for this mode, e.g. a per site override, gets only
  // the low entropy hints, as AwSettings::UpdateUserAgentLocked() does for a
  // custom UA. Overrides may append a token: check the prefix.
  if (!base::StartsWith(user_agent,
                        SessionSettings::GetDefaultUserAgent(mode))) {
    auto metadata =
        embedder_support::GetUserAgentMetadata(/*only_low_entropy_ch=*/true);
    // Sec-CH-UA-Mobile is low entropy: derive it from the UA we send.
    metadata.mobile = user_agent.find(" Mobile") != std::string::npos;
    return metadata;
  }

  const bool is_desktop = mode == SessionSettings::UserAgentMode::kDesktop;

  auto metadata = embedder_support::GetUserAgentMetadata();
  // The default mobile bit describes the device (it needs
  // --use-mobile-user-agent), not the mode we claim: set it from the mode and
  // recompute the form factors embedder_support::GetUserAgentMetadata() derived
  // from the default.
  metadata.mobile = !is_desktop;
  metadata.form_factors =
      embedder_support::GetFormFactorsClientHint(metadata, metadata.mobile);
  if (!is_desktop)
    return metadata;

  // Mirrors the desktop overrides Clank applies in
  // JNI_ContentUtils_SetUserAgentOverride(), and the DESKTOP defaults in
  // embedder_support::GetUserAgentMetadata().
  metadata.platform = "Linux";
  metadata.platform_version = std::string();
  metadata.model = std::string();
  metadata.architecture = "x86";
  metadata.bitness = "64";
  metadata.wow64 = false;
  return metadata;
}

}  // namespace

// static
std::string SessionSettings::GetDefaultUserAgent(UserAgentMode mode) {
  static const base::NoDestructor<std::string> kWolvicUserAgent(
      embedder_support::GetUserAgent() + " Mobile");
  static const base::NoDestructor<std::string> kWolvicUserAgentVR(
      embedder_support::GetUserAgent() + " Mobile VR");
  static const base::NoDestructor<std::string> kWolvicUserAgentDesktop([] {
    const char kLinuxInfoStr[] = "X11; Linux x86_64";
    return embedder_support::BuildUserAgentFromOSAndProduct(
        kLinuxInfoStr, embedder_support::GetProductAndVersion());
  }());

  std::string user_agent;
  switch (mode) {
    case UserAgentMode::kMobile:
      user_agent = *kWolvicUserAgent;
      break;
    case UserAgentMode::kMobileVR:
      user_agent = *kWolvicUserAgentVR;
      break;
    case UserAgentMode::kDesktop:
      user_agent = *kWolvicUserAgentDesktop;
      break;
  }

  if (!ProductToken().empty())
    user_agent += " " + ProductToken();
  return user_agent;
}

// static
SessionSettings* SessionSettings::GetOrCreateForWebContents(
    content::WebContents* web_contents) {
  if (auto* settings = static_cast<SessionSettings*>(
          web_contents->GetUserData(kSessionSettingsKey))) {
    return settings;
  }

  auto settings = base::WrapUnique(new SessionSettings(web_contents));
  auto* settings_ptr = settings.get();
  web_contents->SetUserData(kSessionSettingsKey, std::move(settings));
  return settings_ptr;
}

SessionSettings::SessionSettings(content::WebContents* web_contents)
    : web_contents_(web_contents) {}

SessionSettings::~SessionSettings() = default;

void SessionSettings::Apply(
    UserAgentMode user_agent_mode,
    const std::optional<std::string>& user_agent_override) {
  ApplyUserAgent(user_agent_mode, user_agent_override);
}

void SessionSettings::ApplyUserAgent(
    UserAgentMode mode,
    const std::optional<std::string>& override) {
  // A session in the default mode is served by
  // ContentBrowserClient::GetUserAgent() and needs no override at all.
  const bool is_overriding = override || mode != kDefaultUserAgentMode;

  blink::UserAgentOverride user_agent;
  if (is_overriding) {
    user_agent.ua_string_override =
        override.value_or(GetDefaultUserAgent(mode));
    user_agent.ua_metadata_override =
        GetUserAgentMetadata(mode, user_agent.ua_string_override);
  }
  web_contents_->SetUserAgentOverride(user_agent,
                                      /*override_in_new_tabs=*/true);

  // An override only reaches a navigation that is marked as overriding the
  // user agent. Mirrors AwSettings::UpdateUserAgentLocked().
  content::NavigationController& controller = web_contents_->GetController();
  for (int i = 0; i < controller.GetEntryCount(); ++i)
    controller.GetEntryAtIndex(i)->SetIsOverridingUserAgent(is_overriding);
}

}  // namespace wolvic
