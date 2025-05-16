// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/resources_private/resources_private_api.h"

#include <string>
#include <utility>

#include "base/values.h"
#include "components/extensions/common/api/resources_private.h"
#include "pdf/buildflags.h"
#include "printing/buildflags/buildflags.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/webui/web_ui_util.h"

// To add a new component to this API, simply:
// 1. Add your component to the Component enum in
//      chrome/common/extensions/api/resources_private.idl
// 2. Create an AddStringsForMyComponent(base::Value::Dict * dict) method.
// 3. Tie in that method to the switch statement in Run()

namespace extensions {

namespace get_strings = api::resources_private::GetStrings;

ResourcesPrivateGetStringsFunction::ResourcesPrivateGetStringsFunction() {}

ResourcesPrivateGetStringsFunction::~ResourcesPrivateGetStringsFunction() {}

ExtensionFunction::ResponseAction ResourcesPrivateGetStringsFunction::Run() {
  // TODO(mshin): Replace g_browser_process to the general browser process instance
  // const std::string& app_locale = g_browser_process->GetApplicationLocale();
  // webui::SetLoadTimeDataDefaults(app_locale, &dict);
  //
  // return RespondNow(WithArguments(std::move(dict)));
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
