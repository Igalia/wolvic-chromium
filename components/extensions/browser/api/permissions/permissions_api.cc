// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/permissions/permissions_api.h"

#include <memory>
#include <utility>

#include "base/functional/bind.h"
#include "base/notreached.h"
#include "components/extensions/common/api/permissions.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/common/error_utils.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_handlers/permissions_parser.h"
#include "extensions/common/permissions/permission_message_provider.h"
#include "extensions/common/permissions/permissions_data.h"
#include "extensions/common/permissions/permissions_info.h"
#include "extensions/common/url_pattern_set.h"

namespace extensions {

using api::permissions::Permissions;

namespace {

PermissionsRequestFunction::DialogAction g_dialog_action =
    PermissionsRequestFunction::DialogAction::kDefault;
PermissionsRequestFunction* g_pending_request_function = nullptr;
bool ignore_user_gesture_for_tests = false;

}  // namespace

ExtensionFunction::ResponseAction PermissionsContainsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction PermissionsGetAllFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction PermissionsRemoveFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// static
base::AutoReset<PermissionsRequestFunction::DialogAction>
PermissionsRequestFunction::SetDialogActionForTests(
    DialogAction dialog_action) {
  return base::AutoReset<PermissionsRequestFunction::DialogAction>(
      &g_dialog_action, dialog_action);
}

// static
void PermissionsRequestFunction::ResolvePendingDialogForTests(
    bool accept_dialog) {
  CHECK(g_pending_request_function);
  PermissionsRequestFunction* pending_function = g_pending_request_function;
  // Clear out the pending function now. After Release() below, it's unsafe to
  // use.
  g_pending_request_function = nullptr;

  // TODO(mshin): Enable the below code after migrating ExtensionInstallPrompt 
  // ExtensionInstallPrompt::DoneCallbackPayload result(
  //     accept_dialog ? ExtensionInstallPrompt::Result::ACCEPTED
  //                   : ExtensionInstallPrompt::Result::USER_CANCELED);
  // pending_function->OnInstallPromptDone(result);
  pending_function->Release();  // Balanced in Run().
}

// static
void PermissionsRequestFunction::SetIgnoreUserGestureForTests(
    bool ignore) {
  ignore_user_gesture_for_tests = ignore;
}

PermissionsRequestFunction::PermissionsRequestFunction() {}

PermissionsRequestFunction::~PermissionsRequestFunction() {
  CHECK_NE(g_pending_request_function, this)
      << "Pending request function was never resolved!";
}

ExtensionFunction::ResponseAction PermissionsRequestFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

bool PermissionsRequestFunction::ShouldKeepWorkerAliveIndefinitely() {
  // `permissions.request()` may trigger a user prompt. In this case, we allow
  // the extension service worker to be kept alive past the typical 5 minute
  // limit per-task, since it may be blocked on user action.
  return true;
}

// void PermissionsRequestFunction::OnInstallPromptDone(
//     ExtensionInstallPrompt::DoneCallbackPayload payload) {
//   // This dialog doesn't support the "withhold permissions" checkbox.
//   DCHECK_NE(payload.result,
//             ExtensionInstallPrompt::Result::ACCEPTED_WITH_WITHHELD_PERMISSIONS);
//   if (payload.result != ExtensionInstallPrompt::Result::ACCEPTED) {
//     Respond(ArgumentList(api::permissions::Request::Results::Create(false)));
//     return;
//   }
//   PermissionsUpdater permissions_updater(browser_context());
//   requesting_withheld_permissions_ = !requested_withheld_->IsEmpty();
//   requesting_optional_permissions_ = !requested_optional_->IsEmpty();
//   if (requesting_withheld_permissions_) {
//     permissions_updater.GrantRuntimePermissions(
//         *extension(), *requested_withheld_,
//         base::BindOnce(&PermissionsRequestFunction::OnRuntimePermissionsGranted,
//                        this));
//   }
//   if (requesting_optional_permissions_) {
//     permissions_updater.GrantOptionalPermissions(
//         *extension(), *requested_optional_,
//         base::BindOnce(
//             &PermissionsRequestFunction::OnOptionalPermissionsGranted, this));
//   }

//   // Grant{Runtime|Optional}Permissions calls above can finish synchronously.
//   if (!did_respond())
//     RespondIfRequestsFinished();
// }

void PermissionsRequestFunction::OnRuntimePermissionsGranted() {
  requesting_withheld_permissions_ = false;
  RespondIfRequestsFinished();
}

void PermissionsRequestFunction::OnOptionalPermissionsGranted() {
  requesting_optional_permissions_ = false;
  RespondIfRequestsFinished();
}

void PermissionsRequestFunction::RespondIfRequestsFinished() {
  if (requesting_withheld_permissions_ || requesting_optional_permissions_)
    return;

  Respond(ArgumentList(api::permissions::Request::Results::Create(true)));
}

std::unique_ptr<const PermissionSet>
PermissionsRequestFunction::TakePromptedPermissionsForTesting() {
  return std::move(prompted_permissions_for_testing_);
}

}  // namespace extensions
