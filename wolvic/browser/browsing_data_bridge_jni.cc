// Copyright 2026 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/jni_headers/BrowsingDataBridge_jni.h"

#include <memory>

#include "base/android/callback_android.h"
#include "base/android/scoped_java_ref.h"
#include "base/time/time.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browsing_data_remover.h"
#include "wolvic/wolvic_content_browser_client.h"

using base::android::JavaRef;
using base::android::ScopedJavaGlobalRef;
using content::BrowsingDataRemover;

namespace wolvic {

namespace {

// Keep in sync with the constants in BrowsingDataBridge.java.
constexpr jint kCookies = 1 << 0;
constexpr jint kCache = 1 << 1;
constexpr jint kDomStorage = 1 << 2;

// Owns itself: deletes itself once the removal task is done.
class RemovalObserver : public BrowsingDataRemover::Observer {
 public:
  explicit RemovalObserver(const JavaRef<jobject>& callback)
      : callback_(callback) {}

  void OnBrowsingDataRemoverDone(uint64_t failed_data_types) override {
    base::android::RunBooleanCallbackAndroid(callback_, failed_data_types == 0);
    delete this;
  }

 private:
  ~RemovalObserver() override = default;

  ScopedJavaGlobalRef<jobject> callback_;
};

}  // namespace

void JNI_BrowsingDataBridge_ClearData(JNIEnv* env,
                                      jint types,
                                      const JavaRef<jobject>& callback) {
  uint64_t remove_mask = 0;
  if (types & kCookies)
    remove_mask |= BrowsingDataRemover::DATA_TYPE_COOKIES;
  if (types & kCache)
    remove_mask |= BrowsingDataRemover::DATA_TYPE_CACHE;
  if (types & kDomStorage)
    remove_mask |= BrowsingDataRemover::DATA_TYPE_DOM_STORAGE;

  // Only the regular context: private sessions are deliberately left alone.
  content::BrowserContext* context =
      WolvicContentBrowserClient::Get()->browser_context();
  if (!context || !remove_mask) {
    base::android::RunBooleanCallbackAndroid(callback, context != nullptr);
    return;
  }

  context->GetBrowsingDataRemover()->RemoveAndReply(
      base::Time(), base::Time::Max(), remove_mask,
      BrowsingDataRemover::ORIGIN_TYPE_UNPROTECTED_WEB |
          BrowsingDataRemover::ORIGIN_TYPE_PROTECTED_WEB,
      new RemovalObserver(callback));
}

}  // namespace wolvic
