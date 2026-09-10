// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/jni_headers/SessionSettings_jni.h"

#include "base/android/jni_string.h"
#include "content/public/browser/web_contents.h"
#include "wolvic/browser/session_settings.h"

using base::android::ConvertJavaStringToUTF8;
using base::android::ConvertUTF8ToJavaString;
using base::android::JavaRef;
using base::android::ScopedJavaLocalRef;

namespace wolvic {

void JNI_SessionSettings_Apply(JNIEnv* env,
                               const JavaRef<jobject>& jweb_contents,
                               jint user_agent_mode,
                               const JavaRef<jstring>& juser_agent_override) {
  auto* web_contents = content::WebContents::FromJavaWebContents(jweb_contents);
  if (!web_contents)
    return;

  std::optional<std::string> user_agent_override;
  if (juser_agent_override) {
    user_agent_override = ConvertJavaStringToUTF8(juser_agent_override);
  }

  SessionSettings::GetOrCreateForWebContents(web_contents)
      ->Apply(static_cast<SessionSettings::UserAgentMode>(user_agent_mode),
              user_agent_override);
}

ScopedJavaLocalRef<jstring> JNI_SessionSettings_GetDefaultUserAgent(JNIEnv* env,
                                                                   jint value) {
  auto mode = static_cast<SessionSettings::UserAgentMode>(value);
  return ConvertUTF8ToJavaString(env,
                                 SessionSettings::GetDefaultUserAgent(mode));
}

}  // namespace wolvic
