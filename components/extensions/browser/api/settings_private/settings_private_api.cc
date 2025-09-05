// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/settings_private/settings_private_api.h"

#include <utility>

#include "base/values.h"
#include "components/extensions/common/api/settings_private.h"
#include "content/public/common/page_zoom.h"
#include "extensions/browser/extension_function_registry.h"

namespace extensions {

////////////////////////////////////////////////////////////////////////////////
// SettingsPrivateSetPrefFunction
////////////////////////////////////////////////////////////////////////////////

SettingsPrivateSetPrefFunction::~SettingsPrivateSetPrefFunction() {
}

ExtensionFunction::ResponseAction SettingsPrivateSetPrefFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// SettingsPrivateGetAllPrefsFunction
////////////////////////////////////////////////////////////////////////////////

SettingsPrivateGetAllPrefsFunction::~SettingsPrivateGetAllPrefsFunction() {
}

ExtensionFunction::ResponseAction SettingsPrivateGetAllPrefsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// SettingsPrivateGetPrefFunction
////////////////////////////////////////////////////////////////////////////////

SettingsPrivateGetPrefFunction::~SettingsPrivateGetPrefFunction() {
}

ExtensionFunction::ResponseAction SettingsPrivateGetPrefFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// SettingsPrivateGetDefaultZoomFunction
////////////////////////////////////////////////////////////////////////////////

SettingsPrivateGetDefaultZoomFunction::
    ~SettingsPrivateGetDefaultZoomFunction() {
}

ExtensionFunction::ResponseAction
    SettingsPrivateGetDefaultZoomFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// SettingsPrivateSetDefaultZoomFunction
////////////////////////////////////////////////////////////////////////////////

SettingsPrivateSetDefaultZoomFunction::
    ~SettingsPrivateSetDefaultZoomFunction() {
}

ExtensionFunction::ResponseAction
    SettingsPrivateSetDefaultZoomFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
