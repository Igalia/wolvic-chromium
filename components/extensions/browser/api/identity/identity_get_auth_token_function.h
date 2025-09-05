// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_IDENTITY_IDENTITY_GET_AUTH_TOKEN_FUNCTION_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_IDENTITY_IDENTITY_GET_AUTH_TOKEN_FUNCTION_H_

#include <memory>
#include <optional>
#include <set>
#include <string>

#include "base/callback_list.h"
#include "base/feature_list.h"
#include "base/gtest_prod_util.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "base/time/time.h"
#include "components/extensions/common/api/identity.h"
#include "components/signin/public/identity_manager/identity_manager.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"
#include "google_apis/gaia/google_service_auth_error.h"
#include "google_apis/gaia/oauth2_mint_token_flow.h"

namespace signin {
class AccessTokenFetcher;
struct AccessTokenInfo;
}  // namespace signin

namespace extensions {
class IdentityGetAuthTokenError;

// Exposed for testing
inline constexpr base::TimeDelta kGetAuthTokenInactivityTime =
    base::Minutes(10);

// identity.getAuthToken fetches an OAuth 2 function for the
// caller. The request has three sub-flows: non-interactive,
// interactive, and sign-in.
//
// In the non-interactive flow, getAuthToken requests a token from
// GAIA. GAIA may respond with a token, an error, or "consent
// required". In the consent required cases, getAuthToken proceeds to
// the second, interactive phase.
//
// The interactive flow presents a scope approval dialog to the
// user. If the user approves the request, a grant will be recorded on
// the server, and an access token will be returned to the caller.
//
// In some cases we need to display a sign-in dialog. Normally the
// profile will be signed in already, but if it turns out we need a
// new login token, there is a sign-in flow. If that flow completes
// successfully, getAuthToken proceeds to the non-interactive flow.
class IdentityGetAuthTokenFunction : public ExtensionFunction {
 public:

  DECLARE_EXTENSION_FUNCTION("identity.getAuthToken",
                             EXPERIMENTAL_IDENTITY_GETAUTHTOKEN)

  IdentityGetAuthTokenFunction();

  void OnIdentityAPIShutdown();

 protected:
  ~IdentityGetAuthTokenFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

  base::WeakPtrFactory<IdentityGetAuthTokenFunction> weak_ptr_factory_{this};
};

}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_IDENTITY_IDENTITY_GET_AUTH_TOKEN_FUNCTION_H_
