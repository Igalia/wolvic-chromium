// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.wolvic;

import androidx.annotation.Nullable;

import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import org.chromium.content_public.browser.WebContents;

/**
 * The user agent settings of a session. The values live here
 * because a session is configured before its WebContents exists; they are
 * applied to the WebContents once it is available.
 */
@JNINamespace("wolvic")
public class SessionSettings {
    public enum UserAgentMode {
        // values have to be synchronized with session_settings.h
        MOBILE(0),
        DESKTOP(1),
        MOBILE_VR(2);

        private final int value;

        private UserAgentMode(int value) {
            this.value = value;
        }

        private int getValue() {
            return value;
        }
    };

    // Has to match SessionSettings::kDefaultUserAgentMode.
    private UserAgentMode mUserAgentMode = UserAgentMode.MOBILE_VR;
    private @Nullable String mUserAgentOverride;
    private @Nullable WebContents mWebContents;

    public SessionSettings() {}

    /** Binds the settings to the WebContents of the session, or unbinds them. */
    public void setWebContents(@Nullable WebContents webContents) {
        mWebContents = webContents;
        apply();
    }

    public void setUserAgentMode(UserAgentMode mode) {
        mUserAgentMode = mode;
        apply();
    }

    public UserAgentMode getUserAgentMode() {
        return mUserAgentMode;
    }

    public void setUserAgentOverride(@Nullable String value) {
        mUserAgentOverride = value;
        apply();
    }

    @Nullable
    public String getUserAgentOverride() {
        return mUserAgentOverride;
    }

    public String getDefaultUserAgent(UserAgentMode mode) {
        return SessionSettingsJni.get().getDefaultUserAgent(mode.getValue());
    }

    private void apply() {
        if (mWebContents == null) return;

        SessionSettingsJni.get().apply(
                mWebContents, mUserAgentMode.getValue(), mUserAgentOverride);
    }

    @NativeMethods
    public interface Natives {
        void apply(WebContents webContents, int userAgentMode,
                @Nullable String userAgentOverride);
        String getDefaultUserAgent(int mode);
    }
}
