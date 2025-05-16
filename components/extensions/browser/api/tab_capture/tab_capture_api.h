// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Defines the Chrome Extensions Tab Capture API functions for accessing
// tab media streams.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_TAB_CAPTURE_TAB_CAPTURE_API_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_TAB_CAPTURE_TAB_CAPTURE_API_H_

#include "components/extensions/common/api/tab_capture.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

class TabCaptureCaptureFunction final : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("tabCapture.capture", TABCAPTURE_CAPTURE)

 private:
  ~TabCaptureCaptureFunction() final {}

  // ExtensionFunction:
  ResponseAction Run() final;
};

class TabCaptureGetCapturedTabsFunction final : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("tabCapture.getCapturedTabs",
                             TABCAPTURE_GETCAPTUREDTABS)

 private:
  ~TabCaptureGetCapturedTabsFunction() final {}

  // ExtensionFunction:
  ResponseAction Run() final;
};

class TabCaptureGetMediaStreamIdFunction final : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("tabCapture.getMediaStreamId",
                             TABCAPTURE_GETMEDIASTREAMID)

 private:
  ~TabCaptureGetMediaStreamIdFunction() final {}

  // ExtensionFunction:
  ResponseAction Run() final;
};

}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_TAB_CAPTURE_TAB_CAPTURE_API_H_
