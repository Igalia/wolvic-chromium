// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_WEBRTC_DESKTOP_CAPTURE_PRIVATE_WEBRTC_DESKTOP_CAPTURE_PRIVATE_API_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_WEBRTC_DESKTOP_CAPTURE_PRIVATE_WEBRTC_DESKTOP_CAPTURE_PRIVATE_API_H_

// #include "components/browser/extensions/api/desktop_capture/desktop_capture_base.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

// TODO(mshin): Replace ExtensionFunction class after migrating desktop_capture_base.h
class WebrtcDesktopCapturePrivateChooseDesktopMediaFunction
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webrtcDesktopCapturePrivate.chooseDesktopMedia",
                             WEBRTCDESKTOPCAPTUREPRIVATE_CHOOSEDESKTOPMEDIA)
  WebrtcDesktopCapturePrivateChooseDesktopMediaFunction();

 private:
  ~WebrtcDesktopCapturePrivateChooseDesktopMediaFunction() override;

  // ExtensionFunction overrides.
  ResponseAction Run() override;
};

class WebrtcDesktopCapturePrivateCancelChooseDesktopMediaFunction
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION(
      "webrtcDesktopCapturePrivate.cancelChooseDesktopMedia",
      WEBRTCDESKTOPCAPTUREPRIVATE_CANCELCHOOSEDESKTOPMEDIA)

  WebrtcDesktopCapturePrivateCancelChooseDesktopMediaFunction();

 private:
  ~WebrtcDesktopCapturePrivateCancelChooseDesktopMediaFunction() override;

  // ExtensionFunction overrides.
  ResponseAction Run() override;
};

}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_WEBRTC_DESKTOP_CAPTURE_PRIVATE_WEBRTC_DESKTOP_CAPTURE_PRIVATE_API_H_
