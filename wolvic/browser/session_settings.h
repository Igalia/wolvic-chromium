// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WOLVIC_BROWSER_SESSION_SETTINGS_H_
#define WOLVIC_BROWSER_SESSION_SETTINGS_H_

#include <optional>
#include <string>

#include "base/memory/raw_ptr.h"
#include "base/supports_user_data.h"

namespace content {
class WebContents;
}

namespace wolvic {

// The user agent and viewport settings of a single session. Java owns the
// values, this applies them to the WebContents the session is showing.
class SessionSettings : public base::SupportsUserData::Data {
 public:
  enum class UserAgentMode {
    // values have to be synchronized with SessionSettings.java
    kMobile = 0,
    kDesktop = 1,
    kMobileVR = 2,
  };

  // The user agent Wolvic sends when a session does not ask for anything else.
  // Sessions in any other mode override it on their own WebContents.
  static constexpr UserAgentMode kDefaultUserAgentMode =
      UserAgentMode::kMobileVR;

  static std::string GetDefaultUserAgent(UserAgentMode mode);

  static SessionSettings* GetOrCreateForWebContents(
      content::WebContents* web_contents);

  SessionSettings(const SessionSettings&) = delete;
  SessionSettings& operator=(const SessionSettings&) = delete;
  ~SessionSettings() override;

  void Apply(UserAgentMode user_agent_mode,
             const std::optional<std::string>& user_agent_override);

 private:
  explicit SessionSettings(content::WebContents* web_contents);

  void ApplyUserAgent(UserAgentMode mode,
                      const std::optional<std::string>& override);

  const raw_ptr<content::WebContents> web_contents_;
};

}  // namespace wolvic

#endif  // WOLVIC_BROWSER_SESSION_SETTINGS_H_
