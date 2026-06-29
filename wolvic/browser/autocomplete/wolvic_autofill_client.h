// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WOLVIC_BROWSER_AUTOCOMPLETE_WOLVIC_AUTOFILL_CLIENT_H_
#define WOLVIC_BROWSER_AUTOCOMPLETE_WOLVIC_AUTOFILL_CLIENT_H_

#include <memory>
#include <string>
#include <vector>

#include "base/android/scoped_java_ref.h"
#include "base/functional/callback.h"
#include "base/memory/weak_ptr.h"
#include "components/autofill/content/browser/content_autofill_client.h"
#include "components/autofill/core/browser/foundations/autofill_client.h"
#include "components/autofill/core/browser/filling/filling_product.h"
#include "components/autofill/core/common/form_interactions_flow.h"

namespace payments {
class PaymentsClient;
}  // namespace payments

namespace wolvic {

class WolvicAutofillClient : public autofill::ContentAutofillClient {
 public:
  // Creates a new WolvicAutofillClient for the given `web_contents` if no
  // ContentAutofillClient is associated with the `web_contents` yet. Otherwise,
  // it's a no-op.
  static void CreateForWebContents(content::WebContents* web_contents);

  WolvicAutofillClient(const WolvicAutofillClient&) = delete;
  WolvicAutofillClient& operator=(const WolvicAutofillClient&) = delete;
  ~WolvicAutofillClient() override;

  // autofill::AutofillClient:
  base::WeakPtr<autofill::AutofillClient> GetWeakPtr() override;
  bool IsOffTheRecord() const override;
  scoped_refptr<network::SharedURLLoaderFactory>
  GetURLLoaderFactory() override;
  autofill::AutofillCrowdsourcingManager& GetCrowdsourcingManager() override;
  bool HasPersonalDataManager() const override;
  autofill::PersonalDataManager& GetPersonalDataManager() override;
  autofill::AutocompleteHistoryManager*
  GetAutocompleteHistoryManager() override;
  PrefService* GetPrefs() override;
  const PrefService* GetPrefs() const override;
  syncer::SyncService* GetSyncService() override;
  signin::IdentityManager* GetIdentityManager() override;
  const signin::IdentityManager* GetIdentityManager() const override;
  autofill::FormDataImporter* GetFormDataImporter() override;
  autofill::payments::PaymentsAutofillClient* GetPaymentsAutofillClient()
      override;
  strike_database::StrikeDatabase* GetStrikeDatabase() override;
  ukm::UkmRecorder* GetUkmRecorder() override;
  autofill::AddressNormalizer* GetAddressNormalizer() override;
  const GURL& GetLastCommittedPrimaryMainFrameURL() const override;
  url::Origin GetLastCommittedPrimaryMainFrameOrigin() const override;
  security_state::SecurityLevel GetSecurityLevelForUmaHistograms() override;
  const translate::LanguageState* GetLanguageState() override;
  translate::TranslateDriver* GetTranslateDriver() override;
  void ShowAutofillSettings(
      autofill::SuggestionType suggestion_type) override;
  void ConfirmSaveAddressProfile(
      const autofill::AutofillProfile& profile,
      const autofill::AutofillProfile* original_profile,
      SaveAddressBubbleType save_address_bubble_type,
      AddressProfileSavePromptCallback callback) override;
  autofill::AutofillClient::SuggestionUiSessionId ShowAutofillSuggestions(
      const PopupOpenArgs& open_args,
      base::WeakPtr<autofill::AutofillSuggestionDelegate> delegate) override;
  void UpdateAutofillDataListValues(
      base::span<const autofill::SelectOption> datalist) override;
  base::span<const autofill::Suggestion> GetAutofillSuggestions()
      const override;
  void UpdateAutofillSuggestions(
      const std::vector<autofill::Suggestion>& suggestions,
      autofill::FillingProduct main_filling_product,
      autofill::AutofillSuggestionTriggerSource trigger_source) override;
  void HideAutofillSuggestions(
      autofill::SuggestionHidingReason reason) override;

  const std::string& GetAppLocale() const override;
  autofill::VotesUploader& GetVotesUploader() override;
  autofill::EntityDataManager* GetEntityDataManager() override;
  autofill::ValuablesDataManager* GetValuablesDataManager() override;
  credential_management::ContentCredentialManager* GetContentCredentialManager()
      override;
  autofill::SingleFieldFillRouter& GetSingleFieldFillRouter() override;
  bool IsAutofillEnabled() const override;
  bool IsAutofillProfileEnabled() const override;
  void DidFillForm(autofill::AutofillTriggerSource trigger_source,
                   bool is_refill) override;
  autofill::autofill_metrics::FormInteractionsUkmLogger&
  GetFormInteractionsUkmLogger() override;

  bool IsAutocompleteEnabled() const override;
  bool IsPasswordManagerEnabled() const override;
  bool IsContextSecure() const override;
  bool IsWalletStorageEnabled() const override;
  autofill::FormInteractionsFlowId GetCurrentFormInteractionsFlowId() override;

  // autofill::ContentAutofillClient:
  std::unique_ptr<autofill::AutofillManager> CreateManager(
      base::PassKey<autofill::ContentAutofillDriver> pass_key,
      autofill::ContentAutofillDriver& driver) override;

  void OnLoginSelected(JNIEnv* env, jint index);

 protected:
  explicit WolvicAutofillClient(content::WebContents* web_contents);

 private:
  content::WebContents* web_contents() const { return web_contents_; }
  void CreatJavaArrayFromSuggestions(JNIEnv* env);

  // These members are initialized lazily in their respective getters.
  // Therefore, do not access the members directly.
  std::unique_ptr<autofill::AutofillCrowdsourcingManager> crowdsourcing_manager_;

  autofill::FormInteractionsFlowId flow_id_{};
  base::Time flow_id_date_;
  raw_ptr<content::WebContents> web_contents_;
  std::vector<autofill::Suggestion> suggestions_;
  autofill::AutofillSuggestionTriggerSource trigger_source_{
      autofill::AutofillSuggestionTriggerSource::kUnspecified};
  base::WeakPtr<autofill::AutofillSuggestionDelegate> delegate_;
  base::android::ScopedJavaGlobalRef<jobject> java_obj_;

  base::WeakPtrFactory<WolvicAutofillClient> weak_ptr_factory_{this};
};

}  // namespace wolvic

#endif  // WOLVIC_BROWSER_AUTOCOMPLETE_WOLVIC_AUTOFILL_CLIENT_H_
