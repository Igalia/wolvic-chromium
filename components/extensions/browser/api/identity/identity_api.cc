// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/identity/identity_api.h"

#include <stddef.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/containers/contains.h"
#include "base/lazy_instance.h"
#include "base/strings/string_number_conversions.h"
#include "base/trace_event/trace_event.h"
#include "base/values.h"
#include "components/extensions/common/api/identity.h"
#include "extensions/browser/extension_function_dispatcher.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_l10n_util.h"
#include "extensions/common/manifest_handlers/oauth2_manifest_handler.h"
#include "extensions/common/permissions/permission_set.h"
#include "extensions/common/permissions/permissions_data.h"
#include "google_apis/gaia/gaia_urls.h"
#include "url/gurl.h"

namespace extensions {

namespace {
const char kIdentityGaiaIdPref[] = "identity_gaia_id";
}

IdentityAPI::IdentityAPI(content::BrowserContext* context)
    : IdentityAPI(context,
                  // TODO(mshin): Support IdentityManagerFactory
                  // IdentityManagerFactory::GetForProfile(
                  //     Profile::FromBrowserContext(context)),
                  nullptr,
                  ExtensionPrefs::Get(context),
                  EventRouter::Get(context)) {}

IdentityAPI::~IdentityAPI() {}

IdentityMintRequestQueue* IdentityAPI::mint_queue() { return nullptr; }

IdentityTokenCache* IdentityAPI::token_cache() {
  return nullptr;
}

void IdentityAPI::SetGaiaIdForExtension(const std::string& extension_id,
                                        const std::string& gaia_id) {
  DCHECK(!gaia_id.empty());
  extension_prefs_->UpdateExtensionPref(extension_id, kIdentityGaiaIdPref,
                                        base::Value(gaia_id));
}

std::optional<std::string> IdentityAPI::GetGaiaIdForExtension(
    const std::string& extension_id) {
  std::string gaia_id;
  if (!extension_prefs_->ReadPrefAsString(extension_id, kIdentityGaiaIdPref,
                                          &gaia_id)) {
    return std::nullopt;
  }
  return gaia_id;
}

void IdentityAPI::EraseGaiaIdForExtension(const std::string& extension_id) {
  extension_prefs_->UpdateExtensionPref(extension_id, kIdentityGaiaIdPref,
                                        std::nullopt);
}

void IdentityAPI::EraseStaleGaiaIdsForAllExtensions() {
  // Refresh tokens haven't been loaded yet. Wait for OnRefreshTokensLoaded() to
  // fire.
  if (!identity_manager_->AreRefreshTokensLoaded())
    return;
  std::vector<CoreAccountInfo> accounts =
      identity_manager_->GetAccountsWithRefreshTokens();
  for (const ExtensionId& extension_id : extension_prefs_->GetExtensions()) {
    std::optional<std::string> gaia_id = GetGaiaIdForExtension(extension_id);
    if (!gaia_id)
      continue;
    if (!base::Contains(accounts, *gaia_id, &CoreAccountInfo::gaia)) {
      EraseGaiaIdForExtension(extension_id);
    }
  }
}

void IdentityAPI::Shutdown() {
  on_shutdown_callback_list_.Notify();
  identity_manager_->RemoveObserver(this);
}

static base::LazyInstance<BrowserContextKeyedAPIFactory<IdentityAPI>>::
    DestructorAtExit g_identity_api_factory = LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<IdentityAPI>* IdentityAPI::GetFactoryInstance() {
  return g_identity_api_factory.Pointer();
}

base::CallbackListSubscription IdentityAPI::RegisterOnShutdownCallback(
    base::OnceClosure cb) {
  return on_shutdown_callback_list_.Add(std::move(cb));
}

bool IdentityAPI::AreExtensionsRestrictedToPrimaryAccount() {
  // return !AccountConsistencyModeManager::IsDiceEnabledForProfile(profile_) &&
  //        !AccountConsistencyModeManager::IsMirrorEnabledForProfile(profile_);
  return false;
}

IdentityAPI::IdentityAPI(content::BrowserContext* context,
                         signin::IdentityManager* identity_manager,
                         ExtensionPrefs* extension_prefs,
                         EventRouter* event_router)
    : browser_context_(context),
      identity_manager_(identity_manager),
      extension_prefs_(extension_prefs),
      event_router_(event_router) {
  identity_manager_->AddObserver(this);
  EraseStaleGaiaIdsForAllExtensions();
}

void IdentityAPI::OnRefreshTokensLoaded() {
  EraseStaleGaiaIdsForAllExtensions();
}

void IdentityAPI::OnRefreshTokenUpdatedForAccount(
    const CoreAccountInfo& account_info) {
  // Refresh tokens are sometimes made available in contexts where
  // AccountTrackerService is not tracking the account in question. Bail out in
  // these cases.
  if (account_info.gaia.empty())
    return;

  FireOnAccountSignInChanged(account_info.gaia, true);
}

void IdentityAPI::OnExtendedAccountInfoRemoved(
    const AccountInfo& account_info) {
  DCHECK(!account_info.gaia.empty());
  EraseStaleGaiaIdsForAllExtensions();
  FireOnAccountSignInChanged(account_info.gaia, false);
}

void IdentityAPI::FireOnAccountSignInChanged(const std::string& gaia_id,
                                             bool is_signed_in) {
  DCHECK(!gaia_id.empty());
  api::identity::AccountInfo api_account_info;
  api_account_info.id = gaia_id;

  auto args =
      api::identity::OnSignInChanged::Create(api_account_info, is_signed_in);
  std::unique_ptr<Event> event(new Event(
      events::IDENTITY_ON_SIGN_IN_CHANGED,
      api::identity::OnSignInChanged::kEventName, std::move(args), browser_context_));

  if (on_signin_changed_callback_for_testing_)
    on_signin_changed_callback_for_testing_.Run(event.get());

  event_router_->BroadcastEvent(std::move(event));
}

template <>
void BrowserContextKeyedAPIFactory<IdentityAPI>::DeclareFactoryDependencies() {
  DependsOn(ExtensionsBrowserClient::Get()->GetExtensionSystemFactory());

  // TODO(mshin): Enable the below code after support Signin and IdentityManagerFactory
  // DependsOn(ChromeSigninClientFactory::GetInstance());
  // DependsOn(IdentityManagerFactory::GetInstance());
}

}  // namespace extensions
