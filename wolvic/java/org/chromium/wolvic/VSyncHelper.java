// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.wolvic;

import android.content.Context;
import android.view.Choreographer;

import org.chromium.base.ContextUtils;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.NativeMethods;
import org.chromium.ui.display.DisplayAndroidManager;

/**
 * Helper class for interfacing with the Android Choreographer from native code.
 * Much of this was inspired in AndroidVSyncHelper class from Chrome.
 */
@JNINamespace("wolvic")
public class VSyncHelper {
    private final long mNativeVSyncHelper;

    private final Choreographer.FrameCallback mCallback = new Choreographer.FrameCallback() {
        @Override
        public void doFrame(long frameTimeNanos) {
            if (mNativeVSyncHelper == 0) return;
            VSyncHelperJni.get().onVSync(
                    mNativeVSyncHelper, VSyncHelper.this, frameTimeNanos);
        }
    };

    @CalledByNative
    private static VSyncHelper create(long nativeVSyncHelper) {
        return new VSyncHelper(nativeVSyncHelper);
    }

    private VSyncHelper(long nativeVSyncHelper) {
        mNativeVSyncHelper = nativeVSyncHelper;
    }

    @CalledByNative
    private void requestVSync() {
        Choreographer.getInstance().postFrameCallback(mCallback);
    }

    @CalledByNative
    private void cancelVSyncRequest() {
        Choreographer.getInstance().removeFrameCallback(mCallback);
    }

    @CalledByNative
    private float getRefreshRate() {
        Context context = ContextUtils.getApplicationContext();
        return DisplayAndroidManager.getDefaultDisplayForContext(context).getRefreshRate();
    }

    @NativeMethods
    interface Natives {
        void onVSync(long nativeVSyncHelper, VSyncHelper caller, long frameTimeNanos);
    }
}
