// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WOLVIC_BROWSER_AUTOCOMPLETE_WOLVIC_AUTOFILL_MANAGER_H_
#define WOLVIC_BROWSER_AUTOCOMPLETE_WOLVIC_AUTOFILL_MANAGER_H_

#include "components/autofill/core/browser/foundations/autofill_manager.h"

namespace wolvic {

class WolvicAutofillManager : public autofill::AutofillManager {
 public:
  explicit WolvicAutofillManager(autofill::AutofillDriver* driver);

  WolvicAutofillManager(const WolvicAutofillManager&) = delete;
  WolvicAutofillManager& operator=(const WolvicAutofillManager&) = delete;

  ~WolvicAutofillManager() override;

  // autofill::AutofillManager:
  base::WeakPtr<AutofillManager> GetWeakPtr() override;
  bool ShouldClearPreviewedForm() override;
  autofill::CreditCardAccessManager* GetCreditCardAccessManager() override;
  const autofill::CreditCardAccessManager* GetCreditCardAccessManager()
      const override;

  void OnFocusOnNonFormFieldImpl() override {}

  void OnDidAutofillFormImpl(const autofill::FormData& form) override {}

  void OnDidEndTextFieldEditingImpl() override {}
  void OnHidePopupImpl() override {}
  void OnSelectFieldOptionsDidChangeImpl(
      const autofill::FormData& form,
      const autofill::FieldGlobalId& field_id) override {}

  void Reset() override {}

  void ReportAutofillWebOTPMetrics(bool used_web_otp) override {}

  void FillOrPreviewField(
      autofill::mojom::ActionPersistence action_persistence,
      autofill::mojom::FieldActionType action_type,
      const autofill::FormData& form,
      const autofill::FormFieldData& field,
      const std::u16string& value,
      autofill::FillingProduct filling_product,
      std::optional<autofill::FieldType> field_type_used) override {}

 protected:
  void OnFormSubmittedImpl(const autofill::FormData& form,
                           autofill::mojom::SubmissionSource source) override {}

  void OnCaretMovedInFormFieldImpl(const autofill::FormData& form,
                                   const autofill::FieldGlobalId& field_id,
                                   const gfx::Rect& caret_bounds) override {}

  void OnTextFieldValueChangedImpl(const autofill::FormData& form,
                                   const autofill::FieldGlobalId& field_id,
                                   const base::TimeTicks timestamp) override {}

  void OnTextFieldDidScrollImpl(const autofill::FormData& form,
                                const autofill::FieldGlobalId& field_id) override {}

  void OnAskForValuesToFillImpl(
      const autofill::FormData& form,
      const autofill::FieldGlobalId& field_id,
      const gfx::Rect& caret_bounds,
      autofill::AutofillSuggestionTriggerSource trigger_source,
      std::optional<autofill::PasswordSuggestionRequest> password_request)
      override;

  void OnFocusOnFormFieldImpl(const autofill::FormData& form,
                              const autofill::FieldGlobalId& field_id) override {}

  void OnSelectControlSelectionChangedImpl(const autofill::FormData& form,
                                           const autofill::FieldGlobalId& field_id) override {}

  void OnJavaScriptChangedAutofilledValueImpl(
      const autofill::FormData& form,
      const autofill::FieldGlobalId& field_id,
      const std::u16string& old_value) override {}

  bool ShouldParseForms() override;

  void OnBeforeProcessParsedForms() override {}

  void OnFormProcessed(const autofill::FormData& form,
                       const autofill::FormStructure& form_structure) override {
  }

  void OnLoadedServerPredictionsImpl(
      base::span<const raw_ref<autofill::FormStructure>> forms) override {}

  void SuppressAutomaticRefillsImpl(
      const autofill::FillId& fill_id) override {}
  void RequestRefillImpl(const autofill::FillId& fill_id) override {}

 private:
  base::WeakPtrFactory<WolvicAutofillManager> weak_ptr_factory_{this};
};

}  // namespace wolvic

#endif  // WOLVIC_BROWSER_AUTOCOMPLETE_WOLVIC_AUTOFILL_MANAGER_H_
