// Copyright 2024 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.wolvic;

import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import org.chromium.content_public.browser.WebContents;

@JNINamespace("wolvic")
public class WolvicWebContentsFactory {
    public WolvicWebContentsFactory() {}

    public static WebContents createWebContents(boolean is_off_the_record) {
        return WolvicWebContentsFactoryJni.get().createWebContents(is_off_the_record);
    }

    @NativeMethods
    public interface Natives {
        WebContents createWebContents(boolean is_off_the_record);
    }
}
