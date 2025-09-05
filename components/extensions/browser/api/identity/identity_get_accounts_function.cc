// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/identity/identity_get_accounts_function.h"

#include <memory>
#include <utility>
#include <vector>

#include "components/extensions/browser/api/identity/identity_api.h"
#include "components/extensions/browser/api/identity/identity_constants.h"
#include "components/extensions/common/api/identity.h"
#include "components/signin/public/identity_manager/account_info.h"
#include "components/signin/public/identity_manager/identity_manager.h"
#include "content/public/browser/browser_context.h"

namespace extensions {

IdentityGetAccountsFunction::IdentityGetAccountsFunction() {
}

IdentityGetAccountsFunction::~IdentityGetAccountsFunction() {
}

ExtensionFunction::ResponseAction IdentityGetAccountsFunction::Run() {
  if (browser_context()->IsOffTheRecord()) {
    return RespondNow(Error(identity_constants::kOffTheRecord));
  }

  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
