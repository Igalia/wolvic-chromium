// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/browser/vr/wvr_thread.h"
#include "wolvic/browser/vr/wvr_api.h"

namespace wolvic {

WvrThread::WvrThread(base::OnceCallback<void()> initialized_callback)
    : base::android::JavaHandlerThread("WvrThread"),
      initialized_callback_(std::move(initialized_callback)) {}

WvrThread::~WvrThread() {
  Stop();
}

void WvrThread::Init() {
  DCHECK(!wvr_manager_);
  wvr_api_ = std::make_unique<WvrApi>();
  wvr_graphics_ = std::make_unique<WvrGraphicsDelegate>();

  wvr_manager_ = std::make_unique<WvrManager>(wvr_api_.get(), wvr_graphics_.get());

  std::move(initialized_callback_).Run();
}

void WvrThread::CleanUp() {
  // Destroy everything created in Init() here, on the WVR thread, while the GL
  // context is current. WvrGraphicsDelegate frees GL resources in its
  // destructor, so it must not be destroyed on the main thread in ~WvrThread().
  // wvr_manager_ holds raw pointers to the other two, so it goes first.
  wvr_manager_.reset();
  wvr_graphics_.reset();
  wvr_api_.reset();
}

}  // namespace wolvic
