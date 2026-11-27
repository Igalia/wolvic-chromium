// Copyright 2026 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.wolvic;

import org.jni_zero.JNINamespace;
import org.jni_zero.NativeMethods;

import org.chromium.base.Callback;

/**
 * Clears browsing data of the regular (non off-the-record) browser context
 * through content::BrowsingDataRemover.
 */
@JNINamespace("wolvic")
public class BrowsingDataBridge {
    // Keep in sync with the switch in browsing_data_bridge_jni.cc.
    public static final int COOKIES = 1 << 0;
    public static final int CACHE = 1 << 1;
    public static final int DOM_STORAGE = 1 << 2;

    /**
     * Removes all the data of the given types. Must be called on the UI thread.
     *
     * @param types a combination of COOKIES, CACHE and DOM_STORAGE.
     * @param callback run on the UI thread when finished, with true if every
     *     requested type was removed successfully.
     */
    public static void clearData(int types, Callback<Boolean> callback) {
        BrowsingDataBridgeJni.get().clearData(types, callback);
    }

    @NativeMethods
    public interface Natives {
        void clearData(int types, Callback<Boolean> callback);
    }
}
