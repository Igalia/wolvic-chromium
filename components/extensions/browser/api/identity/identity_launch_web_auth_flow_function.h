// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_IDENTITY_IDENTITY_LAUNCH_WEB_AUTH_FLOW_FUNCTION_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_IDENTITY_IDENTITY_LAUNCH_WEB_AUTH_FLOW_FUNCTION_H_

#include <string>

#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"

namespace extensions {

// When enabled, abortOnLoadForNonInteractive and timeoutMsForNonInteractive
// arguments to launchWebAuthFlow will be used to allow the loaded auth page to
// wait before failing with an 'interaction required' error. This allows JS to
// run and redirects to happen after page load.
BASE_DECLARE_FEATURE(kNonInteractiveTimeoutForWebAuthFlow);

class IdentityLaunchWebAuthFlowFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("identity.launchWebAuthFlow",
                             EXPERIMENTAL_IDENTITY_LAUNCHWEBAUTHFLOW)

  // Used to track error state of the function call and for the histogram
  // exposure.
  // These values are persisted to logs. Entries should not be renumbered and
  // numeric values should never be reused.
  enum class Error {
    kNone = 0,
    kOffTheRecord = 1,
    kUserRejected = 2,
    kInteractionRequired = 3,
    kPageLoadFailure = 4,
    kUnexpectedError = 5,
    kPageLoadTimedOut = 6,
    kCannotCreateWindow = 7,
    kInvalidURLScheme = 8,
    kMaxValue = kInvalidURLScheme,
  };

  IdentityLaunchWebAuthFlowFunction();

 private:
  // ExtensionFunction:
  ~IdentityLaunchWebAuthFlowFunction() override;
  ResponseAction Run() override;
};

}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_IDENTITY_IDENTITY_LAUNCH_WEB_AUTH_FLOW_FUNCTION_H_
