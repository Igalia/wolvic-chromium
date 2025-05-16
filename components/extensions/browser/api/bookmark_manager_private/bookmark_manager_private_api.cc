// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/bookmark_manager_private/bookmark_manager_private_api.h"

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <utility>
#include <vector>

#include "base/i18n/file_util_icu.h"
#include "base/i18n/time_formatting.h"
#include "base/lazy_instance.h"
#include "base/memory/raw_ptr.h"
#include "base/notreached.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/thread_pool.h"
#include "base/time/time.h"
#include "base/values.h"
#include "components/extensions/common/api/bookmark_manager_private.h"
#include "components/extensions/common/extension_constants.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/bookmarks/browser/bookmark_node_data.h"
#include "components/bookmarks/browser/bookmark_utils.h"
#include "components/bookmarks/browser/scoped_group_bookmark_actions.h"
#include "components/bookmarks/common/bookmark_metrics.h"
#include "components/bookmarks/common/bookmark_pref_names.h"
#include "components/bookmarks/managed/managed_bookmark_service.h"
#include "components/prefs/pref_service.h"
#include "components/strings/grit/components_strings.h"
#include "components/undo/bookmark_undo_service.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "extensions/browser/extension_function_dispatcher.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/view_type_utils.h"
#include "extensions/common/mojom/view_type.mojom.h"
#include "ui/base/dragdrop/mojom/drag_drop_types.mojom-shared.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/webui/web_ui_util.h"
#include "ui/shell_dialogs/selected_file_info.h"

using bookmarks::BookmarkModel;
using bookmarks::BookmarkNode;
using bookmarks::BookmarkNodeData;

namespace extensions {

BookmarkManagerPrivateEventRouter::BookmarkManagerPrivateEventRouter(
    content::BrowserContext* browser_context,
    BookmarkModel* bookmark_model)
    : browser_context_(browser_context), bookmark_model_(bookmark_model) {
  bookmark_model_->AddObserver(this);
}

BookmarkManagerPrivateEventRouter::~BookmarkManagerPrivateEventRouter() {
  if (bookmark_model_)
    bookmark_model_->RemoveObserver(this);
}

void BookmarkManagerPrivateEventRouter::DispatchEvent(
    events::HistogramValue histogram_value,
    const std::string& event_name,
    base::Value::List event_args) {
  EventRouter::Get(browser_context_)
      ->BroadcastEvent(std::make_unique<Event>(histogram_value, event_name,
                                               std::move(event_args)));
}

void BookmarkManagerPrivateEventRouter::BookmarkModelChanged() {}

void BookmarkManagerPrivateEventRouter::BookmarkModelBeingDeleted() {
  bookmark_model_ = nullptr;
}

BookmarkManagerPrivateAPI::BookmarkManagerPrivateAPI(
    content::BrowserContext* browser_context)
    : browser_context_(browser_context) {
}

BookmarkManagerPrivateAPI::~BookmarkManagerPrivateAPI() = default;

void BookmarkManagerPrivateAPI::Shutdown() {
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

static base::LazyInstance<
    BrowserContextKeyedAPIFactory<BookmarkManagerPrivateAPI>>::DestructorAtExit
    g_bookmark_manager_private_api_factory = LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<BookmarkManagerPrivateAPI>*
BookmarkManagerPrivateAPI::GetFactoryInstance() {
  return g_bookmark_manager_private_api_factory.Pointer();
}

void BookmarkManagerPrivateAPI::OnListenerAdded(
    const EventListenerInfo& details) {
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
  // TODO(mshin): Enable the below code after supporting Bookmark
  // event_router_ = std::make_unique<BookmarkManagerPrivateEventRouter>(
  //     browser_context_,
  //     BookmarkModelFactory::GetForBrowserContext(browser_context_));
}

BookmarkManagerPrivateDragEventRouter::BookmarkManagerPrivateDragEventRouter(
    content::WebContents* web_contents)
    : content::WebContentsUserData<BookmarkManagerPrivateDragEventRouter>(
          *web_contents),
      browser_context_(web_contents->GetBrowserContext()) {
}

BookmarkManagerPrivateDragEventRouter::
    ~BookmarkManagerPrivateDragEventRouter() {
  // No need to remove ourselves as the BookmarkTabHelper's delegate, since they
  // are both WebContentsUserData and will be deleted at the same time.
}

void BookmarkManagerPrivateDragEventRouter::DispatchEvent(
    events::HistogramValue histogram_value,
    const std::string& event_name,
    base::Value::List args) {
  EventRouter* event_router = EventRouter::Get(browser_context_);
  if (!event_router)
    return;

  std::unique_ptr<Event> event(
      new Event(histogram_value, event_name, std::move(args)));
  event_router->BroadcastEvent(std::move(event));
}

// TODO(mshin): Enable below code after support Bookmark
// void BookmarkManagerPrivateDragEventRouter::OnDragEnter(
//     const BookmarkNodeData& data) {
//   if (!data.is_valid())
//     return;
//   DispatchEvent(events::BOOKMARK_MANAGER_PRIVATE_ON_DRAG_ENTER,
//                 bookmark_manager_private::OnDragEnter::kEventName,
//                 bookmark_manager_private::OnDragEnter::Create(
//                     CreateApiBookmarkNodeData(browser_context_, data)));
// }

// void BookmarkManagerPrivateDragEventRouter::OnDragOver(
//     const BookmarkNodeData& data) {
//   // Intentionally empty since these events happens too often and floods the
//   // message queue. We do not need this event for the bookmark manager anyway.
// }

// void BookmarkManagerPrivateDragEventRouter::OnDragLeave(
//     const BookmarkNodeData& data) {
//   if (!data.is_valid())
//     return;
//   DispatchEvent(events::BOOKMARK_MANAGER_PRIVATE_ON_DRAG_LEAVE,
//                 bookmark_manager_private::OnDragLeave::kEventName,
//                 bookmark_manager_private::OnDragLeave::Create(
//                     CreateApiBookmarkNodeData(browser_context_, data)));
// }

// void BookmarkManagerPrivateDragEventRouter::OnDrop(
//     const BookmarkNodeData& data) {
//   if (!data.is_valid())
//     return;
//   DispatchEvent(events::BOOKMARK_MANAGER_PRIVATE_ON_DROP,
//                 bookmark_manager_private::OnDrop::kEventName,
//                 bookmark_manager_private::OnDrop::Create(
//                     CreateApiBookmarkNodeData(browser_context_, data)));

//   // Make a copy that is owned by this instance.
//   ClearBookmarkNodeData();
//   bookmark_drag_data_ = data;
// }

const BookmarkNodeData*
BookmarkManagerPrivateDragEventRouter::GetBookmarkNodeData() {
  if (bookmark_drag_data_.is_valid())
    return &bookmark_drag_data_;
  return nullptr;
}

void BookmarkManagerPrivateDragEventRouter::ClearBookmarkNodeData() {
  bookmark_drag_data_.Clear();
}

ExtensionFunction::ResponseValue ClipboardBookmarkManagerFunction::CopyOrCut(
    bool cut,
    const std::vector<std::string>& id_list) {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateCopyFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateCutFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivatePasteFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateCanPasteFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateSortChildrenFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateStartDragFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateDropFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateGetSubtreeFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateRemoveTreesFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateUndoFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateRedoFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateOpenInNewTabFunction::RunOnReady() {
  return Error("Not Implement");
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateOpenInNewWindowFunction::RunOnReady() {
  return Error("Not Implement");
}

BookmarkManagerPrivateIOFunction::BookmarkManagerPrivateIOFunction() {}

BookmarkManagerPrivateIOFunction::~BookmarkManagerPrivateIOFunction() {
  // There may be pending file dialogs, we need to tell them that we've gone
  // away so they don't try and call back to us.
  if (select_file_dialog_.get())
    select_file_dialog_->ListenerDestroyed();
}

void BookmarkManagerPrivateIOFunction::ShowSelectFileDialog(
    ui::SelectFileDialog::Type type,
    const base::FilePath& default_path) {
  if (!dispatcher())
    return;  // Extension was unloaded.

  // Early return if the select file dialog is already active.
  if (select_file_dialog_)
    return;

  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  // Balanced in one of the callbacks of SelectFileDialog:
  // either FileSelectionCanceled, or FileSelected
  AddRef();

  // TODO(mshin): Enable the below code after support Bookmark
  // WebContents* web_contents = GetSenderWebContents();

  // select_file_dialog_ = ui::SelectFileDialog::Create(
  //     this, std::make_unique<ChromeSelectFilePolicy>(web_contents));
  // ui::SelectFileDialog::FileTypeInfo file_type_info;
  // file_type_info.extensions.resize(1);
  // file_type_info.extensions[0].push_back(FILE_PATH_LITERAL("html");
  // gfx::NativeWindow owning_window =
  //     web_contents ? platform_util::GetTopLevel(web_contents->GetNativeView())
  //                  : gfx::NativeWindow();
  // // |web_contents| can be nullptr (for background pages), which is fine. In
  // // such a case if file-selection dialogs are forbidden by policy, we will not
  // // show an InfoBar, which is better than letting one appear out of the blue.
  // select_file_dialog_->SelectFile(
  //     type, std::u16string(), default_path, &file_type_info, 0,
  //     base::FilePath::StringType(), owning_window, nullptr);
}

void BookmarkManagerPrivateIOFunction::FileSelectionCanceled(void* params) {
  select_file_dialog_.reset();
  Release();  // Balanced in BookmarkManagerPrivateIOFunction::SelectFile()
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateImportFunction::RunOnReady() {
  // TODO(mshin): Enable the below code after support Bookmark
  // if (!EditBookmarksEnabled())
  //   return Error(bookmark_api_constants::kEditBookmarksDisabled);
  // ShowSelectFileDialog(ui::SelectFileDialog::SELECT_OPEN_FILE,
  //                      base::FilePath());
  // // TODO(crbug.com/1073255): This will respond before a file is selected, which
  // // seems incorrect. Waiting and responding until after
  // // ui::SelectFileDialog::Listener is fired should be right thing to do, but
  // // that requires auditing bookmark page callsites.
  return NoArguments();
}

void BookmarkManagerPrivateImportFunction::FileSelected(
    const ui::SelectedFileInfo& file,
    int index,
    void* params) {
  // TODO(mshin): Enable the below code after support Bookmark
  // Deletes itself.
  // ExternalProcessImporterHost* importer_host = new ExternalProcessImporterHost;
  // importer::SourceProfile source_profile;
  // source_profile.importer_type = importer::TYPE_BOOKMARKS_FILE;
  // source_profile.source_path = file.path();
  // importer_host->StartImportSettings(source_profile,
  //                                    browser_context_,
  //                                    importer::FAVORITES,
  //                                    new ProfileWriter(browser_context_));

  // importer::LogImporterUseToMetrics("BookmarksAPI",
  //                                   importer::TYPE_BOOKMARKS_FILE);
  // select_file_dialog_.reset();
  Release();  // Balanced in BookmarkManagerPrivateIOFunction::SelectFile()
}

ExtensionFunction::ResponseValue
BookmarkManagerPrivateExportFunction::RunOnReady() {
  return Error("Not Implement");
}

void BookmarkManagerPrivateExportFunction::FileSelected(
    const ui::SelectedFileInfo& file,
    int index,
    void* params) {
  // TODO(mshin): Enable the below code after support Bookmark
  // bookmark_html_writer::WriteBookmarks(browser_context_, file.path(), nullptr);
  // select_file_dialog_.reset();
  Release();  // Balanced in BookmarkManagerPrivateIOFunction::SelectFile()
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(BookmarkManagerPrivateDragEventRouter);

}  // namespace extensions
