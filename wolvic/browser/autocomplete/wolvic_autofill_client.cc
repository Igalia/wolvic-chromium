// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/browser/autocomplete/wolvic_autofill_client.h"

#include <utility>

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/check.h"
#include "base/notreached.h"
#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "components/autofill/content/browser/content_autofill_driver.h"
#include "components/autofill/content/browser/content_autofill_driver_factory.h"
#include "components/autofill/content/browser/renderer_forms_from_browser_form.h"
#include "components/autofill/core/browser/autofill_type.h"
#include "components/autofill/core/browser/country_type.h"
#include "components/autofill/core/browser/data_model/payments/autofill_offer_data.h"
#include "components/autofill/core/browser/filling/filling_product.h"
#include "components/autofill/core/browser/form_import/form_data_importer.h"
#include "components/autofill/core/browser/form_structure.h"
#include "components/autofill/core/browser/ui/autofill_suggestion_delegate.h"
#include "components/autofill/core/common/autofill_clock.h"
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

autofill::AutofillCrowdsourcingManager&
WolvicAutofillClient::GetCrowdsourcingManager() {
  if (!crowdsourcing_manager_) {
    // Lazy initialization to avoid virtual function calls in the constructor.
    crowdsourcing_manager_ =
        std::make_unique<autofill::AutofillCrowdsourcingManager>(
            this, GetChannel());
  }
  return *crowdsourcing_manager_;
}

bool WolvicAutofillClient::HasPersonalDataManager() const {
  return false;
}

autofill::PersonalDataManager& WolvicAutofillClient::GetPersonalDataManager() {
  NOTREACHED();
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

const signin::IdentityManager* WolvicAutofillClient::GetIdentityManager()
    const {
  return nullptr;
}

metrics::ProfileMetricsService*
WolvicAutofillClient::GetProfileMetricsService() {
  return WolvicBrowserContext::FromWebContents(*web_contents())
      ->GetProfileMetricsService();
}

autofill::PasswordManagerDelegate*
WolvicAutofillClient::GetPasswordManagerDelegate(
    const autofill::FieldGlobalId& field_id) {
  content::RenderFrameHost* rfh = autofill::FindRenderFrameHostByToken(
      *web_contents(), field_id.frame_token);
  if (!rfh)
    return nullptr;
  password_manager::ContentPasswordManagerDriver* driver =
      password_manager::ContentPasswordManagerDriver::GetForRenderFrameHost(
          rfh);
  return driver ? driver->GetPasswordAutofillManager() : nullptr;
}

autofill::FormDataImporter* WolvicAutofillClient::GetFormDataImporter() {
  return nullptr;
}

autofill::payments::PaymentsAutofillClient*
WolvicAutofillClient::GetPaymentsAutofillClient() {
  return nullptr;
}

strike_database::StrikeDatabase* WolvicAutofillClient::GetStrikeDatabase() {
  return nullptr;
}

ukm::UkmRecorder* WolvicAutofillClient::GetUkmRecorder() {
  return ukm::UkmRecorder::Get();
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

void WolvicAutofillClient::ConfirmSaveAddressProfile(
    const autofill::AutofillProfile& profile,
    const autofill::AutofillProfile* original_profile,
    SaveAddressBubbleType save_address_bubble_type,
    AddressProfileSavePromptCallback callback) {
  // Not implemented
  std::move(callback).Run(
      AddressPromptUserDecision::kIgnored,
      autofill::AutofillProfile(autofill::AddressCountryCode("")));
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
          .row = index, .sub_popup_level = 0});
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

autofill::AutofillClient::SuggestionUiSessionId
WolvicAutofillClient::ShowAutofillSuggestions(
    const autofill::AutofillClient::PopupOpenArgs& open_args,
    base::WeakPtr<autofill::AutofillSuggestionDelegate> delegate) {
  suggestions_ = std::move(open_args.suggestions);
  trigger_source_ = open_args.trigger_source;
  delegate_ = delegate;

  JNIEnv* env = AttachCurrentThread();
  CreatJavaArrayFromSuggestions(env);
  Java_AutofillManager_showAutofillPopup(env, java_obj_);

  delegate_->OnSuggestionsShown(suggestions_);

  return autofill::AutofillClient::SuggestionUiSessionId();
}

void WolvicAutofillClient::UpdateAutofillDataListValues(
    base::span<const autofill::SelectOption> datalist) {
}

base::span<const autofill::Suggestion>
WolvicAutofillClient::GetAutofillSuggestions() const {
  return suggestions_;
}

void WolvicAutofillClient::UpdateAutofillSuggestions(
    const std::vector<autofill::Suggestion>& suggestions,
    autofill::FillingProduct main_filling_product,
    autofill::AutofillSuggestionTriggerSource trigger_source,
    autofill::AutofillSuggestionsIgnoreFocusLoss ignore_focus_loss) {
  if (!delegate_)
    return;

  suggestions_ = std::move(suggestions);

  JNIEnv* env = AttachCurrentThread();
  CreatJavaArrayFromSuggestions(env);
  Java_AutofillManager_showAutofillPopup(env, java_obj_);
}

void WolvicAutofillClient::HideSuggestions(
    autofill::SuggestionHidingReason reason,
    std::optional<autofill::FillingProduct> product) {
  // Wolvic's credential picker is an asynchronous, modal VR surface rather than
  // an inline popup. Opening it blurs the underlying web input, so the autofill
  // core emits focus/renderer-driven hide requests. Honoring those would tear
  // down `delegate_`/`suggestions_` before the user taps a row, silently
  // dropping the fill. Only dismiss for reasons that reflect an explicit
  // accept/abort or genuine teardown; ignore the spurious focus churn.
  switch (reason) {
    case autofill::SuggestionHidingReason::kAcceptSuggestion:
    case autofill::SuggestionHidingReason::kUserAborted:
    case autofill::SuggestionHidingReason::kNavigation:
    case autofill::SuggestionHidingReason::kTabGone:
    case autofill::SuggestionHidingReason::kViewDestroyed:
      break;
    default:
      return;
  }
  JNIEnv* env = AttachCurrentThread();
  Java_AutofillManager_dismissPrompt(env, java_obj_);
  if (delegate_) {
    delegate_->ClearPreviewedForm();
    delegate_->OnSuggestionsHidden(reason);
  }
  suggestions_.clear();
  delegate_.reset();
}

const std::string& WolvicAutofillClient::GetAppLocale() const {
  static const std::string kEmpty;
  return kEmpty;
}

autofill::VotesUploader& WolvicAutofillClient::GetVotesUploader() {
  NOTREACHED();
}

autofill::EntityDataManager* WolvicAutofillClient::GetEntityDataManager() {
  return nullptr;
}

autofill::ValuablesDataManager*
WolvicAutofillClient::GetValuablesDataManager() {
  return nullptr;
}

credential_management::ContentCredentialManager*
WolvicAutofillClient::GetContentCredentialManager() {
  return nullptr;
}

autofill::SingleFieldFillRouter& WolvicAutofillClient::GetSingleFieldFillRouter() {
  NOTREACHED();
}

bool WolvicAutofillClient::IsAutofillEnabled() const {
  return false;
}

bool WolvicAutofillClient::IsAutofillProfileEnabled() const {
  return false;
}

autofill::autofill_metrics::FormInteractionsUkmLogger&
WolvicAutofillClient::GetFormInteractionsUkmLogger() {
  NOTREACHED();
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

bool WolvicAutofillClient::IsPasswordManagerEnabled() const {
  ui::WindowAndroid* window_android =
      web_contents()->GetTopLevelNativeWindow();
  if (!window_android)
    return false;

  JNIEnv* env = AttachCurrentThread();
  return Java_AutofillManager_isPasswordManagerEnabled(
      env, java_obj_, window_android->GetJavaObject());
}

bool WolvicAutofillClient::IsContextSecure() const {
  return false;
}

bool WolvicAutofillClient::IsWalletPublicPassStorageEnabled() const {
  return false;
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
