// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/browser/autocomplete/wolvic_autofill_manager.h"

#include "components/autofill/core/browser/foundations/autofill_manager.h"
#include "components/autofill/core/browser/integrators/password_manager/password_manager_delegate.h"
#include "components/autofill/core/common/password_form_fill_data.h"

namespace wolvic {

WolvicAutofillManager::WolvicAutofillManager(autofill::AutofillDriver* driver)
    : autofill::AutofillManager(driver) {}

WolvicAutofillManager::~WolvicAutofillManager() = default;

base::WeakPtr<autofill::AutofillManager> WolvicAutofillManager::GetWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

bool WolvicAutofillManager::ShouldClearPreviewedForm() {
  return false;
}

bool WolvicAutofillManager::ShouldParseForms() {
  return true;
}

autofill::CreditCardAccessManager*
WolvicAutofillManager::GetCreditCardAccessManager() {
  return nullptr;
}

const autofill::CreditCardAccessManager*
WolvicAutofillManager::GetCreditCardAccessManager() const {
  return nullptr;
}

void WolvicAutofillManager::OnAskForValuesToFillImpl(
    const autofill::FormData& form,
    const autofill::FieldGlobalId& field_id,
    const gfx::Rect& caret_bounds,
    autofill::AutofillSuggestionTriggerSource trigger_source,
    std::optional<autofill::PasswordSuggestionRequest> password_request) {
  if (!password_request.has_value())
    return;
  // Surface password suggestions through the client's popup
  // (WolvicAutofillClient::ShowAutofillSuggestions) via the delegate's dropdown
  // path; Wolvic has no keyboard-replacing surface.
  autofill::PasswordManagerDelegate* delegate =
      client().GetPasswordManagerDelegate(field_id);
  if (delegate) {
    delegate->ShowSuggestions(password_request->field);
  }
}

}  // namespace wolvic
