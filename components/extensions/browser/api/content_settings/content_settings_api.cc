// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/content_settings/content_settings_api.h"

namespace extensions {

ExtensionFunction::ResponseAction
ContentSettingsContentSettingClearFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction
ContentSettingsContentSettingGetFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction
ContentSettingsContentSettingSetFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction
ContentSettingsContentSettingGetResourceIdentifiersFunction::Run() {
  // The only setting that supported resource identifiers was plugins. Since
  // plugins have been deprecated since Chrome 87, there are no resource
  // identifiers for existing settings (but we retain the function for
  // backwards and potential forwards compatibility).
  return RespondNow(NoArguments());
}

}  // namespace extensions
