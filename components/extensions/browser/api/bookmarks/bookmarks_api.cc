// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/bookmarks/bookmarks_api.h"

#include <stddef.h>

#include <limits>
#include <memory>
#include <string>
#include <utility>

#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/lazy_instance.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "components/extensions/browser/api/bookmarks/bookmark_api_constants.h"
#include "components/extensions/common/api/bookmarks.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/browser/bookmark_utils.h"
#include "components/bookmarks/common/bookmark_metrics.h"
#include "components/bookmarks/common/bookmark_pref_names.h"
#include "components/bookmarks/managed/managed_bookmark_service.h"
#include "components/prefs/pref_service.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_function_dispatcher.h"
#include "ui/base/l10n/l10n_util.h"

using bookmarks::BookmarkModel;
using bookmarks::BookmarkNode;
using bookmarks::ManagedBookmarkService;

namespace extensions {

using api::bookmarks::BookmarkTreeNode;
using api::bookmarks::CreateDetails;
using content::BrowserContext;
using content::BrowserThread;
using content::WebContents;

ExtensionFunction::ResponseAction BookmarksFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

BookmarkModel* BookmarksFunction::GetBookmarkModel() {
  // TODO(mshin): Implement BookmarkModelFactory
  // return BookmarkModelFactory::GetForBrowserContext(browser_context());
  return nullptr;
}

ManagedBookmarkService* BookmarksFunction::GetManagedBookmarkService() {
  // TODO(mshin): Implement ManagedBookmarkServiceFactory
  // return ManagedBookmarkServiceFactory::GetForProfile(browser_context());
  return nullptr;
}

const BookmarkNode* BookmarksFunction::GetBookmarkNodeFromId(
    const std::string& id_string,
    std::string* error) {
  int64_t id;
  if (!base::StringToInt64(id_string, &id)) {
    *error = bookmark_api_constants::kInvalidIdError;
    return nullptr;
  }

  // TODO(mshin): Implement BookmarkModelFactory
  const BookmarkNode* node = nullptr;
  //     bookmarks::GetBookmarkNodeByID(
  //         BookmarkModelFactory::GetForBrowserContext(browser_context()), id);
  if (!node)
    *error = bookmark_api_constants::kNoNodeError;

  return node;
}

const BookmarkNode* BookmarksFunction::CreateBookmarkNode(
    BookmarkModel* model,
    const CreateDetails& details,
    std::string* error) {
  int64_t parent_id;

  if (!details.parent_id) {
    // Optional, default to "other bookmarks".
    parent_id = model->other_node()->id();
  } else if (!base::StringToInt64(*details.parent_id, &parent_id)) {
    *error = bookmark_api_constants::kInvalidIdError;
    return nullptr;
  }
  const BookmarkNode* parent = bookmarks::GetBookmarkNodeByID(model, parent_id);
  if (!CanBeModified(parent, error)) {
    return nullptr;
  }
  if (!parent->is_folder()) {
    *error = bookmark_api_constants::kInvalidParentError;
    return nullptr;
  }

  size_t index;
  if (!details.index) {  // Optional (defaults to end).
    index = parent->children().size();
  } else {
    if (*details.index < 0 ||
        static_cast<size_t>(*details.index) > parent->children().size()) {
      *error = bookmark_api_constants::kInvalidIndexError;
      return nullptr;
    }
    index = static_cast<size_t>(*details.index);
  }

  std::u16string title;  // Optional.
  if (details.title)
    title = base::UTF8ToUTF16(*details.title);

  std::string url_string;  // Optional.
  if (details.url)
    url_string = *details.url;

  GURL url(url_string);
  if (!url_string.empty() && !url.is_valid()) {
    *error = bookmark_api_constants::kInvalidUrlError;
    return nullptr;
  }

  const BookmarkNode* node;
  if (url_string.length()) {
    node = model->AddNewURL(parent, index, title, url);
  } else {
    node = model->AddFolder(parent, index, title);
    model->SetDateFolderModified(parent, base::Time::Now());
  }

  DCHECK(node);

  return node;
}

bool BookmarksFunction::EditBookmarksEnabled() {
  PrefService* prefs = user_prefs::UserPrefs::Get(browser_context());
  return prefs->GetBoolean(bookmarks::prefs::kEditBookmarksEnabled);
}

bool BookmarksFunction::CanBeModified(const BookmarkNode* node,
                                      std::string* error) {
  if (!node) {
    *error = bookmark_api_constants::kNoParentError;
    return false;
  }
  if (node->is_root()) {
    *error = bookmark_api_constants::kModifySpecialError;
    return false;
  }
  ManagedBookmarkService* managed = GetManagedBookmarkService();
  if (bookmarks::IsDescendantOf(node, managed->managed_node())) {
    *error = bookmark_api_constants::kModifyManagedError;
    return false;
  }
  return true;
}

void BookmarksFunction::OnResponded() {
  DCHECK(response_type());
  if (*response_type() == ExtensionFunction::SUCCEEDED) {
    // BookmarksApiWatcher::GetForBrowserContext(browser_context())
    //     ->NotifyApiInvoked(extension(), this);
  }
}

void BookmarksFunction::BookmarkModelChanged() {
}

void BookmarksFunction::BookmarkModelLoaded(bool ids_reassigned) {
  GetBookmarkModel()->RemoveObserver(this);

  ResponseValue response = RunOnReady();
  Respond(std::move(response));

  Release();  // Balanced in Run().
}

BookmarkEventRouter::BookmarkEventRouter(content::BrowserContext* context)
    : browser_context_(context)
      // TODO(mshin): Implement BookmarkModelFactory and ManagedBookmarkServiceFactory
      /*,
      model_(BookmarkModelFactory::GetForBrowserContext(context)),
      managed_(ManagedBookmarkServiceFactory::GetForProfile(context))*/ {
  model_->AddObserver(this);
}

BookmarkEventRouter::~BookmarkEventRouter() {
  if (model_) {
    model_->RemoveObserver(this);
  }
}

void BookmarkEventRouter::DispatchEvent(events::HistogramValue histogram_value,
                                        const std::string& event_name,
                                        base::Value::List event_args) {
  EventRouter* event_router = EventRouter::Get(browser_context_);
  if (event_router) {
    event_router->BroadcastEvent(std::make_unique<extensions::Event>(
        histogram_value, event_name, std::move(event_args)));
  }
}

void BookmarkEventRouter::BookmarkModelLoaded(bool ids_reassigned) {
  // TODO(erikkay): Perhaps we should send this event down to the extension
  // so they know when it's safe to use the API?
}

void BookmarkEventRouter::BookmarkModelBeingDeleted() {
  model_ = nullptr;
}

void BookmarkEventRouter::BookmarkNodeMoved(const BookmarkNode* old_parent,
                                            size_t old_index,
                                            const BookmarkNode* new_parent,
                                            size_t new_index) {
  const BookmarkNode* node = new_parent->children()[new_index].get();
  api::bookmarks::OnMoved::MoveInfo move_info;
  move_info.parent_id = base::NumberToString(new_parent->id());
  move_info.index = static_cast<int>(new_index);
  move_info.old_parent_id = base::NumberToString(old_parent->id());
  move_info.old_index = static_cast<int>(old_index);

  DispatchEvent(events::BOOKMARKS_ON_MOVED, api::bookmarks::OnMoved::kEventName,
                api::bookmarks::OnMoved::Create(
                    base::NumberToString(node->id()), move_info));
}

void BookmarkEventRouter::BookmarkNodeAdded(const BookmarkNode* parent,
                                            size_t index,
                                            bool added_by_user) {
}

void BookmarkEventRouter::BookmarkNodeRemoved(
    const BookmarkNode* parent,
    size_t index,
    const BookmarkNode* node,
    const std::set<GURL>& removed_urls) {
}

void BookmarkEventRouter::BookmarkAllUserNodesRemoved(
    const std::set<GURL>& removed_urls) {
  // TODO(crbug.com/1468324): This used to be used only on Android, but that's
  // no longer the case. We need to implement a new event to handle this.
}

void BookmarkEventRouter::BookmarkNodeChanged(const BookmarkNode* node) {
  // TODO(erikkay) The only three things that BookmarkModel sends this
  // notification for are title, url and favicon.  Since we're currently
  // ignoring favicon and since the notification doesn't say which one anyway,
  // for now we only include title and url.  The ideal thing would be to change
  // BookmarkModel to indicate what changed.
  api::bookmarks::OnChanged::ChangeInfo change_info;
  change_info.title = base::UTF16ToUTF8(node->GetTitle());
  if (node->is_url())
    change_info.url = node->url().spec();

  DispatchEvent(events::BOOKMARKS_ON_CHANGED,
                api::bookmarks::OnChanged::kEventName,
                api::bookmarks::OnChanged::Create(
                    base::NumberToString(node->id()), change_info));
}

void BookmarkEventRouter::BookmarkNodeFaviconChanged(const BookmarkNode* node) {
  // TODO(erikkay) anything we should do here?
}

void BookmarkEventRouter::BookmarkNodeChildrenReordered(
    const BookmarkNode* node) {
  api::bookmarks::OnChildrenReordered::ReorderInfo reorder_info;
  for (const auto& child : node->children())
    reorder_info.child_ids.push_back(base::NumberToString(child->id()));

  DispatchEvent(events::BOOKMARKS_ON_CHILDREN_REORDERED,
                api::bookmarks::OnChildrenReordered::kEventName,
                api::bookmarks::OnChildrenReordered::Create(
                    base::NumberToString(node->id()), reorder_info));
}

void BookmarkEventRouter::ExtensiveBookmarkChangesBeginning() {
  DispatchEvent(events::BOOKMARKS_ON_IMPORT_BEGAN,
                api::bookmarks::OnImportBegan::kEventName,
                api::bookmarks::OnImportBegan::Create());
}

void BookmarkEventRouter::ExtensiveBookmarkChangesEnded() {
  DispatchEvent(events::BOOKMARKS_ON_IMPORT_ENDED,
                api::bookmarks::OnImportEnded::kEventName,
                api::bookmarks::OnImportEnded::Create());
}

BookmarksAPI::BookmarksAPI(BrowserContext* context)
    : browser_context_(context) {
  EventRouter* event_router = EventRouter::Get(browser_context_);
  event_router->RegisterObserver(this, api::bookmarks::OnCreated::kEventName);
  event_router->RegisterObserver(this, api::bookmarks::OnRemoved::kEventName);
  event_router->RegisterObserver(this, api::bookmarks::OnChanged::kEventName);
  event_router->RegisterObserver(this, api::bookmarks::OnMoved::kEventName);
  event_router->RegisterObserver(
      this, api::bookmarks::OnChildrenReordered::kEventName);
  event_router->RegisterObserver(this,
                                 api::bookmarks::OnImportBegan::kEventName);
  event_router->RegisterObserver(this,
                                 api::bookmarks::OnImportEnded::kEventName);
}

BookmarksAPI::~BookmarksAPI() {
}

void BookmarksAPI::Shutdown() {
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

static base::LazyInstance<BrowserContextKeyedAPIFactory<BookmarksAPI>>::
    DestructorAtExit g_bookmarks_api_factory = LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<BookmarksAPI>*
BookmarksAPI::GetFactoryInstance() {
  return g_bookmarks_api_factory.Pointer();
}

void BookmarksAPI::OnListenerAdded(const EventListenerInfo& details) {
  bookmark_event_router_ = std::make_unique<BookmarkEventRouter>(
      browser_context_);
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

ExtensionFunction::ResponseValue BookmarksGetFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue BookmarksGetChildrenFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue BookmarksGetRecentFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue BookmarksGetTreeFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue BookmarksGetSubTreeFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue BookmarksSearchFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue BookmarksRemoveFunctionBase::RunOnReady() {
  return Error("Not Implement");
}

bool BookmarksRemoveFunction::is_recursive() const {
  return false;
}

bool BookmarksRemoveTreeFunction::is_recursive() const {
  return true;
}

ExtensionFunction::ResponseValue BookmarksCreateFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue BookmarksMoveFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue BookmarksUpdateFunction::RunOnReady() {
  return Error("Not Implement");
}

}  // namespace extensions
