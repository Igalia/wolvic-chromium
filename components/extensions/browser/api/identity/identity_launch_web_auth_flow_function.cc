// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/identity/identity_launch_web_auth_flow_function.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "base/metrics/histogram_functions.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "components/extensions/browser/api/identity/identity_constants.h"
#include "components/extensions/common/api/identity.h"
#include "components/prefs/pref_service.h"
#include "extensions/browser/pref_names.h"

namespace extensions {

BASE_FEATURE(kNonInteractiveTimeoutForWebAuthFlow,
             "NonInteractiveTimeoutForWebAuthFlow",
             base::FEATURE_ENABLED_BY_DEFAULT);

IdentityLaunchWebAuthFlowFunction::IdentityLaunchWebAuthFlowFunction() {
}

IdentityLaunchWebAuthFlowFunction::~IdentityLaunchWebAuthFlowFunction() {
}

ExtensionFunction::ResponseAction IdentityLaunchWebAuthFlowFunction::Run() {
  return RespondNow(NoArguments());
}

}  // namespace extensions
