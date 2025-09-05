// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Defines the Chrome Extensions BrowsingData API functions, which entail
// clearing browsing data, and clearing the browser's cache (which, let's be
// honest, are the same thing), as specified in the extension API JSON.

#include "components/extensions/browser/api/browsing_data/browsing_data_api.h"

#include <string>
#include <utility>

#include "base/functional/bind.h"
#include "base/strings/stringprintf.h"
#include "base/task/thread_pool.h"
#include "components/browsing_data/content/browsing_data_helper.h"
#include "components/browsing_data/core/pref_names.h"
#include "components/history/core/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "components/signin/public/identity_manager/identity_manager.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/browsing_data_filter_builder.h"
#include "content/public/browser/browsing_data_remover.h"
#include "extensions/common/error_utils.h"
#include "extensions/common/extension.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"

using browsing_data::BrowsingDataType;
using browsing_data::ClearBrowsingDataTab;
using content::BrowserThread;

namespace {

// TODO(mshin): Support chrome_browsing_data_remover
// const int64_t kFilterableDataTypes =
//     chrome_browsing_data_remover::DATA_TYPE_SITE_DATA |
//     content::BrowsingDataRemover::DATA_TYPE_CACHE;

// static_assert((kFilterableDataTypes &
//                ~chrome_browsing_data_remover::FILTERABLE_DATA_TYPES) == 0,
//               "kFilterableDataTypes must be a subset of "
//               "chrome_browsing_data_remover::FILTERABLE_DATA_TYPES");

uint64_t MaskForKey(const char* key) {
  // TODO(mshin): Support chrome_browsing_data_remover

  if (strcmp(key, extension_browsing_data_api_constants::kCacheKey) == 0)
    return content::BrowsingDataRemover::DATA_TYPE_CACHE;
  if (strcmp(key, extension_browsing_data_api_constants::kCookiesKey) == 0) {
    return content::BrowsingDataRemover::DATA_TYPE_COOKIES;
  }
  if (strcmp(key, extension_browsing_data_api_constants::kDownloadsKey) == 0)
    return content::BrowsingDataRemover::DATA_TYPE_DOWNLOADS;
  if (strcmp(key, extension_browsing_data_api_constants::kFileSystemsKey) == 0)
    return content::BrowsingDataRemover::DATA_TYPE_FILE_SYSTEMS;
  // if (strcmp(key, extension_browsing_data_api_constants::kFormDataKey) == 0)
  //   return chrome_browsing_data_remover::DATA_TYPE_FORM_DATA;
  // if (strcmp(key, extension_browsing_data_api_constants::kHistoryKey) == 0)
  //   return chrome_browsing_data_remover::DATA_TYPE_HISTORY;
  if (strcmp(key, extension_browsing_data_api_constants::kIndexedDBKey) == 0)
    return content::BrowsingDataRemover::DATA_TYPE_INDEXED_DB;
  if (strcmp(key, extension_browsing_data_api_constants::kLocalStorageKey) == 0)
    return content::BrowsingDataRemover::DATA_TYPE_LOCAL_STORAGE;
  // if (strcmp(key, extension_browsing_data_api_constants::kPasswordsKey) == 0)
  //   return chrome_browsing_data_remover::DATA_TYPE_PASSWORDS;
  if (strcmp(key, extension_browsing_data_api_constants::kServiceWorkersKey) ==
      0)
    return content::BrowsingDataRemover::DATA_TYPE_SERVICE_WORKERS;
  if (strcmp(key, extension_browsing_data_api_constants::kCacheStorageKey) == 0)
    return content::BrowsingDataRemover::DATA_TYPE_CACHE_STORAGE;
  if (strcmp(key, extension_browsing_data_api_constants::kWebSQLKey) == 0)
    return content::BrowsingDataRemover::DATA_TYPE_WEB_SQL;

  return 0ULL;
}

// Returns false if any of the selected data types are not allowed to be
// deleted.
bool IsRemovalPermitted(uint64_t removal_mask, PrefService* prefs) {
  // Enterprise policy or user preference might prohibit deleting browser or
  // download history.
  if (/*(removal_mask & chrome_browsing_data_remover::DATA_TYPE_HISTORY) ||*/
      (removal_mask & content::BrowsingDataRemover::DATA_TYPE_DOWNLOADS)) {
    return prefs->GetBoolean(prefs::kAllowDeletingBrowserHistory);
  }
  return true;
}

}  // namespace

bool BrowsingDataSettingsFunction::isDataTypeSelected(
    BrowsingDataType data_type,
    ClearBrowsingDataTab tab) {
  std::string pref_name;
  bool success = GetDeletionPreferenceFromDataType(data_type, tab, &pref_name);
  return success && prefs_->GetBoolean(pref_name);
}

ExtensionFunction::ResponseAction BrowsingDataSettingsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void BrowsingDataSettingsFunction::SetDetails(base::Value::Dict* selected_dict,
                                              base::Value::Dict* permitted_dict,
                                              const char* data_type,
                                              bool is_selected) {
  bool is_permitted = IsRemovalPermitted(MaskForKey(data_type), prefs_);
  selected_dict->Set(data_type, is_selected && is_permitted);
  permitted_dict->Set(data_type, is_permitted);
}

BrowsingDataRemoverFunction::BrowsingDataRemoverFunction() = default;

void BrowsingDataRemoverFunction::OnBrowsingDataRemoverDone(
    uint64_t failed_data_types) {
  OnTaskFinished();
}

void BrowsingDataRemoverFunction::OnTaskFinished() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK_GT(pending_tasks_, 0);
  if (--pending_tasks_ > 0)
    return;
  // TODO(mshin): Support sync
  // synced_data_deletion_.reset();
  observation_.Reset();
  Respond(NoArguments());
  Release();  // Balanced in StartRemoving.
}

ExtensionFunction::ResponseAction BrowsingDataRemoverFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

BrowsingDataRemoverFunction::~BrowsingDataRemoverFunction() = default;

bool BrowsingDataRemoverFunction::IsPauseSyncAllowed() {
  return true;
}

void BrowsingDataRemoverFunction::StartRemoving() {
  content::BrowsingDataRemover* remover = browser_context()->GetBrowsingDataRemover();

  // Add a ref (Balanced in OnTaskFinished)
  AddRef();

  // Prevent Sync from being paused, if required.
  // TODO(mshin): Support sync
  // DCHECK(!synced_data_deletion_);
  // if (!IsPauseSyncAllowed() && IsSyncRunning(browser_context())) {
  //   synced_data_deletion_ =
  //       AccountReconcilorFactory::GetForProfile(browser_context())
  //           ->GetScopedSyncDataDeletion();
  // }

  // Create a BrowsingDataRemover, set the current object as an observer (so
  // that we're notified after removal) and call remove() with the arguments
  // we've generated above. We can use a raw pointer here, as the browsing data
  // remover is responsible for deleting itself once data removal is complete.
  observation_.Observe(remover);

  DCHECK_EQ(pending_tasks_, 0);
  pending_tasks_ = 1;
  if (removal_mask_ & content::BrowsingDataRemover::DATA_TYPE_COOKIES &&
      !origins_.empty()) {
    pending_tasks_ += 1;
    removal_mask_ &= ~content::BrowsingDataRemover::DATA_TYPE_COOKIES;
    // Cookies are scoped more broadly than origins, so we expand the
    // origin filter to registrable domains in order to match all cookies
    // that could be applied to an origin. This is the same behavior as
    // the Clear-Site-Data header.
    auto filter_builder = content::BrowsingDataFilterBuilder::Create(mode_);
    for (const auto& origin : origins_) {
      std::string domain = GetDomainAndRegistry(
          origin, net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
      if (domain.empty())
        domain = origin.host();  // IP address or internal hostname.
      filter_builder->AddRegisterableDomain(domain);
    }
    remover->RemoveWithFilterAndReply(
        remove_since_, base::Time::Max(),
        content::BrowsingDataRemover::DATA_TYPE_COOKIES, origin_type_mask_,
        std::move(filter_builder), this);
  }
  if (removal_mask_) {
    pending_tasks_ += 1;
    auto filter_builder = content::BrowsingDataFilterBuilder::Create(mode_);
    for (const auto& origin : origins_) {
      filter_builder->AddOrigin(origin);
    }
    remover->RemoveWithFilterAndReply(remove_since_, base::Time::Max(),
                                      removal_mask_, origin_type_mask_,
                                      std::move(filter_builder), this);
  }
  OnTaskFinished();
}

bool BrowsingDataRemoverFunction::ParseOriginTypeMask(
    const base::Value::Dict& options,
    uint64_t* origin_type_mask) {
  // Parse the |options| dictionary to generate the origin set mask. Default to
  // UNPROTECTED_WEB if the developer doesn't specify anything.
  *origin_type_mask = content::BrowsingDataRemover::ORIGIN_TYPE_UNPROTECTED_WEB;

  const base::Value* origin_type_dict =
      options.Find(extension_browsing_data_api_constants::kOriginTypesKey);
  if (!origin_type_dict)
    return true;

  if (!origin_type_dict->is_dict())
    return false;

  const base::Value::Dict& origin_type = origin_type_dict->GetDict();

  // The developer specified something! Reset to 0 and parse the dictionary.
  *origin_type_mask = 0;

  // Unprotected web.
  const base::Value* option = origin_type.Find(
      extension_browsing_data_api_constants::kUnprotectedWebKey);
  if (option) {
    if (!option->is_bool())
      return false;

    *origin_type_mask |=
        option->GetBool()
            ? content::BrowsingDataRemover::ORIGIN_TYPE_UNPROTECTED_WEB
            : 0;
  }

  // Protected web.
  option =
      origin_type.Find(extension_browsing_data_api_constants::kProtectedWebKey);
  if (option) {
    if (!option->is_bool())
      return false;

    *origin_type_mask |=
        option->GetBool()
            ? content::BrowsingDataRemover::ORIGIN_TYPE_PROTECTED_WEB
            : 0;
  }

  // Extensions.
  option =
      origin_type.Find(extension_browsing_data_api_constants::kExtensionsKey);
  if (option) {
    if (!option->is_bool())
      return false;

    // TODO(mshin): Support chrome_browsing_data_remover
    // *origin_type_mask |=
    //     option->GetBool() ? chrome_browsing_data_remover::ORIGIN_TYPE_EXTENSION
    //                       : 0;
  }

  return true;
}

BrowsingDataRemoverFunction::OriginParsingResult
BrowsingDataRemoverFunction::ParseOrigins(const base::Value::List& list_value) {
  std::vector<url::Origin> result;
  result.reserve(list_value.size());
  for (const auto& value : list_value) {
    if (!value.is_string()) {
      return base::unexpected(BadMessage());
    }
    url::Origin origin = url::Origin::Create(GURL(value.GetString()));
    if (origin.opaque()) {
      return base::unexpected(Error(base::StringPrintf(
          extension_browsing_data_api_constants::kInvalidOriginError,
          value.GetString().c_str())));
    }
    result.push_back(std::move(origin));
  }
  return result;
}

// Parses the |dataToRemove| argument to generate the removal mask.
// Returns false if parse was not successful, i.e. if 'dataToRemove' is not
// present or any data-type keys don't have supported (boolean) values.
bool BrowsingDataRemoveFunction::GetRemovalMask(uint64_t* removal_mask) {
  if (args().size() <= 1 || !args()[1].is_dict())
    return false;

  *removal_mask = 0;
  for (const auto kv : args()[1].GetDict()) {
    if (!kv.second.is_bool())
      return false;
    if (kv.second.GetBool())
      *removal_mask |= MaskForKey(kv.first.c_str());
  }

  return true;
}

bool BrowsingDataRemoveFunction::IsPauseSyncAllowed() {
  return false;
}

bool BrowsingDataRemoveAppcacheFunction::GetRemovalMask(
    uint64_t* removal_mask) {
  // TODO(http://crbug.com/1266606): deprecate and remove this extension api
  *removal_mask = 0;
  return true;
}

bool BrowsingDataRemoveCacheFunction::GetRemovalMask(uint64_t* removal_mask) {
  *removal_mask = content::BrowsingDataRemover::DATA_TYPE_CACHE;
  return true;
}

bool BrowsingDataRemoveCookiesFunction::GetRemovalMask(uint64_t* removal_mask) {
  *removal_mask = content::BrowsingDataRemover::DATA_TYPE_COOKIES;
  return true;
}

bool BrowsingDataRemoveDownloadsFunction::GetRemovalMask(
    uint64_t* removal_mask) {
  *removal_mask = content::BrowsingDataRemover::DATA_TYPE_DOWNLOADS;
  return true;
}

bool BrowsingDataRemoveFileSystemsFunction::GetRemovalMask(
    uint64_t* removal_mask) {
  *removal_mask = content::BrowsingDataRemover::DATA_TYPE_FILE_SYSTEMS;
  return true;
}

bool BrowsingDataRemoveFormDataFunction::GetRemovalMask(
    uint64_t* removal_mask) {
  // TODO(mshin): Support chrome_browsing_data_remover
  *removal_mask = 0;//chrome_browsing_data_remover::DATA_TYPE_FORM_DATA;
  return true;
}

bool BrowsingDataRemoveHistoryFunction::GetRemovalMask(uint64_t* removal_mask) {
  // TODO(mshin): Support chrome_browsing_data_remover
  *removal_mask = 0;//chrome_browsing_data_remover::DATA_TYPE_HISTORY;
  return true;
}

bool BrowsingDataRemoveIndexedDBFunction::GetRemovalMask(
    uint64_t* removal_mask) {
  *removal_mask = content::BrowsingDataRemover::DATA_TYPE_INDEXED_DB;
  return true;
}

bool BrowsingDataRemoveLocalStorageFunction::GetRemovalMask(
    uint64_t* removal_mask) {
  *removal_mask = content::BrowsingDataRemover::DATA_TYPE_LOCAL_STORAGE;
  return true;
}

bool BrowsingDataRemovePluginDataFunction::GetRemovalMask(
    uint64_t* removal_mask) {
  // Plugin data is not supported anymore. (crbug.com/1135788)
  *removal_mask = 0;
  return true;
}

bool BrowsingDataRemovePasswordsFunction::GetRemovalMask(
    uint64_t* removal_mask) {
  *removal_mask = 0; // chrome_browsing_data_remover::DATA_TYPE_PASSWORDS;
  return true;
}

bool BrowsingDataRemoveServiceWorkersFunction::GetRemovalMask(
    uint64_t* removal_mask) {
  *removal_mask = content::BrowsingDataRemover::DATA_TYPE_SERVICE_WORKERS;
  return true;
}

bool BrowsingDataRemoveCacheStorageFunction::GetRemovalMask(
    uint64_t* removal_mask) {
  *removal_mask = content::BrowsingDataRemover::DATA_TYPE_CACHE_STORAGE;
  return true;
}

bool BrowsingDataRemoveWebSQLFunction::GetRemovalMask(uint64_t* removal_mask) {
  *removal_mask = content::BrowsingDataRemover::DATA_TYPE_WEB_SQL;
  return true;
}
