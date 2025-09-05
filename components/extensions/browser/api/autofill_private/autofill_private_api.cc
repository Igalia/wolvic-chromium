// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/autofill_private/autofill_private_api.h"

namespace extensions {

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateGetAccountInfoFunction

ExtensionFunction::ResponseAction AutofillPrivateGetAccountInfoFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateSaveAddressFunction

ExtensionFunction::ResponseAction AutofillPrivateSaveAddressFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateGetCountryListFunction

ExtensionFunction::ResponseAction AutofillPrivateGetCountryListFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateGetAddressComponentsFunction

ExtensionFunction::ResponseAction
AutofillPrivateGetAddressComponentsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateGetAddressListFunction

ExtensionFunction::ResponseAction AutofillPrivateGetAddressListFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateSaveCreditCardFunction

ExtensionFunction::ResponseAction AutofillPrivateSaveCreditCardFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateRemoveEntryFunction

ExtensionFunction::ResponseAction AutofillPrivateRemoveEntryFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateGetCreditCardListFunction

ExtensionFunction::ResponseAction
AutofillPrivateGetCreditCardListFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateMigrateCreditCardsFunction

ExtensionFunction::ResponseAction
AutofillPrivateMigrateCreditCardsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateLogServerCardLinkClickedFunction

ExtensionFunction::ResponseAction
AutofillPrivateLogServerCardLinkClickedFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateLogServerIbanLinkClickedFunction

ExtensionFunction::ResponseAction
AutofillPrivateLogServerIbanLinkClickedFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateSetCreditCardFIDOAuthEnabledStateFunction

ExtensionFunction::ResponseAction
AutofillPrivateSetCreditCardFIDOAuthEnabledStateFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateSaveIbanFunction

ExtensionFunction::ResponseAction AutofillPrivateSaveIbanFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateGetIbanListFunction

ExtensionFunction::ResponseAction AutofillPrivateGetIbanListFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateIsValidIbanFunction

ExtensionFunction::ResponseAction AutofillPrivateIsValidIbanFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateAddVirtualCardFunction

ExtensionFunction::ResponseAction AutofillPrivateAddVirtualCardFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateRemoveVirtualCardFunction

ExtensionFunction::ResponseAction
AutofillPrivateRemoveVirtualCardFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateAuthenticateUserAndFlipMandatoryAuthToggleFunction

ExtensionFunction::ResponseAction
AutofillPrivateAuthenticateUserAndFlipMandatoryAuthToggleFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// Update the Mandatory auth toggle pref and log whether the auth was successful
// or not.
void AutofillPrivateAuthenticateUserAndFlipMandatoryAuthToggleFunction::
    UpdateMandatoryAuthTogglePref(bool reauth_succeeded) {
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateGetLocalCardFunction

ExtensionFunction::ResponseAction AutofillPrivateGetLocalCardFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// This is triggered after the reauth is completed and a local card may be
// returned based on the auth result. We also log whether the auth was
// successful or not.
void AutofillPrivateGetLocalCardFunction::OnReauthFinished(bool can_retrieve) {
}

void AutofillPrivateGetLocalCardFunction::ReturnCreditCard() {
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateCheckIfDeviceAuthAvailableFunction

ExtensionFunction::ResponseAction
AutofillPrivateCheckIfDeviceAuthAvailableFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateBulkDeleteAllCvcsFunction

ExtensionFunction::ResponseAction
AutofillPrivateBulkDeleteAllCvcsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

////////////////////////////////////////////////////////////////////////////////
// AutofillPrivateSetAutofillSyncToggleEnabledFunction

ExtensionFunction::ResponseAction
AutofillPrivateSetAutofillSyncToggleEnabledFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
