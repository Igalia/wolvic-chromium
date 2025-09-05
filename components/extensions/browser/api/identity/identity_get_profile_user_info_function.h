// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_IDENTITY_IDENTITY_GET_PROFILE_USER_INFO_FUNCTION_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_IDENTITY_IDENTITY_GET_PROFILE_USER_INFO_FUNCTION_H_

#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"

namespace extensions {

class IdentityGetProfileUserInfoFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("identity.getProfileUserInfo",
                             IDENTITY_GETPROFILEUSERINFO)

  IdentityGetProfileUserInfoFunction();

 private:
  ~IdentityGetProfileUserInfoFunction() override;

  // ExtensionFunction implementation.
  ExtensionFunction::ResponseAction Run() override;
};

}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_IDENTITY_IDENTITY_GET_PROFILE_USER_INFO_FUNCTION_H_
