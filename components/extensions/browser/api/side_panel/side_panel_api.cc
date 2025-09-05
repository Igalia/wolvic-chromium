// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/side_panel/side_panel_api.h"

#include <optional>

#include "base/types/expected.h"
#include "base/values.h"
#include "components/extensions/common/api/side_panel.h"
#include "extensions/common/extension_features.h"

namespace extensions {
namespace {

bool IsSidePanelApiAvailable() {
  return base::FeatureList::IsEnabled(
      extensions_features::kExtensionSidePanelIntegration);
}

}  // namespace

SidePanelApiFunction::SidePanelApiFunction() = default;
SidePanelApiFunction::~SidePanelApiFunction() = default;
SidePanelService* SidePanelApiFunction::GetService() {
  return nullptr;
}

ExtensionFunction::ResponseAction SidePanelApiFunction::Run() {
  if (!IsSidePanelApiAvailable())
    return RespondNow(Error("API Unavailable"));
  return RunFunction();
}

ExtensionFunction::ResponseAction SidePanelGetOptionsFunction::RunFunction() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction SidePanelSetOptionsFunction::RunFunction() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction
SidePanelSetPanelBehaviorFunction::RunFunction() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction
SidePanelGetPanelBehaviorFunction::RunFunction() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction SidePanelOpenFunction::RunFunction() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
