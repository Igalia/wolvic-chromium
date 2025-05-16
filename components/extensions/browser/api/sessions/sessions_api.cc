// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/sessions/sessions_api.h"

#include <stddef.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "base/i18n/rtl.h"
#include "base/lazy_instance.h"
#include "base/memory/raw_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "components/extensions/browser/api/sessions/session_id.h"
#include "components/sessions/content/content_live_tab.h"
#include "components/sync_sessions/open_tabs_ui_delegate.h"
#include "components/sync_sessions/session_sync_service.h"
#include "components/sync_sessions/synced_session.h"
#include "components/url_formatter/url_formatter.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/extension_function_dispatcher.h"
#include "extensions/browser/extension_function_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/error_utils.h"
#include "extensions/common/mojom/context_type.mojom.h"

namespace extensions {

namespace windows = api::windows;

namespace {

namespace GetRecentlyClosed = api::sessions::GetRecentlyClosed;
namespace GetDevices = api::sessions::GetDevices;
namespace Restore = api::sessions::Restore;

// Comparator function for use with std::sort that will sort sessions by
// descending modified_time (i.e., most recent first).
bool SortSessionsByRecency(const sync_sessions::SyncedSession* s1,
                           const sync_sessions::SyncedSession* s2) {
  return s1->GetModifiedTime() > s2->GetModifiedTime();
}

// Comparator function for use with std::sort that will sort tabs in a window
// by descending timestamp (i.e., most recent first).
bool SortTabsByRecency(const sessions::SessionTab* t1,
                       const sessions::SessionTab* t2) {
  return t1->timestamp > t2->timestamp;
}

api::tabs::Tab CreateTabModelHelper(
    const sessions::SerializedNavigationEntry& current_navigation,
    const std::string& session_id,
    int index,
    bool pinned,
    bool active,
    const Extension* extension,
    mojom::ContextType context) {
  api::tabs::Tab tab_struct;

  const GURL& url = current_navigation.virtual_url();
  std::string title = base::UTF16ToUTF8(current_navigation.title());

  tab_struct.session_id = session_id;
  tab_struct.url = url.spec();
  tab_struct.fav_icon_url = current_navigation.favicon_url().spec();
  if (!title.empty()) {
    tab_struct.title = title;
  } else {
    tab_struct.title = base::UTF16ToUTF8(url_formatter::FormatUrl(url));
  }
  tab_struct.index = index;
  tab_struct.pinned = pinned;
  tab_struct.active = active;

  // TODO(mshin): Enable the below code after migrating ExtensionTabUtil
  // ExtensionTabUtil::ScrubTabBehavior scrub_tab_behavior =
  //     ExtensionTabUtil::GetScrubTabBehavior(extension, context, url);
  // ExtensionTabUtil::ScrubTabForExtension(extension, nullptr, &tab_struct,
  //                                        scrub_tab_behavior);
  return tab_struct;
}

api::windows::Window CreateWindowModelHelper(
    std::vector<api::tabs::Tab> tabs,
    const std::string& session_id,
    const api::windows::WindowType& type,
    const api::windows::WindowState& state) {
  api::windows::Window window_struct;
  window_struct.tabs = std::move(tabs);
  window_struct.session_id = session_id;
  window_struct.incognito = false;
  window_struct.always_on_top = false;
  window_struct.focused = false;
  window_struct.type = type;
  window_struct.state = state;
  return window_struct;
}

api::sessions::Session CreateSessionModelHelper(
    int last_modified,
    std::optional<api::tabs::Tab> tab,
    std::optional<api::windows::Window> window,
    std::optional<api::tab_groups::TabGroup> group) {
  api::sessions::Session session_struct;
  session_struct.last_modified = last_modified;
  if (tab) {
    session_struct.tab = std::move(*tab);
  } else if (window) {
    session_struct.window = std::move(*window);
  } else if (group) {
    NOTREACHED();  // TODO(crbug.com/1192309): Implement group support.
  } else {
    NOTREACHED();
  }
  return session_struct;
}

}  // namespace

api::tabs::Tab SessionsGetRecentlyClosedFunction::CreateTabModel(
    const sessions::TabRestoreService::Tab& tab,
    bool active) {
  return CreateTabModelHelper(tab.navigations[tab.current_navigation_index],
                              base::NumberToString(tab.id.id()),
                              tab.tabstrip_index, tab.pinned, active,
                              extension(), source_context_type());
}

api::windows::Window SessionsGetRecentlyClosedFunction::CreateWindowModel(
    const sessions::TabRestoreService::Window& window) {
  DCHECK(!window.tabs.empty());

  std::vector<api::tabs::Tab> tabs;
  for (const auto& tab : window.tabs) {
    tabs.push_back(
        CreateTabModel(*tab, tab->tabstrip_index == window.selected_tab_index));
  }

  return CreateWindowModelHelper(
      std::move(tabs), base::NumberToString(window.id.id()),
      api::windows::WindowType::kNormal, api::windows::WindowState::kNormal);
}

api::tab_groups::TabGroup SessionsGetRecentlyClosedFunction::CreateGroupModel(
    const sessions::TabRestoreService::Group& group) {
  DCHECK(!group.tabs.empty());

  // TODO(mshin): Enable the below code after migrating tab_groups_util
  // return tab_groups_util::CreateTabGroupObject(group.group_id,
  //                                              group.visual_data);
  return api::tab_groups::TabGroup();
}

api::sessions::Session SessionsGetRecentlyClosedFunction::CreateSessionModel(
    const sessions::TabRestoreService::Entry& entry) {
  std::optional<api::tabs::Tab> tab;
  std::optional<api::windows::Window> window;
  std::optional<api::tab_groups::TabGroup> group;
  switch (entry.type) {
    case sessions::TabRestoreService::TAB:
      tab = CreateTabModel(
          static_cast<const sessions::TabRestoreService::Tab&>(entry), false);
      break;
    case sessions::TabRestoreService::WINDOW:
      window = CreateWindowModel(
          static_cast<const sessions::TabRestoreService::Window&>(entry));
      break;
    case sessions::TabRestoreService::GROUP:
      group = CreateGroupModel(
          static_cast<const sessions::TabRestoreService::Group&>(entry));
  }
  return CreateSessionModelHelper(entry.timestamp.ToTimeT(), std::move(tab),
                                  std::move(window), std::move(group));
}

ExtensionFunction::ResponseAction SessionsGetRecentlyClosedFunction::Run() {
  std::optional<GetRecentlyClosed::Params> params =
      GetRecentlyClosed::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);
  int max_results = api::sessions::MAX_SESSION_RESULTS;
  if (params->filter && params->filter->max_results)
    max_results = *params->filter->max_results;
  EXTENSION_FUNCTION_VALIDATE(max_results >= 0 &&
      max_results <= api::sessions::MAX_SESSION_RESULTS);

  std::vector<api::sessions::Session> result;
  // TODO(mshin): Support TabRestoreServiceFactory
  sessions::TabRestoreService* tab_restore_service = nullptr;
  //     TabRestoreServiceFactory::GetForProfile(browser_context());

  // TabRestoreServiceFactory::GetForProfile() can return nullptr (i.e., when in
  // incognito mode)
  if (!tab_restore_service) {
    DCHECK(browser_context()->IsOffTheRecord())
        << "sessions::TabRestoreService expected for normal profiles";
    return RespondNow(ArgumentList(GetRecentlyClosed::Results::Create(result)));
  }

  // List of entries. They are ordered from most to least recent.
  // We prune the list to contain max 25 entries at any time and removes
  // uninteresting entries.
  int counter = 0;
  for (const auto& entry : tab_restore_service->entries()) {
    // TODO(crbug.com/1192309): Support group entries in the Sessions API,
    // rather than sharding the group out into individual tabs.
    if (entry->type == sessions::TabRestoreService::GROUP) {
      auto& group =
          static_cast<const sessions::TabRestoreService::Group&>(*entry);
      for (const auto& tab : group.tabs)
        if (counter++ < max_results)
          result.push_back(CreateSessionModel(*tab));
        else
          break;
    } else {
      if (counter++ < max_results)
        result.push_back(CreateSessionModel(*entry));
      else
        break;
    }
  }

  return RespondNow(ArgumentList(GetRecentlyClosed::Results::Create(result)));
}

api::tabs::Tab SessionsGetDevicesFunction::CreateTabModel(
    const std::string& session_tag,
    const sessions::SessionTab& tab,
    int tab_index,
    bool active) {
  std::string session_id = SessionId(session_tag, tab.tab_id.id()).ToString();
  return CreateTabModelHelper(
      tab.navigations[tab.normalized_navigation_index()], session_id, tab_index,
      tab.pinned, active, extension(), source_context_type());
}

std::optional<api::windows::Window>
SessionsGetDevicesFunction::CreateWindowModel(
    const sessions::SessionWindow& window,
    const std::string& session_tag) {
  DCHECK(!window.tabs.empty());

  // Ignore app popup window for now because we do not have a corresponding
  // api::windows::WindowType value.
  if (window.type == sessions::SessionWindow::TYPE_APP_POPUP)
    return std::nullopt;

  // Prune tabs that are not syncable or are NewTabPage. Then, sort the tabs
  // from most recent to least recent.
  std::vector<const sessions::SessionTab*> tabs_in_window;
  for (const auto& tab : window.tabs) {
    if (tab->navigations.empty())
      continue;

    // We don't consider to handle NTP url
    // const sessions::SerializedNavigationEntry& current_navigation =
    //     tab->navigations.at(tab->normalized_navigation_index());
    // if (search::IsNTPOrRelatedURL(
    //         current_navigation.virtual_url(),
    //         Profile::FromBrowserContext(browser_context()))) {
    //   continue;
    // }
    tabs_in_window.push_back(tab.get());
  }
  if (tabs_in_window.empty())
    return std::nullopt;
  std::sort(tabs_in_window.begin(), tabs_in_window.end(), SortTabsByRecency);

  std::vector<api::tabs::Tab> tabs;
  for (size_t i = 0; i < tabs_in_window.size(); ++i) {
    tabs.push_back(CreateTabModel(session_tag, *tabs_in_window[i], i,
                                  window.selected_tab_index == (int)i));
  }

  std::string session_id =
      SessionId(session_tag, window.window_id.id()).ToString();

  api::windows::WindowType type = api::windows::WindowType::kNone;
  switch (window.type) {
    case sessions::SessionWindow::TYPE_NORMAL:
      type = api::windows::WindowType::kNormal;
      break;
    case sessions::SessionWindow::TYPE_POPUP:
      type = api::windows::WindowType::kPopup;
      break;
    case sessions::SessionWindow::TYPE_APP:
      type = api::windows::WindowType::kApp;
      break;
    case sessions::SessionWindow::TYPE_DEVTOOLS:
      type = api::windows::WindowType::kDevtools;
      break;
    case sessions::SessionWindow::TYPE_APP_POPUP:
      NOTREACHED();
  }

  api::windows::WindowState state = api::windows::WindowState::kNone;
  switch (window.show_state) {
    case ui::SHOW_STATE_NORMAL:
      state = api::windows::WindowState::kNormal;
      break;
    case ui::SHOW_STATE_MINIMIZED:
      state = api::windows::WindowState::kMinimized;
      break;
    case ui::SHOW_STATE_MAXIMIZED:
      state = api::windows::WindowState::kMaximized;
      break;
    case ui::SHOW_STATE_FULLSCREEN:
      state = api::windows::WindowState::kFullscreen;
      break;
    case ui::SHOW_STATE_DEFAULT:
    case ui::SHOW_STATE_INACTIVE:
    case ui::SHOW_STATE_END:
      break;
  }

  api::windows::Window window_struct =
      CreateWindowModelHelper(std::move(tabs), session_id, type, state);
  // TODO(dwankri): Dig deeper to resolve bounds not being optional, so closed
  // windows in GetRecentlyClosed can have set values in Window helper.
  window_struct.left = window.bounds.x();
  window_struct.top = window.bounds.y();
  window_struct.width = window.bounds.width();
  window_struct.height = window.bounds.height();

  return window_struct;
}

std::optional<api::sessions::Session>
SessionsGetDevicesFunction::CreateSessionModel(
    const sessions::SessionWindow& window,
    const std::string& session_tag) {
  std::optional<api::windows::Window> window_model =
      CreateWindowModel(window, session_tag);
  // There is a chance that after pruning uninteresting tabs the window will be
  // empty.
  if (!window_model)
    return std::nullopt;

  return CreateSessionModelHelper(window.timestamp.ToTimeT(), std::nullopt,
                                  std::move(window_model), std::nullopt);
}

api::sessions::Device SessionsGetDevicesFunction::CreateDeviceModel(
    const sync_sessions::SyncedSession* session) {
  int max_results = api::sessions::MAX_SESSION_RESULTS;
  // Already validated in RunAsync().
  std::optional<GetDevices::Params> params = GetDevices::Params::Create(args());
  if (params->filter && params->filter->max_results)
    max_results = *params->filter->max_results;

  api::sessions::Device device_struct;
  device_struct.info = session->GetSessionName();
  device_struct.device_name = session->GetSessionName();

  for (auto it = session->windows.begin();
       it != session->windows.end() &&
       static_cast<int>(device_struct.sessions.size()) < max_results;
       ++it) {
    std::optional<api::sessions::Session> session_model = CreateSessionModel(
        it->second->wrapped_window, session->GetSessionTag());
    if (session_model)
      device_struct.sessions.push_back(std::move(*session_model));
  }
  return device_struct;
}

ExtensionFunction::ResponseAction SessionsGetDevicesFunction::Run() {
  // TODO(mshin): Support SessionSyncServiceFactory
  sync_sessions::SessionSyncService* service =
      // SessionSyncServiceFactory::GetInstance()->GetForProfile(
      //     browser_context());
      nullptr;
  DCHECK(service);

  sync_sessions::OpenTabsUIDelegate* open_tabs =
      service->GetOpenTabsUIDelegate();
  std::vector<raw_ptr<const sync_sessions::SyncedSession, VectorExperimental>>
      sessions;
  // If the user has disabled tab sync, GetOpenTabsUIDelegate() returns null.
  if (!(open_tabs && open_tabs->GetAllForeignSessions(&sessions))) {
    return RespondNow(ArgumentList(
        GetDevices::Results::Create(std::vector<api::sessions::Device>())));
  }

  std::optional<GetDevices::Params> params = GetDevices::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);
  if (params->filter && params->filter->max_results) {
    EXTENSION_FUNCTION_VALIDATE(*params->filter->max_results >= 0 &&
        *params->filter->max_results <= api::sessions::MAX_SESSION_RESULTS);
  }

  std::vector<api::sessions::Device> result;
  // Sort sessions from most recent to least recent.
  std::sort(sessions.begin(), sessions.end(), SortSessionsByRecency);
  for (const sync_sessions::SyncedSession* session : sessions) {
    result.push_back(CreateDeviceModel(session));
  }

  return RespondNow(ArgumentList(GetDevices::Results::Create(result)));
}

ExtensionFunction::ResponseAction SessionsRestoreFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

SessionsEventRouter::SessionsEventRouter(content::BrowserContext* context)
    : browser_context_(context)
    // TODO(mshin): Support TabRestoreServiceFactory
    /*, tab_restore_service_(TabRestoreServiceFactory::GetForProfile(browser_context_))*/ {
  // TabRestoreServiceFactory::GetForProfile() can return nullptr (i.e., when in
  // incognito mode)
  if (tab_restore_service_) {
    tab_restore_service_->LoadTabsFromLastSession();
    tab_restore_service_->AddObserver(this);
  }
}

SessionsEventRouter::~SessionsEventRouter() {
  if (tab_restore_service_)
    tab_restore_service_->RemoveObserver(this);
}

void SessionsEventRouter::TabRestoreServiceChanged(
    sessions::TabRestoreService* service) {
  EventRouter::Get(browser_context_)->BroadcastEvent(std::make_unique<Event>(
      events::SESSIONS_ON_CHANGED, api::sessions::OnChanged::kEventName,
      base::Value::List()));
}

void SessionsEventRouter::TabRestoreServiceDestroyed(
    sessions::TabRestoreService* service) {
  tab_restore_service_ = nullptr;
}

SessionsAPI::SessionsAPI(content::BrowserContext* context)
    : browser_context_(context) {
  EventRouter::Get(browser_context_)->RegisterObserver(this,
      api::sessions::OnChanged::kEventName);
}

SessionsAPI::~SessionsAPI() {
}

void SessionsAPI::Shutdown() {
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

static base::LazyInstance<BrowserContextKeyedAPIFactory<SessionsAPI>>::
    DestructorAtExit g_sessions_api_factory = LAZY_INSTANCE_INITIALIZER;

BrowserContextKeyedAPIFactory<SessionsAPI>*
SessionsAPI::GetFactoryInstance() {
  return g_sessions_api_factory.Pointer();
}

void SessionsAPI::OnListenerAdded(const EventListenerInfo& details) {
  sessions_event_router_ = std::make_unique<SessionsEventRouter>(browser_context_);
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

}  // namespace extensions
