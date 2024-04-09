// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/browser/session_settings.h"

#include "base/check.h"
#include "components/embedder_support/user_agent_utils.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/common/user_agent.h"
#include "third_party/blink/public/common/features.h"

namespace wolvic {

namespace {

SessionSettings* g_instance = nullptr;

}  // namespace

SessionSettings::SessionSettings() {
  DCHECK(!g_instance);
  g_instance = this;
}

SessionSettings::~SessionSettings() {
  g_instance = nullptr;
}

SessionSettings* SessionSettings::Get() {
  DCHECK(g_instance);
  return g_instance;
}

void SessionSettings::SetUserAgentMode(UserAgentMode value) {
  user_agent_mode_ = value;
}

SessionSettings::UserAgentMode SessionSettings::GetUserAgentMode() const {
  return user_agent_mode_;
}

void SessionSettings::SetUserAgentOverride(
    const absl::optional<std::string>& ua_string_override) {
  user_agent_override_ = ua_string_override;
  if (!ua_string_override || !web_contents()) {
    return;
  }

  blink::UserAgentOverride override_ua_with_metadata;
  override_ua_with_metadata.ua_string_override = *ua_string_override;

  // If kUACHOverrideBlank is enabled, set user-agent metadata with the
  // default blank value.
  if (ua_string_override && !ua_string_override->empty() &&
      base::FeatureList::IsEnabled(blink::features::kUACHOverrideBlank)) {
    override_ua_with_metadata.ua_metadata_override =
      blink::UserAgentMetadata();
  }

  // Generate user-agent client hints in the following three cases:
  // 1. If user provide the user-agent metadata overrides, we use the
  // override data to populate the user-agent client hints.
  // 2. Otherwise, if override user-agent contains default user-agent, we
  // use system default user-agent metadata to populate the user-agent
  // client hints.
  // 3. Finally, if the above two cases don't match, we only populate system
  // default low-entropy client hints.
  if (base::FeatureList::IsEnabled(blink::features::kUserAgentClientHint)) {
    // TODO(jfernandez): Implement the user-agent client hints logic
  }

  // Set overridden user-agent and default client hints metadata if applied.
  web_contents()->SetUserAgentOverride(override_ua_with_metadata, true);

  content::NavigationController& controller = web_contents()->GetController();
  for (int i = 0; i < controller.GetEntryCount(); ++i)
    controller.GetEntryAtIndex(i)->SetIsOverridingUserAgent(true);
}

void SessionSettings::SetWebContents(content::WebContents* web_contents) {
   Observe(web_contents);
}

absl::optional<std::string> SessionSettings::GetUserAgentOverride() const {
  return user_agent_override_;
}

std::string SessionSettings::GetDefaultUserAgent(UserAgentMode mode) const {
  static std::string kWolvicUserAgent;
  static std::string kWolvicUserAgentVR;
  static std::string kWolvicUserAgentDesktop;

  static std::once_flag once_flag;
  std::call_once(once_flag, [] {
    kWolvicUserAgent = embedder_support::GetUserAgent() + " Mobile";
    kWolvicUserAgentVR = embedder_support::GetUserAgent() + " Mobile VR";

    const char kLinuxInfoStr[] = "X11; Linux x86_64";
    kWolvicUserAgentDesktop = content::BuildUserAgentFromOSAndProduct(kLinuxInfoStr, embedder_support::GetProductAndVersion());
  });

  switch (mode) {
    case UserAgentMode::kMobile:
      return kWolvicUserAgent;
    case UserAgentMode::kMobileVR:
      return kWolvicUserAgentVR;
    case UserAgentMode::kDesktop:
      return kWolvicUserAgentDesktop;
  }
}

void SessionSettings::RenderViewHostChanged(content::RenderViewHost* old_host,
                                            content::RenderViewHost* new_host) {
  DCHECK_EQ(new_host, web_contents()->GetRenderViewHost());

  //UpdateEverything();
}

void SessionSettings::WebContentsDestroyed() {
  // The destroyed WebContents instance is removed from the Observers
  // lists, calling to the ResetWebContents private functtion, which
  // assignes nullptr to the web_contents_ attribute.
}

}  // namespace wolvic
