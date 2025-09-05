// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/desktop_capture/desktop_capture_api.h"

#include "base/command_line.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "net/base/url_util.h"
#include "services/network/public/cpp/is_potentially_trustworthy.h"

namespace extensions {

DesktopCaptureChooseDesktopMediaFunction::
    DesktopCaptureChooseDesktopMediaFunction() {
}

DesktopCaptureChooseDesktopMediaFunction::
    ~DesktopCaptureChooseDesktopMediaFunction() {
}

ExtensionFunction::ResponseAction
DesktopCaptureChooseDesktopMediaFunction::Run() {
  EXTENSION_FUNCTION_VALIDATE(args().size() > 0);
  return RespondNow(Error("Not Implement"));
}

bool DesktopCaptureChooseDesktopMediaFunction::
    ShouldKeepWorkerAliveIndefinitely() {
  // `desktopCapture.chooseDesktopMedia()` displays a chooser dialog for the
  // user to select the media to share with the extension; thus, we keep the
  // worker alive for an extended period.
  return true;
}

std::string DesktopCaptureChooseDesktopMediaFunction::GetExtensionTargetName()
    const {
  return std::string();
}

DesktopCaptureCancelChooseDesktopMediaFunction::
    DesktopCaptureCancelChooseDesktopMediaFunction() {}

DesktopCaptureCancelChooseDesktopMediaFunction::
    ~DesktopCaptureCancelChooseDesktopMediaFunction() {}

ExtensionFunction::ResponseAction
DesktopCaptureCancelChooseDesktopMediaFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
