// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Implements the Chrome Extensions Tab Capture API.

#include "components/extensions/browser/api/tab_capture/tab_capture_api.h"

#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "base/command_line.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "components/sessions/content/session_tab_helper.h"
#include "content/public/browser/desktop_media_id.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "extensions/common/features/feature.h"
#include "extensions/common/features/feature_provider.h"
#include "extensions/common/features/simple_feature.h"
#include "extensions/common/permissions/permissions_data.h"
#include "extensions/common/switches.h"
#include "net/base/url_util.h"
#include "services/network/public/cpp/is_potentially_trustworthy.h"

using content::DesktopMediaID;
using content::WebContentsMediaCaptureId;
using extensions::api::tab_capture::MediaStreamConstraint;

namespace TabCapture = extensions::api::tab_capture;
namespace GetCapturedTabs = TabCapture::GetCapturedTabs;

namespace extensions {

ExtensionFunction::ResponseAction TabCaptureCaptureFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction TabCaptureGetCapturedTabsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction TabCaptureGetMediaStreamIdFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
