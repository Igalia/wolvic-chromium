// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/identity/identity_remove_cached_auth_token_function.h"

#include "components/extensions/browser/api/identity/identity_api.h"
#include "components/extensions/browser/api/identity/identity_constants.h"
#include "components/extensions/common/api/identity.h"

namespace extensions {

IdentityRemoveCachedAuthTokenFunction::IdentityRemoveCachedAuthTokenFunction() {
}

IdentityRemoveCachedAuthTokenFunction::
    ~IdentityRemoveCachedAuthTokenFunction() {}

ExtensionFunction::ResponseAction IdentityRemoveCachedAuthTokenFunction::Run() {
  if (browser_context()->IsOffTheRecord())
    return RespondNow(Error(identity_constants::kOffTheRecord));

  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
