// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/webrtc_desktop_capture_private/webrtc_desktop_capture_private_api.h"

#include "base/command_line.h"
#include "base/strings/utf_string_conversions.h"
#include "components/extensions/common/api/tabs.h"
#include "components/extensions/common/api/webrtc_desktop_capture_private.h"
#include "content/public/browser/web_contents.h"
#include "net/base/url_util.h"
#include "services/network/public/cpp/is_potentially_trustworthy.h"

namespace extensions {

WebrtcDesktopCapturePrivateChooseDesktopMediaFunction::
    WebrtcDesktopCapturePrivateChooseDesktopMediaFunction() {
}

WebrtcDesktopCapturePrivateChooseDesktopMediaFunction::
    ~WebrtcDesktopCapturePrivateChooseDesktopMediaFunction() {
}

ExtensionFunction::ResponseAction
WebrtcDesktopCapturePrivateChooseDesktopMediaFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

WebrtcDesktopCapturePrivateCancelChooseDesktopMediaFunction::
    WebrtcDesktopCapturePrivateCancelChooseDesktopMediaFunction() {}

WebrtcDesktopCapturePrivateCancelChooseDesktopMediaFunction::
    ~WebrtcDesktopCapturePrivateCancelChooseDesktopMediaFunction() {}

ExtensionFunction::ResponseAction
WebrtcDesktopCapturePrivateCancelChooseDesktopMediaFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
