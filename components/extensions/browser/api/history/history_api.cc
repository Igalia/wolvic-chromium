// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/history/history_api.h"

#include <memory>
#include <set>
#include <utility>

#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/functional/callback_helpers.h"
#include "base/json/json_writer.h"
#include "base/lazy_instance.h"
#include "base/location.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/cancelable_task_tracker.h"
#include "base/task/single_thread_task_runner.h"
#include "base/time/time.h"
#include "base/values.h"
#include "components/history/core/browser/history_service.h"
#include "components/history/core/browser/history_types.h"
#include "components/history/core/common/pref_names.h"
#include "components/prefs/pref_service.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_system_provider.h"
#include "extensions/browser/extensions_browser_client.h"

namespace extensions {

using api::history::HistoryItem;
using api::history::VisitItem;

typedef std::vector<api::history::HistoryItem> HistoryItemList;
typedef std::vector<api::history::VisitItem> VisitItemList;

namespace AddUrl = api::history::AddUrl;
namespace DeleteUrl = api::history::DeleteUrl;
namespace DeleteRange = api::history::DeleteRange;
namespace GetVisits = api::history::GetVisits;
namespace OnVisited = api::history::OnVisited;
namespace OnVisitRemoved = api::history::OnVisitRemoved;
namespace Search = api::history::Search;

namespace {

const char kInvalidUrlError[] = "Url is invalid.";

HistoryItem GetHistoryItem(const history::URLRow& row) {
  HistoryItem history_item;

  history_item.id = base::NumberToString(row.id());
  history_item.url = row.url().spec();
  history_item.title = base::UTF16ToUTF8(row.title());
  history_item.last_visit_time =
      row.last_visit().InMillisecondsFSinceUnixEpoch();
  history_item.typed_count = row.typed_count();
  history_item.visit_count = row.visit_count();

  return history_item;
}

VisitItem GetVisitItem(const history::VisitRow& row) {
  VisitItem visit_item;

  visit_item.id = base::NumberToString(row.url_id);
  visit_item.visit_id = base::NumberToString(row.visit_id);
  visit_item.visit_time = row.visit_time.InMillisecondsFSinceUnixEpoch();
  visit_item.referring_visit_id = base::NumberToString(row.referring_visit);

  api::history::TransitionType transition = api::history::TransitionType::kLink;
  switch (row.transition & ui::PAGE_TRANSITION_CORE_MASK) {
    case ui::PAGE_TRANSITION_LINK:
      transition = api::history::TransitionType::kLink;
      break;
    case ui::PAGE_TRANSITION_TYPED:
      transition = api::history::TransitionType::kTyped;
      break;
    case ui::PAGE_TRANSITION_AUTO_BOOKMARK:
      transition = api::history::TransitionType::kAutoBookmark;
      break;
    case ui::PAGE_TRANSITION_AUTO_SUBFRAME:
      transition = api::history::TransitionType::kAutoSubframe;
      break;
    case ui::PAGE_TRANSITION_MANUAL_SUBFRAME:
      transition = api::history::TransitionType::kManualSubframe;
      break;
    case ui::PAGE_TRANSITION_GENERATED:
      transition = api::history::TransitionType::kGenerated;
      break;
    case ui::PAGE_TRANSITION_AUTO_TOPLEVEL:
      transition = api::history::TransitionType::kAutoToplevel;
      break;
    case ui::PAGE_TRANSITION_FORM_SUBMIT:
      transition = api::history::TransitionType::kFormSubmit;
      break;
    case ui::PAGE_TRANSITION_RELOAD:
      transition = api::history::TransitionType::kReload;
      break;
    case ui::PAGE_TRANSITION_KEYWORD:
      transition = api::history::TransitionType::kKeyword;
      break;
    case ui::PAGE_TRANSITION_KEYWORD_GENERATED:
      transition = api::history::TransitionType::kKeywordGenerated;
      break;
    default:
      DCHECK(false);
  }

  visit_item.transition = transition;

  visit_item.is_local = row.originator_cache_guid.empty();

  return visit_item;
}

}  // namespace

HistoryEventRouter::HistoryEventRouter(content::BrowserContext* browser_context,
                                       history::HistoryService* history_service)
    : browser_context_(browser_context) {
  DCHECK(browser_context_);
  history_service_observation_.Observe(history_service);
}

HistoryEventRouter::~HistoryEventRouter() {
}

void HistoryEventRouter::OnURLVisited(history::HistoryService* history_service,
                                      const history::URLRow& url_row,
                                      const history::VisitRow& new_visit) {
  auto args = OnVisited::Create(GetHistoryItem(url_row));
  DispatchEvent(browser_context_, events::HISTORY_ON_VISITED,
                api::history::OnVisited::kEventName, std::move(args));
}

void HistoryEventRouter::OnURLsDeleted(
    history::HistoryService* history_service,
    const history::DeletionInfo& deletion_info) {
  OnVisitRemoved::Removed removed;
  removed.all_history = deletion_info.IsAllHistory();

  removed.urls.emplace();
  for (const auto& row : deletion_info.deleted_rows())
    removed.urls->push_back(row.url().spec());

  auto args = OnVisitRemoved::Create(removed);
  DispatchEvent(browser_context_, events::HISTORY_ON_VISIT_REMOVED,
                api::history::OnVisitRemoved::kEventName, std::move(args));
}

void HistoryEventRouter::DispatchEvent(content::BrowserContext* browser_context,
                                       events::HistogramValue histogram_value,
                                       const std::string& event_name,
                                       base::Value::List event_args) {
  if (browser_context && EventRouter::Get(browser_context)) {
    auto event = std::make_unique<Event>(histogram_value, event_name,
                                         std::move(event_args), browser_context);
    EventRouter::Get(browser_context)->BroadcastEvent(std::move(event));
  }
}

HistoryAPI::HistoryAPI(content::BrowserContext* context)
    : browser_context_(context) {
  EventRouter* event_router = EventRouter::Get(browser_context_);
  event_router->RegisterObserver(this, api::history::OnVisited::kEventName);
  event_router->RegisterObserver(this,
                                 api::history::OnVisitRemoved::kEventName);
}

HistoryAPI::~HistoryAPI() {
}

void HistoryAPI::Shutdown() {
  history_event_router_.reset();
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

static base::LazyInstance<BrowserContextKeyedAPIFactory<HistoryAPI>>::
    DestructorAtExit g_history_api_factory = LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<HistoryAPI>* HistoryAPI::GetFactoryInstance() {
  return g_history_api_factory.Pointer();
}

template <>
void BrowserContextKeyedAPIFactory<HistoryAPI>::DeclareFactoryDependencies() {
  // DependsOn(ActivityLog::GetFactoryInstance());
  // DependsOn(HistoryServiceFactory::GetInstance());
  DependsOn(ExtensionsBrowserClient::Get()->GetExtensionSystemFactory());
}

void HistoryAPI::OnListenerAdded(const EventListenerInfo& details) {
  // history_event_router_ = std::make_unique<HistoryEventRouter>(
  //     browser_context_, HistoryServiceFactory::GetForProfile(
  //                  browser_context_, ServiceAccessType::EXPLICIT_ACCESS));
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

bool HistoryFunction::ValidateUrl(const std::string& url_string,
                                  GURL* url,
                                  std::string* error) {
  GURL temp_url(url_string);
  if (!temp_url.is_valid()) {
    *error = kInvalidUrlError;
    return false;
  }
  url->Swap(&temp_url);
  return true;
}

bool HistoryFunction::VerifyDeleteAllowed(std::string* error) {
  // TODO(mshin): Enable the below code after supporting Preference
  // PrefService* prefs = GetProfile()->GetPrefs();
  // if (!prefs->GetBoolean(prefs::kAllowDeletingBrowserHistory)) {
  //   *error = kDeleteProhibitedError;
  //   return false;
  // }
  return true;
}

base::Time HistoryFunction::GetTime(double ms_from_epoch) {
  return base::Time::FromMillisecondsSinceUnixEpoch(ms_from_epoch);
}

HistoryFunctionWithCallback::HistoryFunctionWithCallback() {}

HistoryFunctionWithCallback::~HistoryFunctionWithCallback() {}

ExtensionFunction::ResponseAction HistoryGetVisitsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void HistoryGetVisitsFunction::QueryComplete(history::QueryURLResult result) {
  VisitItemList visit_item_vec;
  if (result.success && !result.visits.empty()) {
    for (const history::VisitRow& visit : result.visits)
      visit_item_vec.push_back(GetVisitItem(visit));
  }

  Respond(ArgumentList(GetVisits::Results::Create(visit_item_vec)));
  Release();  // Balanced in Run().
}

ExtensionFunction::ResponseAction HistorySearchFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void HistorySearchFunction::SearchComplete(history::QueryResults results) {
  HistoryItemList history_item_vec;
  if (!results.empty()) {
    for (const auto& item : results)
      history_item_vec.push_back(GetHistoryItem(item));
  }
  Respond(ArgumentList(Search::Results::Create(history_item_vec)));
  Release();  // Balanced in Run().
}

ExtensionFunction::ResponseAction HistoryAddUrlFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction HistoryDeleteUrlFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction HistoryDeleteRangeFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void HistoryDeleteRangeFunction::DeleteComplete() {
  Respond(NoArguments());
  Release();  // Balanced in Run().
}

ExtensionFunction::ResponseAction HistoryDeleteAllFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void HistoryDeleteAllFunction::DeleteComplete() {
  Respond(NoArguments());
  Release();  // Balanced in Run().
}

}  // namespace extensions
