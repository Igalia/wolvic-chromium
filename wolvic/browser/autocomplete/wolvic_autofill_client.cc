// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/browser/autocomplete/wolvic_autofill_client.h"

#include <utility>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/check.h"
#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "components/autofill/content/browser/content_autofill_driver.h"
#include "components/autofill/content/browser/content_autofill_driver_factory.h"
#include "components/autofill/core/browser/autofill_type.h"
#include "components/autofill/core/browser/country_type.h"
#include "components/autofill/core/browser/data_model/autofill_offer_data.h"
#include "components/autofill/core/browser/filling_product.h"
#include "components/autofill/core/browser/form_data_importer.h"
#include "components/autofill/core/browser/form_structure.h"
#include "components/autofill/core/common/form_interactions_flow.h"
#include "components/autofill/core/common/unique_ids.h"
#include "components/password_manager/content/browser/content_password_manager_driver.h"
#include "components/password_manager/core/browser/password_manager_settings_service.h"
#include "components/password_manager/core/browser/password_manager_util.h"
#include "components/password_manager/core/browser/password_requirements_service.h"
#include "components/prefs/pref_service.h"
#include "components/security_state/core/security_state.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/ssl_status.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "ui/android/window_android.h"
#include "url/origin.h"
#include "wolvic/browser/autocomplete/wolvic_autofill_manager.h"
#include "wolvic/browser/autocomplete/wolvic_password_form_util.h"
#include "wolvic/jni_headers/AutofillManager_jni.h"
#include "wolvic/wolvic_browser_context.h"
#include "wolvic/wolvic_content_browser_client.h"

using base::android::AttachCurrentThread;

namespace wolvic {

// static
void WolvicAutofillClient::CreateForWebContents(
    content::WebContents* web_contents) {
  DCHECK(web_contents);
  if (!FromWebContents(web_contents)) {
    web_contents->SetUserData(
        UserDataKey(),
        base::WrapUnique(new WolvicAutofillClient(web_contents)));
  }
}

WolvicAutofillClient::~WolvicAutofillClient() = default;

base::WeakPtr<autofill::AutofillClient> WolvicAutofillClient::GetWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

bool WolvicAutofillClient::IsOffTheRecord() const {
  return web_contents()->GetBrowserContext()->IsOffTheRecord();
}

scoped_refptr<network::SharedURLLoaderFactory>
WolvicAutofillClient::GetURLLoaderFactory() {
  return web_contents()
      ->GetBrowserContext()
      ->GetDefaultStoragePartition()
      ->GetURLLoaderFactoryForBrowserProcess();
}

autofill::AutofillCrowdsourcingManager*
WolvicAutofillClient::GetCrowdsourcingManager() {
  if (!crowdsourcing_manager_) {
    // Lazy initialization to avoid virtual function calls in the constructor.
    crowdsourcing_manager_ =
        std::make_unique<autofill::AutofillCrowdsourcingManager>(
            this, GetChannel(), GetLogManager());
  }
  return crowdsourcing_manager_.get();
}

autofill::PersonalDataManager* WolvicAutofillClient::GetPersonalDataManager() {
  return nullptr;
}

autofill::AutocompleteHistoryManager*
WolvicAutofillClient::GetAutocompleteHistoryManager() {
  return WolvicBrowserContext::FromWebContents(*web_contents())
      ->GetAutocompleteHistoryManager();
}

PrefService* WolvicAutofillClient::GetPrefs() {
  return WolvicBrowserContext::FromWebContents(*web_contents())
      ->GetPrefService();
}

const PrefService* WolvicAutofillClient::GetPrefs() const {
  return WolvicBrowserContext::FromWebContents(*web_contents())
      ->GetPrefService();
}

syncer::SyncService* WolvicAutofillClient::GetSyncService() {
  return nullptr;
}

signin::IdentityManager* WolvicAutofillClient::GetIdentityManager() {
  return nullptr;
}

const signin::IdentityManager* WolvicAutofillClient::GetIdentityManager() const {
  return nullptr;
}

autofill::FormDataImporter* WolvicAutofillClient::GetFormDataImporter() {
  return nullptr;
}

autofill::payments::PaymentsAutofillClient*
WolvicAutofillClient::GetPaymentsAutofillClient() {
  return nullptr;
}

autofill::StrikeDatabase* WolvicAutofillClient::GetStrikeDatabase() {
  return nullptr;
}

ukm::UkmRecorder* WolvicAutofillClient::GetUkmRecorder() {
  return ukm::UkmRecorder::Get();
}

ukm::SourceId WolvicAutofillClient::GetUkmSourceId() {
  return web_contents()->GetPrimaryMainFrame()->GetPageUkmSourceId();
}

autofill::AddressNormalizer* WolvicAutofillClient::GetAddressNormalizer() {
  return nullptr;
}

const GURL& WolvicAutofillClient::GetLastCommittedPrimaryMainFrameURL() const {
  return web_contents()->GetPrimaryMainFrame()->GetLastCommittedURL();
}

url::Origin WolvicAutofillClient::GetLastCommittedPrimaryMainFrameOrigin()
    const {
  return web_contents()->GetPrimaryMainFrame()->GetLastCommittedOrigin();
}

security_state::SecurityLevel
WolvicAutofillClient::GetSecurityLevelForUmaHistograms() {
  return security_state::SecurityLevel::SECURITY_LEVEL_COUNT;
}

const translate::LanguageState* WolvicAutofillClient::GetLanguageState() {
  return nullptr;
}

translate::TranslateDriver* WolvicAutofillClient::GetTranslateDriver() {
  return nullptr;
}

void WolvicAutofillClient::ShowAutofillSettings(
  autofill::SuggestionType suggestion_type) {}

void WolvicAutofillClient::ShowEditAddressProfileDialog(
    const autofill::AutofillProfile& profile,
    AddressProfileSavePromptCallback on_user_decision_callback) {
}

void WolvicAutofillClient::ShowDeleteAddressProfileDialog(
    const autofill::AutofillProfile& profile,
    AddressProfileDeleteDialogCallback delete_dialog_callback) {
}

void WolvicAutofillClient::ConfirmSaveAddressProfile(
    const autofill::AutofillProfile& profile,
    const autofill::AutofillProfile* original_profile,
    bool is_migration_to_account,
    AddressProfileSavePromptCallback callback) {
  // Not implemented
  std::move(callback).Run(
      AddressPromptUserDecision::kIgnored,
      autofill::AutofillProfile(AddressCountryCode("")));
}

void WolvicAutofillClient::OnLoginSelected(JNIEnv* env, jint index) {
  if (!delegate_)
    return;
 
  if (index < 0 || static_cast<size_t>(index) >= suggestions_.size()) {
    delegate_->ClearPreviewedForm();
    return;
  }

  delegate_->DidAcceptSuggestion(
      suggestions_[index],
      autofill::AutofillSuggestionDelegate::SuggestionMetadata{
          index, /*sub_popup_level=*/0});
}

void WolvicAutofillClient::CreatJavaArrayFromSuggestions(JNIEnv* env) {
  Java_AutofillManager_createUsernameArray(env, java_obj_, suggestions_.size());
  for (size_t idx = 0; idx < suggestions_.size(); idx ++) {
    Java_AutofillManager_addUsername(
        env, java_obj_, base::checked_cast<int>(idx),
        base::android::ConvertUTF16ToJavaString(
            env, suggestions_[idx].main_text.value));
  }
}

// static
autofill::AutofillClient::SuggestionUiSessionId
WolvicAutofillClient::GenerateSuggestionUiSessionId() {
  static AutofillClient::SuggestionUiSessionId::Generator generator;
  return generator.GenerateNextId();
}

autofill::AutofillClient::SuggestionUiSessionId WolvicAutofillClient::ShowAutofillSuggestions(
    const autofill::AutofillClient::PopupOpenArgs& open_args,
    base::WeakPtr<autofill::AutofillSuggestionDelegate> delegate) {
  suggestions_ = std::move(open_args.suggestions);
  trigger_source_ = open_args.trigger_source;
  delegate_ = delegate;

  const autofill::AutofillClient::SuggestionUiSessionId session_id = GenerateSuggestionUiSessionId();

  JNIEnv* env = AttachCurrentThread();
  CreatJavaArrayFromSuggestions(env);
  Java_AutofillManager_showAutofillPopup(env, java_obj_);

  delegate_->OnSuggestionsShown(suggestions_);

  return session_id;
}

void WolvicAutofillClient::UpdateAutofillDataListValues(
    base::span<const autofill::SelectOption> datalist) {
}

void WolvicAutofillClient::PinAutofillSuggestions() {}

void WolvicAutofillClient::UpdateAutofillSuggestions(
    const std::vector<autofill::Suggestion>& suggestions,
    autofill::FillingProduct main_filling_product,
    autofill::AutofillSuggestionTriggerSource trigger_source) {
  if (!delegate_)
    return;

  suggestions_ = std::move(suggestions);

  JNIEnv* env = AttachCurrentThread();
  CreatJavaArrayFromSuggestions(env);
  Java_AutofillManager_showAutofillPopup(env, java_obj_);
}

void WolvicAutofillClient::HideAutofillSuggestions(
    autofill::SuggestionHidingReason reason) {
  JNIEnv* env = AttachCurrentThread();
  Java_AutofillManager_dismissPrompt(env, java_obj_);
  if (delegate_) {
    delegate_->ClearPreviewedForm();
    delegate_->OnSuggestionsHidden();
  }
  suggestions_.clear();
  delegate_.reset();
}

bool WolvicAutofillClient::IsAutocompleteEnabled() const {
  ui::WindowAndroid* window_android =
      web_contents()->GetTopLevelNativeWindow();
  if (!window_android)
    return false;

  JNIEnv* env = AttachCurrentThread();
  return Java_AutofillManager_isAutocompleteEnabled(
      env, java_obj_, window_android->GetJavaObject());
}

bool WolvicAutofillClient::IsPasswordManagerEnabled() {
  ui::WindowAndroid* window_android =
      web_contents()->GetTopLevelNativeWindow();
  if (!window_android)
    return false;

  JNIEnv* env = AttachCurrentThread();
  return Java_AutofillManager_isPasswordManagerEnabled(
      env, java_obj_, window_android->GetJavaObject());
}

void WolvicAutofillClient::DidFillOrPreviewForm(
    autofill::mojom::ActionPersistence action_persistence,
    autofill::AutofillTriggerSource trigger_source,
    bool is_refill) {
}

bool WolvicAutofillClient::IsContextSecure() const {
  return false;
}

autofill::FormInteractionsFlowId
WolvicAutofillClient::GetCurrentFormInteractionsFlowId() {
  constexpr base::TimeDelta max_flow_time = base::Minutes(20);
  base::Time now = autofill::AutofillClock::Now();

  if (now - flow_id_date_ > max_flow_time || now < flow_id_date_) {
    flow_id_ = autofill::FormInteractionsFlowId();
    flow_id_date_ = now;
  }
  return flow_id_;
}

std::unique_ptr<autofill::AutofillManager> WolvicAutofillClient::CreateManager(
    base::PassKey<autofill::ContentAutofillDriver> pass_key,
    autofill::ContentAutofillDriver& driver) {
  return std::make_unique<WolvicAutofillManager>(&driver);
}

WolvicAutofillClient::WolvicAutofillClient(content::WebContents* web_contents)
    : autofill::ContentAutofillClient(web_contents),
      web_contents_(web_contents) {
  JNIEnv* env = AttachCurrentThread();
  java_obj_ = Java_AutofillManager_create(env, reinterpret_cast<intptr_t>(this));
}

}  // namespace wolvic
