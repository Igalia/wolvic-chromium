// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/identity/identity_clear_all_cached_auth_tokens_function.h"

#include "components/extensions/browser/api/identity/identity_api.h"
#include "components/extensions/browser/api/identity/identity_constants.h"

namespace extensions {

IdentityClearAllCachedAuthTokensFunction::
    IdentityClearAllCachedAuthTokensFunction() = default;
IdentityClearAllCachedAuthTokensFunction::
    ~IdentityClearAllCachedAuthTokensFunction() = default;

ExtensionFunction::ResponseAction
IdentityClearAllCachedAuthTokensFunction::Run() {
  if (browser_context()->IsOffTheRecord())
    return RespondNow(Error(identity_constants::kOffTheRecord));

  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
