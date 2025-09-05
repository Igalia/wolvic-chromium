// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/passwords_private/passwords_private_api.h"

#include <optional>

#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/location.h"
#include "base/metrics/histogram_macros.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/single_thread_task_runner.h"
#include "base/values.h"
#include "components/extensions/common/api/passwords_private.h"
#include "components/password_manager/core/browser/manage_passwords_referrer.h"
#include "components/password_manager/core/browser/password_manager_util.h"
#include "components/password_manager/core/common/password_manager_pref_names.h"
#include "components/prefs/pref_service.h"
#include "components/sync/service/sync_service.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/extension_function_registry.h"

namespace extensions {

namespace {

using ResponseAction = ExtensionFunction::ResponseAction;

}

// PasswordsPrivateRecordPasswordsPageAccessInSettingsFunction
ResponseAction
PasswordsPrivateRecordPasswordsPageAccessInSettingsFunction::Run() {
  UMA_HISTOGRAM_ENUMERATION(
      "PasswordManager.ManagePasswordsReferrer",
      password_manager::ManagePasswordsReferrer::kChromeSettings);
  return RespondNow(NoArguments());
}

// PasswordsPrivateChangeCredentialFunction
ResponseAction PasswordsPrivateChangeCredentialFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateRemoveCredentialFunction
ResponseAction PasswordsPrivateRemoveCredentialFunction::Run() {
  return RespondNow(Error("Not Implement"));;
}

// PasswordsPrivateRemovePasswordExceptionFunction
ResponseAction PasswordsPrivateRemovePasswordExceptionFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateUndoRemoveSavedPasswordOrExceptionFunction
ResponseAction
PasswordsPrivateUndoRemoveSavedPasswordOrExceptionFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateRequestPlaintextPasswordFunction
ResponseAction PasswordsPrivateRequestPlaintextPasswordFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateRequestCredentialDetailsFunction
ResponseAction PasswordsPrivateRequestCredentialsDetailsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateGetSavedPasswordListFunction
ResponseAction PasswordsPrivateGetSavedPasswordListFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateGetCredentialGroupsFunction
ResponseAction PasswordsPrivateGetCredentialGroupsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateGetPasswordExceptionListFunction
ResponseAction PasswordsPrivateGetPasswordExceptionListFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateMovePasswordToAccountFunction
ResponseAction PasswordsPrivateMovePasswordsToAccountFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateFetchFamilyMembersFunction
ResponseAction PasswordsPrivateFetchFamilyMembersFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateSharePasswordFunction
ResponseAction PasswordsPrivateSharePasswordFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void PasswordsPrivateFetchFamilyMembersFunction::FamilyFetchCompleted(
    const api::passwords_private::FamilyFetchResults& result) {
  Respond(ArgumentList(
      api::passwords_private::FetchFamilyMembers::Results::Create(result)));
}

// PasswordsPrivateImportPasswordsFunction
ResponseAction PasswordsPrivateImportPasswordsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void PasswordsPrivateImportPasswordsFunction::ImportRequestCompleted(
    const api::passwords_private::ImportResults& result) {
  Respond(ArgumentList(
      api::passwords_private::ImportPasswords::Results::Create(result)));
}

// PasswordsPrivateContinueImportFunction
ResponseAction PasswordsPrivateContinueImportFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void PasswordsPrivateContinueImportFunction::ImportCompleted(
    const api::passwords_private::ImportResults& result) {
  Respond(ArgumentList(
      api::passwords_private::ImportPasswords::Results::Create(result)));
}

// PasswordsPrivateResetImporterFunction
ResponseAction PasswordsPrivateResetImporterFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateExportPasswordsFunction
ResponseAction PasswordsPrivateExportPasswordsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void PasswordsPrivateExportPasswordsFunction::ExportRequestCompleted(
    const std::string& error) {
  if (error.empty()) {
    Respond(NoArguments());
  } else {
    Respond(Error(error));
  }
}

// PasswordsPrivateRequestExportProgressStatusFunction
ResponseAction PasswordsPrivateRequestExportProgressStatusFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateIsOptedInForAccountStorageFunction
ResponseAction PasswordsPrivateIsOptedInForAccountStorageFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateOptInForAccountStorageFunction
ResponseAction PasswordsPrivateOptInForAccountStorageFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateGetInsecureCredentialsFunction:
PasswordsPrivateGetInsecureCredentialsFunction::
    ~PasswordsPrivateGetInsecureCredentialsFunction() = default;

ResponseAction PasswordsPrivateGetInsecureCredentialsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateGetCredentialsWithReusedPasswordFunction:
PasswordsPrivateGetCredentialsWithReusedPasswordFunction::
    ~PasswordsPrivateGetCredentialsWithReusedPasswordFunction() = default;

ResponseAction PasswordsPrivateGetCredentialsWithReusedPasswordFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateMuteInsecureCredentialFunction:
PasswordsPrivateMuteInsecureCredentialFunction::
    ~PasswordsPrivateMuteInsecureCredentialFunction() = default;

ResponseAction PasswordsPrivateMuteInsecureCredentialFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateUnmuteInsecureCredentialFunction:
PasswordsPrivateUnmuteInsecureCredentialFunction::
    ~PasswordsPrivateUnmuteInsecureCredentialFunction() = default;

ResponseAction PasswordsPrivateUnmuteInsecureCredentialFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateStartPasswordCheckFunction:
PasswordsPrivateStartPasswordCheckFunction::
    ~PasswordsPrivateStartPasswordCheckFunction() = default;

ResponseAction PasswordsPrivateStartPasswordCheckFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void PasswordsPrivateStartPasswordCheckFunction::OnStarted(
    password_manager::BulkLeakCheckService::State state) {
  const bool is_running =
      state == password_manager::BulkLeakCheckService::State::kRunning;
  Respond(is_running ? NoArguments()
                     : Error("Starting password check failed."));
}

// PasswordsPrivateGetPasswordCheckStatusFunction:
PasswordsPrivateGetPasswordCheckStatusFunction::
    ~PasswordsPrivateGetPasswordCheckStatusFunction() = default;

ResponseAction PasswordsPrivateGetPasswordCheckStatusFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateIsAccountStoreDefaultFunction
ResponseAction PasswordsPrivateIsAccountStoreDefaultFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateGetUrlCollectionFunction:
ResponseAction PasswordsPrivateGetUrlCollectionFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateAddPasswordFunction
ResponseAction PasswordsPrivateAddPasswordFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateExtendAuthValidityFunction
ResponseAction PasswordsPrivateExtendAuthValidityFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateSwitchBiometricAuthBeforeFillingStateFunction
ResponseAction
PasswordsPrivateSwitchBiometricAuthBeforeFillingStateFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateShowExportedFileInShellFunction
ResponseAction PasswordsPrivateShowExportedFileInShellFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// PasswordsPrivateShowAddShortcutDialogFunction
ResponseAction PasswordsPrivateShowAddShortcutDialogFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
