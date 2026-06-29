// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/browser/session_settings.h"

#include "base/check.h"
#include "base/no_destructor.h"
#include "components/embedder_support/user_agent_utils.h"

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
    const std::optional<std::string>& value) {
  user_agent_override_ = value;
}

std::optional<std::string> SessionSettings::GetUserAgentOverride() const {
  return user_agent_override_;
}

std::string SessionSettings::GetDefaultUserAgent(UserAgentMode mode) const {
  static const base::NoDestructor<std::string> kWolvicUserAgent(
      embedder_support::GetUserAgent() + " Mobile");
  static const base::NoDestructor<std::string> kWolvicUserAgentVR(
      embedder_support::GetUserAgent() + " Mobile VR");
  static const base::NoDestructor<std::string> kWolvicUserAgentDesktop([] {
    const char kLinuxInfoStr[] = "X11; Linux x86_64";
    return embedder_support::BuildUserAgentFromOSAndProduct(
        kLinuxInfoStr, embedder_support::GetProductAndVersion());
  }());

  switch (mode) {
    case UserAgentMode::kMobile:
      return *kWolvicUserAgent;
    case UserAgentMode::kMobileVR:
      return *kWolvicUserAgentVR;
    case UserAgentMode::kDesktop:
      return *kWolvicUserAgentDesktop;
  }
}

}  // namespace wolvic
