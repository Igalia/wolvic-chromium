// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_DASHBOARD_PRIVATE_DASHBOARD_PRIVATE_API_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_DASHBOARD_PRIVATE_DASHBOARD_PRIVATE_API_H_

#include <memory>
#include <optional>
#include <string>

#include "components/extensions/common/api/dashboard_private.h"
#include "extensions/browser/extension_function.h"

class SkBitmap;

namespace extensions {

class Extension;

class DashboardPrivateShowPermissionPromptForDelegatedInstallFunction
    : public ExtensionFunction /*,
      public WebstoreInstallHelper::Delegate */ {
 public:
  DECLARE_EXTENSION_FUNCTION(
      "dashboardPrivate.showPermissionPromptForDelegatedInstall",
      DASHBOARDPRIVATE_SHOWPERMISSIONPROMPTFORDELEGATEDINSTALL)

  DashboardPrivateShowPermissionPromptForDelegatedInstallFunction();

  DashboardPrivateShowPermissionPromptForDelegatedInstallFunction(
      const DashboardPrivateShowPermissionPromptForDelegatedInstallFunction&) =
      delete;
  DashboardPrivateShowPermissionPromptForDelegatedInstallFunction& operator=(
      const DashboardPrivateShowPermissionPromptForDelegatedInstallFunction&) =
      delete;

 private:
  using Params =
     api::dashboard_private::ShowPermissionPromptForDelegatedInstall::Params;

  ~DashboardPrivateShowPermissionPromptForDelegatedInstallFunction() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;

  // TODO(mshin): Enable the below code after migrating WebstoreInstallHelper
  // WebstoreInstallHelper::Delegate:
  // void OnWebstoreParseSuccess(const std::string& id,
  //                             const SkBitmap& icon,
  //                             base::Value::Dict parsed_manifest) override;
  // void OnWebstoreParseFailure(const std::string& id,
  //                             InstallHelperResultCode result,
  //                             const std::string& error_message) override;

  // void OnInstallPromptDone(ExtensionInstallPrompt::DoneCallbackPayload payload);

  ExtensionFunction::ResponseValue BuildResponse(
      api::dashboard_private::Result result,
      const std::string& error);

  const Params::Details& details() const { return params_->details; }

  std::optional<Params> params_;

  // A dummy Extension object we create for the purposes of using
  // ExtensionInstallPrompt to prompt for confirmation of the install.
  scoped_refptr<Extension> dummy_extension_;

  // TODO(mshin): Enable the below code after migrating ExtensionInstallPrompt
  // std::unique_ptr<ExtensionInstallPrompt> install_prompt_;
};

}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_DASHBOARD_PRIVATE_DASHBOARD_PRIVATE_API_H_
