// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.wolvic;

import androidx.annotation.Nullable;

import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.NativeMethods;

@JNINamespace("wolvic")
public class SessionSettings {
    public SessionSettings() {}

    public void setUserAgentMode(int value) {
        SessionSettingsJni.get().setUserAgentMode(value);
    }

    public int getUserAgentMode() {
        return SessionSettingsJni.get().getUserAgentMode();
    }

    public void setUserAgentOverride(@Nullable String value) {
        SessionSettingsJni.get().setUserAgentOverride(value);
    }

    @Nullable
    public String getUserAgentOverride() {
        return SessionSettingsJni.get().getUserAgentOverride();
    }

    @NativeMethods
    public interface Natives {
        void setUserAgentMode(int value);
        int getUserAgentMode();
        void setUserAgentOverride(@Nullable String value);
        @Nullable
        String getUserAgentOverride();
    }
}
