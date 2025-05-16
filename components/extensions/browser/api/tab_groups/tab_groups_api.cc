// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/tab_groups/tab_groups_api.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/strings/pattern.h"
#include "base/strings/utf_string_conversions.h"
#include "components/extensions/common/extension_constants.h"
#include "components/tab_groups/tab_group_color.h"
#include "components/tab_groups/tab_group_id.h"
#include "components/tab_groups/tab_group_visual_data.h"
#include "ui/gfx/range/range.h"

namespace extensions {

ExtensionFunction::ResponseAction TabGroupsGetFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction TabGroupsQueryFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction TabGroupsUpdateFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction TabGroupsMoveFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
