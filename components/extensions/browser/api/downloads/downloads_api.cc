// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/downloads/downloads_api.h"

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>

#include "base/containers/flat_map.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/functional/callback_helpers.h"
#include "base/i18n/time_formatting.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/memory/weak_ptr.h"
#include "base/metrics/histogram_macros.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/cancelable_task_tracker.h"
#include "base/task/current_thread.h"
#include "base/task/single_thread_task_runner.h"
#include "base/values.h"
#include "build/build_config.h"
#include "components/extensions/common/api/downloads.h"
#include "components/download/public/common/download_danger_type.h"
#include "components/download/public/common/download_interrupt_reasons.h"
#include "components/download/public/common/download_item.h"
#include "components/download/public/common/download_url_parameters.h"
#include "components/web_modal/web_contents_modal_dialog_manager.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_function_dispatcher.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/warning_service.h"
#include "extensions/common/extension_id.h"
#include "extensions/common/mojom/context_type.mojom.h"
#include "extensions/common/mojom/event_dispatcher.mojom-forward.h"
#include "extensions/common/permissions/permissions_data.h"
#include "net/base/filename_util.h"
#include "net/base/load_flags.h"
#include "net/http/http_util.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/base/webui/web_ui_util.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_skia_rep.h"

using content::BrowserContext;
using content::BrowserThread;
using content::DownloadManager;
using download::DownloadItem;
using download::DownloadPathReservationTracker;
using extensions::mojom::APIPermissionID;

namespace download_extension_errors {

const char kEmptyFile[] = "Filename not yet determined";
const char kFileAlreadyDeleted[] = "Download file already deleted";
const char kFileNotRemoved[] = "Unable to remove file";
const char kIconNotFound[] = "Icon not found";
const char kInvalidDangerType[] = "Invalid danger type";
const char kInvalidFilename[] = "Invalid filename";
const char kInvalidFilter[] = "Invalid query filter";
const char kInvalidHeaderName[] = "Invalid request header name";
const char kInvalidHeaderUnsafe[] = "Unsafe request header name";
const char kInvalidHeaderValue[] = "Invalid request header value";
const char kInvalidId[] = "Invalid downloadId";
const char kInvalidOrderBy[] = "Invalid orderBy field";
const char kInvalidQueryLimit[] = "Invalid query limit";
const char kInvalidState[] = "Invalid state";
const char kInvalidURL[] = "Invalid URL";
const char kInvisibleContext[] =
    "Javascript execution context is not visible "
    "(tab, window, popup bubble)";
const char kNotComplete[] = "Download must be complete";
const char kNotDangerous[] = "Download must be dangerous";
const char kNotInProgress[] = "Download must be in progress";
const char kNotResumable[] = "DownloadItem.canResume must be true";
const char kOpenPermission[] = "The \"downloads.open\" permission is required";
const char kShelfDisabled[] = "Another extension has disabled the shelf";
const char kShelfPermission[] =
    "downloads.setShelfEnabled requires the "
    "\"downloads.shelf\" permission";
const char kTooManyListeners[] =
    "Each extension may have at most one "
    "onDeterminingFilename listener between all of its renderer execution "
    "contexts.";
const char kUiDisabled[] = "Another extension has disabled the download UI";
const char kUiPermission[] =
    "downloads.setUiOptions requires the "
    "\"downloads.ui\" permission";
const char kUnexpectedDeterminer[] = "Unexpected determineFilename call";
const char kUserGesture[] = "User gesture required";

}  // namespace download_extension_errors

namespace extensions {

namespace {

namespace downloads = api::downloads;

bool ShouldExport(const DownloadItem& download_item) {
  return !download_item.IsTemporary() &&
         download_item.GetDownloadSource() !=
             download::DownloadSource::INTERNAL_API;
}

}  // namespace

const char DownloadedByExtension::kKey[] = "DownloadItem DownloadedByExtension";

DownloadedByExtension* DownloadedByExtension::Get(
    download::DownloadItem* item) {
  base::SupportsUserData::Data* data = item->GetUserData(kKey);
  return (data == nullptr) ? nullptr
                           : static_cast<DownloadedByExtension*>(data);
}

DownloadedByExtension::DownloadedByExtension(download::DownloadItem* item,
                                             const ExtensionId& id,
                                             const std::string& name)
    : id_(id), name_(name) {
  item->SetUserData(kKey, base::WrapUnique(this));
}

DownloadsDownloadFunction::DownloadsDownloadFunction() {}

DownloadsDownloadFunction::~DownloadsDownloadFunction() {}

ExtensionFunction::ResponseAction DownloadsDownloadFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DownloadsSearchFunction::DownloadsSearchFunction() {}

DownloadsSearchFunction::~DownloadsSearchFunction() {}

ExtensionFunction::ResponseAction DownloadsSearchFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DownloadsPauseFunction::DownloadsPauseFunction() {}

DownloadsPauseFunction::~DownloadsPauseFunction() {}

ExtensionFunction::ResponseAction DownloadsPauseFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DownloadsResumeFunction::DownloadsResumeFunction() {}

DownloadsResumeFunction::~DownloadsResumeFunction() {}

ExtensionFunction::ResponseAction DownloadsResumeFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DownloadsCancelFunction::DownloadsCancelFunction() {}

DownloadsCancelFunction::~DownloadsCancelFunction() {}

ExtensionFunction::ResponseAction DownloadsCancelFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DownloadsEraseFunction::DownloadsEraseFunction() {}

DownloadsEraseFunction::~DownloadsEraseFunction() {}

ExtensionFunction::ResponseAction DownloadsEraseFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DownloadsRemoveFileFunction::DownloadsRemoveFileFunction() {}

DownloadsRemoveFileFunction::~DownloadsRemoveFileFunction() {}

ExtensionFunction::ResponseAction DownloadsRemoveFileFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void DownloadsRemoveFileFunction::Done(bool success) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  if (!success) {
    Respond(Error(download_extension_errors::kFileNotRemoved));
  } else {
    Respond(NoArguments());
  }
}

DownloadsAcceptDangerFunction::DownloadsAcceptDangerFunction() {}

DownloadsAcceptDangerFunction::~DownloadsAcceptDangerFunction() {}

ExtensionFunction::ResponseAction DownloadsAcceptDangerFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DownloadsShowFunction::DownloadsShowFunction() {}

DownloadsShowFunction::~DownloadsShowFunction() {}

ExtensionFunction::ResponseAction DownloadsShowFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DownloadsShowDefaultFolderFunction::DownloadsShowDefaultFolderFunction() {}

DownloadsShowDefaultFolderFunction::~DownloadsShowDefaultFolderFunction() {}

ExtensionFunction::ResponseAction DownloadsShowDefaultFolderFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DownloadsOpenFunction::DownloadsOpenFunction() {}

DownloadsOpenFunction::~DownloadsOpenFunction() {}

ExtensionFunction::ResponseAction DownloadsOpenFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DownloadsSetShelfEnabledFunction::DownloadsSetShelfEnabledFunction() {}

DownloadsSetShelfEnabledFunction::~DownloadsSetShelfEnabledFunction() {}

ExtensionFunction::ResponseAction DownloadsSetShelfEnabledFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DownloadsSetUiOptionsFunction::DownloadsSetUiOptionsFunction() = default;

DownloadsSetUiOptionsFunction::~DownloadsSetUiOptionsFunction() = default;

ExtensionFunction::ResponseAction DownloadsSetUiOptionsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DownloadsGetFileIconFunction::DownloadsGetFileIconFunction() {}

DownloadsGetFileIconFunction::~DownloadsGetFileIconFunction() {}


ExtensionFunction::ResponseAction DownloadsGetFileIconFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionDownloadsEventRouter::ExtensionDownloadsEventRouter(
    content::BrowserContext* browser_context,
    DownloadManager* manager)
    : browser_context_(browser_context), notifier_(manager, this) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(browser_context_);
  extension_registry_observation_.Observe(ExtensionRegistry::Get(browser_context_));
  EventRouter* router = EventRouter::Get(browser_context_);
  if (router)
    router->RegisterObserver(this,
                             downloads::OnDeterminingFilename::kEventName);
}

ExtensionDownloadsEventRouter::~ExtensionDownloadsEventRouter() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  EventRouter* router = EventRouter::Get(browser_context_);
  if (router)
    router->UnregisterObserver(this);
}

void ExtensionDownloadsEventRouter::
    SetDetermineFilenameTimeoutSecondsForTesting(int s) {
}

void ExtensionDownloadsEventRouter::SetUiEnabled(const Extension* extension,
                                                 bool enabled) {
  auto iter = ui_disabling_extensions_.find(extension);
  if (iter == ui_disabling_extensions_.end()) {
    if (!enabled)
      ui_disabling_extensions_.insert(extension);
  } else if (enabled) {
    ui_disabling_extensions_.erase(extension);
  }
}

bool ExtensionDownloadsEventRouter::IsUiEnabled() const {
  return ui_disabling_extensions_.empty();
}

// The method by which extensions hook into the filename determination process
// is based on the method by which the omnibox API allows extensions to hook
// into the omnibox autocompletion process. Extensions that wish to play a part
// in the filename determination process call
// chrome.downloads.onDeterminingFilename.addListener, which adds an
// EventListener object to ExtensionEventRouter::listeners().
//
// When a download's filename is being determined, DownloadTargetDeterminer (via
// ChromeDownloadManagerDelegate (CDMD) ::NotifyExtensions()) passes a callback
// to ExtensionDownloadsEventRouter::OnDeterminingFilename (ODF), which stores
// the callback in the item's ExtensionDownloadsEventRouterData (EDERD) along
// with all of the extension IDs that are listening for onDeterminingFilename
// events. ODF dispatches chrome.downloads.onDeterminingFilename.
//
// When the extension's event handler calls |suggestCallback|,
// downloads_custom_bindings.js calls
// DownloadsInternalDetermineFilenameFunction::RunAsync, which calls
// EDER::DetermineFilename, which notifies the item's EDERD.
//
// When the last extension's event handler returns, EDERD invokes the callback
// that CDMD passed to ODF, allowing DownloadTargetDeterminer to continue the
// filename determination process. If multiple extensions wish to override the
// filename, then the extension that was last installed wins.

void ExtensionDownloadsEventRouter::OnDeterminingFilename(
    DownloadItem* item,
    const base::FilePath& suggested_path,
    FilenameChangedCallback filename_changed_callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
}

void ExtensionDownloadsEventRouter::DetermineFilenameInternal(
    const base::FilePath& filename,
    downloads::FilenameConflictAction conflict_action,
    const ExtensionId& suggesting_extension_id,
    const base::Time& suggesting_install_time,
    const ExtensionId& incumbent_extension_id,
    const base::Time& incumbent_install_time,
    ExtensionId* winner_extension_id,
    base::FilePath* determined_filename,
    downloads::FilenameConflictAction* determined_conflict_action,
    WarningSet* warnings) {
  DCHECK(!filename.empty() ||
         (conflict_action != downloads::FilenameConflictAction::kUniquify));
  DCHECK(!suggesting_extension_id.empty());

  if (incumbent_extension_id.empty()) {
    *winner_extension_id = suggesting_extension_id;
    *determined_filename = filename;
    *determined_conflict_action = conflict_action;
    return;
  }

  if (suggesting_install_time < incumbent_install_time) {
    *winner_extension_id = incumbent_extension_id;
    warnings->insert(Warning::CreateDownloadFilenameConflictWarning(
        suggesting_extension_id, incumbent_extension_id, filename,
        *determined_filename));
    return;
  }

  *winner_extension_id = suggesting_extension_id;
  warnings->insert(Warning::CreateDownloadFilenameConflictWarning(
      incumbent_extension_id, suggesting_extension_id, *determined_filename,
      filename));
  *determined_filename = filename;
  *determined_conflict_action = conflict_action;
}

bool ExtensionDownloadsEventRouter::DetermineFilename(
    content::BrowserContext* browser_context,
    bool include_incognito,
    const ExtensionId& ext_id,
    int download_id,
    const base::FilePath& const_filename,
    downloads::FilenameConflictAction conflict_action,
    std::string* error) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  return false;
}

void ExtensionDownloadsEventRouter::OnListenerRemoved(
    const EventListenerInfo& details) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
}

// That's all the methods that have to do with filename determination. The rest
// have to do with the other, less special events.

void ExtensionDownloadsEventRouter::OnDownloadCreated(
    DownloadManager* manager,
    DownloadItem* download_item) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
}

void ExtensionDownloadsEventRouter::OnDownloadUpdated(
    DownloadManager* manager,
    DownloadItem* download_item) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
}

void ExtensionDownloadsEventRouter::OnDownloadRemoved(
    DownloadManager* manager,
    DownloadItem* download_item) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  if (!ShouldExport(*download_item))
    return;
  DispatchEvent(events::DOWNLOADS_ON_ERASED, downloads::OnErased::kEventName,
                true, Event::WillDispatchCallback(),
                base::Value(static_cast<int>(download_item->GetId())));
}

void ExtensionDownloadsEventRouter::DispatchEvent(
    events::HistogramValue histogram_value,
    const std::string& event_name,
    bool include_incognito,
    Event::WillDispatchCallback will_dispatch_callback,
    base::Value arg) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  if (!EventRouter::Get(browser_context_))
    return;
  base::Value::List args;
  args.Append(std::move(arg));
  // The downloads system wants to share on-record events with off-record
  // extension renderers even in incognito_split_mode because that's how
  // chrome://downloads works. The "restrict_to_profile" mechanism does not
  // anticipate this, so it does not automatically prevent sharing off-record
  // events with on-record extension renderers.
  // TODO(lazyboy): When |restrict_to_browser_context| is nullptr, this will
  // broadcast events to unrelated profiles, not just incognito. Fix this
  // by introducing "include incognito" option to Event constructor.
  // https://crbug.com/726022.
  content::BrowserContext* restrict_to_browser_context =
      (include_incognito && !browser_context_->IsOffTheRecord()) ?
          nullptr : browser_context_.get();
  auto event =
      std::make_unique<Event>(histogram_value, event_name, std::move(args),
                              restrict_to_browser_context);
  event->will_dispatch_callback = std::move(will_dispatch_callback);
  EventRouter::Get(browser_context_)->BroadcastEvent(std::move(event));
}

void ExtensionDownloadsEventRouter::OnExtensionUnloaded(
    content::BrowserContext* browser_context,
    const Extension* extension,
    UnloadedExtensionReason reason) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  auto iter = ui_disabling_extensions_.find(extension);
  if (iter != ui_disabling_extensions_.end())
    ui_disabling_extensions_.erase(iter);
}

void ExtensionDownloadsEventRouter::CheckForHistoryFilesRemoval() {
  static const int kFileExistenceRateLimitSeconds = 10;
  DownloadManager* manager = notifier_.GetManager();
  if (!manager)
    return;
  base::Time now(base::Time::Now());
  int delta = now.ToTimeT() - last_checked_removal_.ToTimeT();
  if (delta <= kFileExistenceRateLimitSeconds)
    return;
  last_checked_removal_ = now;
  manager->CheckForHistoryFilesRemoval();
}

}  // namespace extensions
