// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/dashboard_private/dashboard_private_api.h"

#include <memory>
#include <utility>

#include "base/functional/bind.h"
#include "base/memory/scoped_refptr.h"
#include "base/values.h"
#include "chrome/browser/bitmap_fetcher/bitmap_fetcher.h"
#include "chrome/browser/profiles/profile.h"
#include "components/crx_file/id_util.h"
#include "content/public/browser/storage_partition.h"
#include "extensions/common/extension.h"
#include "net/base/load_flags.h"
#include "url/gurl.h"

namespace extensions {

namespace ShowPermissionPromptForDelegatedInstall =
    api::dashboard_private::ShowPermissionPromptForDelegatedInstall;

DashboardPrivateShowPermissionPromptForDelegatedInstallFunction::
    DashboardPrivateShowPermissionPromptForDelegatedInstallFunction() {
}

DashboardPrivateShowPermissionPromptForDelegatedInstallFunction::
    ~DashboardPrivateShowPermissionPromptForDelegatedInstallFunction() {
}

ExtensionFunction::ResponseAction
DashboardPrivateShowPermissionPromptForDelegatedInstallFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// TODO(mshin): Enable the below code after migrating WebstoreInstallHelper
// void DashboardPrivateShowPermissionPromptForDelegatedInstallFunction::
//     OnWebstoreParseSuccess(const std::string& id,
//                            const SkBitmap& icon,
//                            base::Value::Dict parsed_manifest) {
//   CHECK_EQ(params_->details.id, id);

//   std::string localized_name = params_->details.localized_name ?
//       *params_->details.localized_name : std::string();

//   std::string error;
//   dummy_extension_ = ExtensionInstallPrompt::GetLocalizedExtensionForDisplay(
//       parsed_manifest, Extension::FROM_WEBSTORE, id, localized_name,
//       std::string(), &error);

//   if (!dummy_extension_.get()) {
//     OnWebstoreParseFailure(params_->details.id,
//                            WebstoreInstallHelper::Delegate::MANIFEST_ERROR,
//                            kDashboardInvalidManifestError);
//     return;
//   }

//   content::WebContents* web_contents = GetSenderWebContents();
//   if (!web_contents) {
//     // The browser window has gone away.
//     Respond(BuildResponse(api::dashboard_private::Result::kUserCancelled,
//                           kDashboardUserCancelledError));
//     // Matches the AddRef in Run().
//     Release();
//     return;
//   }
//   std::unique_ptr<ExtensionInstallPrompt::Prompt> prompt(
//       new ExtensionInstallPrompt::Prompt(
//           ExtensionInstallPrompt::DELEGATED_PERMISSIONS_PROMPT));
//   prompt->set_delegated_username(details().delegated_user);

//   install_prompt_ = std::make_unique<ExtensionInstallPrompt>(web_contents);
//   install_prompt_->ShowDialog(
//       base::BindOnce(
//           &DashboardPrivateShowPermissionPromptForDelegatedInstallFunction::
//               OnInstallPromptDone,
//           this),
//       dummy_extension_.get(), &icon, std::move(prompt),
//       ExtensionInstallPrompt::GetDefaultShowDialogCallback());
//   // Control flow finishes up in OnInstallPromptDone().
// }

// void DashboardPrivateShowPermissionPromptForDelegatedInstallFunction::
//     OnWebstoreParseFailure(
//     const std::string& id,
//     WebstoreInstallHelper::Delegate::InstallHelperResultCode result,
//     const std::string& error_message) {
//   CHECK_EQ(params_->details.id, id);

//   Respond(BuildResponse(WebstoreInstallHelperResultToDashboardApiResult(result),
//                         error_message));

//   // Matches the AddRef in Run().
//   Release();
// }

// void DashboardPrivateShowPermissionPromptForDelegatedInstallFunction::
//     OnInstallPromptDone(ExtensionInstallPrompt::DoneCallbackPayload payload) {
//   // TODO(crbug.com/984069): Handle `ACCEPTED_WITH_WITHHELD_PERMISSIONS` when it
//   // is supported for this case.
//   DCHECK_NE(payload.result,
//             ExtensionInstallPrompt::Result::ACCEPTED_WITH_WITHHELD_PERMISSIONS);
//   bool accepted = (payload.result == ExtensionInstallPrompt::Result::ACCEPTED);
//   Respond(
//       BuildResponse(accepted ? api::dashboard_private::Result::kEmptyString
//                              : api::dashboard_private::Result::kUserCancelled,
//                     accepted ? std::string() : kDashboardUserCancelledError));

//   Release();  // Matches the AddRef in Run().
// }

ExtensionFunction::ResponseValue
DashboardPrivateShowPermissionPromptForDelegatedInstallFunction::BuildResponse(
    api::dashboard_private::Result result, const std::string& error) {
  // The web store expects an empty string on success.
  auto args = ShowPermissionPromptForDelegatedInstall::Results::Create(result);
  if (result == api::dashboard_private::Result::kEmptyString) {
    return ArgumentList(std::move(args));
  }
  return ErrorWithArguments(std::move(args), error);
}

}  // namespace extensions
