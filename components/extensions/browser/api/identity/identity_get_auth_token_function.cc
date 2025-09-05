// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/identity/identity_get_auth_token_function.h"

#include <set>
#include <vector>

#include "base/functional/bind.h"
#include "base/location.h"
#include "base/metrics/field_trial_params.h"
#include "base/metrics/histogram_functions.h"
#include "base/notreached.h"
#include "base/strings/strcat.h"
#include "base/strings/string_piece.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"
#include "components/extensions/browser/api/identity/identity_api.h"
#include "components/prefs/pref_service.h"
#include "components/signin/public/base/signin_pref_names.h"
#include "components/signin/public/identity_manager/access_token_info.h"
#include "components/signin/public/identity_manager/accounts_in_cookie_jar_info.h"
#include "components/signin/public/identity_manager/scope_set.h"
#include "components/version_info/version_info.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/common/api/oauth2.h"
#include "extensions/common/manifest_handlers/oauth2_manifest_handler.h"
#include "google_apis/gaia/gaia_auth_util.h"
#include "google_apis/gaia/gaia_urls.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "ui/base/idle/idle.h"

namespace extensions {

IdentityGetAuthTokenFunction::IdentityGetAuthTokenFunction() = default;

IdentityGetAuthTokenFunction::~IdentityGetAuthTokenFunction() {
  TRACE_EVENT_NESTABLE_ASYNC_END0("identity", "IdentityGetAuthTokenFunction",
                                  this);
}

ExtensionFunction::ResponseAction IdentityGetAuthTokenFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
