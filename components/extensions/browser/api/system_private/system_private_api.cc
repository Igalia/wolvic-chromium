// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/system_private/system_private_api.h"

#include <memory>
#include <utility>

#include "base/values.h"
#include "components/extensions/browser/event_router_forwarder.h"
#include "components/extensions/common/api/system_private.h"
#include "components/policy/core/common/policy_pref_names.h"
#include "components/prefs/pref_service.h"
#include "google_apis/google_api_keys.h"

namespace extensions {

namespace system_private = api::system_private;

ExtensionFunction::ResponseAction
SystemPrivateGetIncognitoModeAvailabilityFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction SystemPrivateGetUpdateStatusFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction SystemPrivateGetApiKeyFunction::Run() {
  return RespondNow(WithArguments(google_apis::GetAPIKey()));
}

}  // namespace extensions
