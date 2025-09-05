// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_DESKTOP_CAPTURE_DESKTOP_CAPTURE_API_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_DESKTOP_CAPTURE_DESKTOP_CAPTURE_API_H_

// #include "components/extensions/browser/api/desktop_capture/desktop_capture_base.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

// TODO(mshin): Replace ExtensionFunction class after migrating desktop_capture_base.h
class DesktopCaptureChooseDesktopMediaFunction
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("desktopCapture.chooseDesktopMedia",
                             DESKTOPCAPTURE_CHOOSEDESKTOPMEDIA)

  DesktopCaptureChooseDesktopMediaFunction();

 private:
  ~DesktopCaptureChooseDesktopMediaFunction() override;

  // ExtensionFunction overrides.
  ResponseAction Run() override;
  bool ShouldKeepWorkerAliveIndefinitely() override;

  // Returns the target name to show in the picker when capture is requested for
  // an extension.  Currently this is the same as the application name.
  std::string GetExtensionTargetName() const;
};

class DesktopCaptureCancelChooseDesktopMediaFunction
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("desktopCapture.cancelChooseDesktopMedia",
                             DESKTOPCAPTURE_CANCELCHOOSEDESKTOPMEDIA)

  DesktopCaptureCancelChooseDesktopMediaFunction();

  // ExtensionFunction overrides.
  ResponseAction Run() override;

 private:
  ~DesktopCaptureCancelChooseDesktopMediaFunction() override;
};

}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_DESKTOP_CAPTURE_DESKTOP_CAPTURE_API_H_
