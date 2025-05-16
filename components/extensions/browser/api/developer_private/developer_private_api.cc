// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/developer_private/developer_private_api.h"

#include <stddef.h>

#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "base/barrier_closure.h"
#include "base/check_is_test.h"
#include "base/containers/contains.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/lazy_instance.h"
#include "base/memory/scoped_refptr.h"
#include "base/ranges/algorithm.h"
#include "base/strings/strcat.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/thread_pool.h"
#include "base/uuid.h"
#include "components/policy/core/common/policy_pref_names.h"
#include "components/supervised_user/core/common/buildflags.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/site_instance.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/drop_data.h"
#include "extensions/browser/api/file_handlers/app_file_handler_util.h"
#include "extensions/browser/app_window/app_window.h"
#include "extensions/browser/app_window/app_window_registry.h"
#include "extensions/browser/disable_reason.h"
#include "extensions/browser/error_map.h"
#include "extensions/browser/event_router_factory.h"
#include "extensions/browser/extension_error.h"
#include "extensions/browser/extension_file_task_runner.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_registry_factory.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/extension_util.h"
#include "extensions/browser/file_highlighter.h"
#include "extensions/browser/management_policy.h"
#include "extensions/browser/path_util.h"
#include "extensions/browser/permissions_manager.h"
#include "extensions/browser/process_manager_factory.h"
#include "extensions/browser/ui_util.h"
#include "extensions/browser/updater/extension_downloader_types.h"
#include "extensions/browser/warning_service.h"
#include "extensions/browser/warning_service_factory.h"
#include "extensions/browser/zipfile_installer.h"
#include "extensions/common/extension_features.h"
#include "extensions/common/extension_id.h"
#include "extensions/common/extension_set.h"
#include "extensions/common/feature_switch.h"
#include "extensions/common/install_warning.h"
#include "extensions/common/manifest.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/common/manifest_handlers/background_info.h"
#include "extensions/common/manifest_handlers/options_page_info.h"
#include "extensions/common/manifest_url_handlers.h"
#include "extensions/common/mojom/context_type.mojom.h"
#include "extensions/common/permissions/permissions_data.h"
#include "extensions/common/url_pattern.h"
#include "extensions/common/url_pattern_set.h"
#include "net/base/filename_util.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "storage/browser/blob/shareable_file_reference.h"
#include "storage/browser/file_system/external_mount_points.h"
#include "storage/browser/file_system/file_system_context.h"
#include "storage/browser/file_system/file_system_operation.h"
#include "storage/browser/file_system/file_system_operation_runner.h"
#include "storage/browser/file_system/isolated_context.h"
#include "third_party/blink/public/common/storage_key/storage_key.h"
#include "third_party/re2/src/re2/re2.h"
#include "ui/base/l10n/l10n_util.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace extensions {

namespace developer = api::developer_private;

namespace {
const char kNoSuchExtensionError[] = "No such extension.";
const char kUserCancelledError[] = "User cancelled uninstall";

const char kUnpackedAppsFolder[] = "apps_target";
const char kManifestFile[] = "manifest.json";

base::FilePath* g_drop_path_for_testing = nullptr;

std::string ReadFileToString(const base::FilePath& path) {
  std::string data;
  // This call can fail, but it doesn't matter for our purposes. If it fails,
  // we simply return an empty string for the manifest, and ignore it.
  std::ignore = base::ReadFileToString(path, &data);
  return data;
}

using GetManifestErrorCallback =
    base::OnceCallback<void(const base::FilePath& file_path,
                            const std::string& error,
                            size_t line_number,
                            const std::string& manifest)>;
// Takes in an |error| string and tries to parse it as a manifest error (with
// line number), asynchronously calling |callback| with the results.
void GetManifestError(const std::string& error,
                      const base::FilePath& extension_path,
                      GetManifestErrorCallback callback) {
  size_t line = 0u;
  size_t column = 0u;
  std::string regex = base::StringPrintf("%s  Line: (\\d+), column: (\\d+), .*",
                                         manifest_errors::kManifestParseError);
  // If this was a JSON parse error, we can highlight the exact line with the
  // error. Otherwise, we should still display the manifest (for consistency,
  // reference, and so that if we ever make this really fancy and add an editor,
  // it's ready).
  //
  // This regex call can fail, but if it does, we just don't highlight anything.
  re2::RE2::FullMatch(error, regex, &line, &column);

  // This will read the manifest and call AddFailure with the read manifest
  // contents.
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_BLOCKING},
      base::BindOnce(&ReadFileToString,
                     extension_path.Append(kManifestFilename)),
      base::BindOnce(std::move(callback), extension_path, error, line));
}

// Creates a developer::LoadError from the provided data.
developer::LoadError CreateLoadError(
    const base::FilePath& file_path,
    const std::string& error,
    size_t line_number,
    const std::string& manifest,
    const DeveloperPrivateAPI::UnpackedRetryId& retry_guid) {
  base::FilePath prettified_path = path_util::PrettifyPath(file_path);

  SourceHighlighter highlighter(manifest, line_number);
  developer::LoadError response;
  response.error = error;
  response.path = base::UTF16ToUTF8(prettified_path.LossyDisplayName());
  response.retry_guid = retry_guid;

  response.source.emplace();
  response.source->before_highlight = highlighter.GetBeforeFeature();
  response.source->highlight = highlighter.GetFeature();
  response.source->after_highlight = highlighter.GetAfterFeature();

  return response;
}

}

namespace ChoosePath = api::developer_private::ChoosePath;
namespace PackDirectory = api::developer_private::PackDirectory;
namespace Reload = api::developer_private::Reload;

static base::LazyInstance<BrowserContextKeyedAPIFactory<DeveloperPrivateAPI>>::
    DestructorAtExit g_developer_private_api_factory =
        LAZY_INSTANCE_INITIALIZER;

class DeveloperPrivateAPI::WebContentsTracker
    : public content::WebContentsObserver {
 public:
  WebContentsTracker(base::WeakPtr<DeveloperPrivateAPI> api,
                     content::WebContents* web_contents)
      : content::WebContentsObserver(web_contents), api_(api) {}

  WebContentsTracker(const WebContentsTracker&) = delete;
  WebContentsTracker& operator=(const WebContentsTracker&) = delete;

 private:
  ~WebContentsTracker() override = default;

  void WebContentsDestroyed() override {
    if (api_)
      api_->web_contents_data_.erase(web_contents());
    delete this;
  }

  base::WeakPtr<DeveloperPrivateAPI> api_;
};

DeveloperPrivateAPI::WebContentsData::WebContentsData() = default;
DeveloperPrivateAPI::WebContentsData::~WebContentsData() = default;
DeveloperPrivateAPI::WebContentsData::WebContentsData(WebContentsData&& other) =
    default;

// static
BrowserContextKeyedAPIFactory<DeveloperPrivateAPI>*
DeveloperPrivateAPI::GetFactoryInstance() {
  return g_developer_private_api_factory.Pointer();
}

// static
std::unique_ptr<developer::ProfileInfo> DeveloperPrivateAPI::CreateProfileInfo(
    content::BrowserContext* context) {
  std::unique_ptr<developer::ProfileInfo> info(new developer::ProfileInfo());

  info->is_child_account = false;
  info->is_incognito_available = true;
  info->is_developer_mode_controlled_by_policy = false;
  info->in_developer_mode = false;
  info->can_load_unpacked = true;
  return info;
}

template <>
void BrowserContextKeyedAPIFactory<
    DeveloperPrivateAPI>::DeclareFactoryDependencies() {
  // DependsOn(ExtensionRegistryFactory::GetInstance());
  // DependsOn(ErrorConsoleFactory::GetInstance());
  // DependsOn(ProcessManagerFactory::GetInstance());
  // DependsOn(AppWindowRegistry::Factory::GetInstance());
  // DependsOn(WarningServiceFactory::GetInstance());
  // DependsOn(ExtensionPrefsFactory::GetInstance());
  // DependsOn(ExtensionManagementFactory::GetInstance());
  // DependsOn(CommandService::GetFactoryInstance());
  // DependsOn(EventRouterFactory::GetInstance());
  // DependsOn(ExtensionSystemFactory::GetInstance());
  // DependsOn(PermissionsManager::GetFactory());
  // DependsOn(ToolbarActionsModelFactory::GetInstance());
}

// static
DeveloperPrivateAPI* DeveloperPrivateAPI::Get(
    content::BrowserContext* context) {
  return GetFactoryInstance()->Get(context);
}

DeveloperPrivateAPI::DeveloperPrivateAPI(content::BrowserContext* context)
    : browser_context_(context) {
  RegisterNotifications();
}

DeveloperPrivateEventRouter::DeveloperPrivateEventRouter(content::BrowserContext* context)
    : browser_context_(context), event_router_(EventRouter::Get(browser_context_)) {
  // extension_registry_observation_.Observe(ExtensionRegistry::Get(browser_context_));
  // error_console_observation_.Observe(ErrorConsole::Get(browser_context_));
  // process_manager_observation_.Observe(ProcessManager::Get(browser_context_));
  // app_window_registry_observation_.Observe(AppWindowRegistry::Get(browser_context_));
  // warning_service_observation_.Observe(WarningService::Get(browser_context_));
  // extension_prefs_observation_.Observe(ExtensionPrefs::Get(browser_context_));
  // extension_management_observation_.Observe(
  //     ExtensionManagementFactory::GetForBrowserContext(browser_context_));
  // command_service_observation_.Observe(CommandService::Get(browser_context_));
  // extension_allowlist_observer_.Observe(
  //     ExtensionSystem::Get(browser_context_)->extension_service()->allowlist());
  // permissions_manager_observation_.Observe(PermissionsManager::Get(browser_context_));
  // toolbar_actions_model_observation_.Observe(ToolbarActionsModel::Get(browser_context_));
  // pref_change_registrar_.Init(browser_context_->GetPrefs());
  // // The unretained is safe, since the PrefChangeRegistrar unregisters the
  // // callback on destruction.
  // pref_change_registrar_.Add(
  //     prefs::kExtensionsUIDeveloperMode,
  //     base::BindRepeating(&DeveloperPrivateEventRouter::OnProfilePrefChanged,
  //                         base::Unretained(this)));
}

DeveloperPrivateEventRouter::~DeveloperPrivateEventRouter() {
}

void DeveloperPrivateEventRouter::AddExtensionId(
    const ExtensionId& extension_id) {
  extension_ids_.insert(extension_id);
}

void DeveloperPrivateEventRouter::RemoveExtensionId(
    const ExtensionId& extension_id) {
  extension_ids_.erase(extension_id);
}

void DeveloperPrivateEventRouter::OnExtensionLoaded(
    content::BrowserContext* browser_context,
    const Extension* extension) {
}

DeveloperPrivateAPI::UnpackedRetryId DeveloperPrivateAPI::AddUnpackedPath(
    content::WebContents* web_contents,
    const base::FilePath& path) {
  DCHECK(web_contents);
  last_unpacked_directory_ = path;
  WebContentsData* data = GetOrCreateWebContentsData(web_contents);
  IdToPathMap& paths = data->allowed_unpacked_paths;
  auto existing =
      base::ranges::find(paths, path, &IdToPathMap::value_type::second);
  if (existing != paths.end())
    return existing->first;

  UnpackedRetryId id = base::Uuid::GenerateRandomV4().AsLowercaseString();
  paths[id] = path;
  return id;
}

base::FilePath DeveloperPrivateAPI::GetUnpackedPath(
    content::WebContents* web_contents,
    const UnpackedRetryId& id) const {
  const WebContentsData* data = GetWebContentsData(web_contents);
  if (!data)
    return base::FilePath();
  const IdToPathMap& paths = data->allowed_unpacked_paths;
  auto path_iter = paths.find(id);
  if (path_iter == paths.end())
    return base::FilePath();
  return path_iter->second;
}

void DeveloperPrivateAPI::SetDraggedPath(content::WebContents* web_contents,
                                         const base::FilePath& dragged_path) {
  WebContentsData* data = GetOrCreateWebContentsData(web_contents);
  data->dragged_path = dragged_path;
}

base::FilePath DeveloperPrivateAPI::GetDraggedPath(
    content::WebContents* web_contents) const {
  const WebContentsData* data = GetWebContentsData(web_contents);
  return data ? data->dragged_path : base::FilePath();
}

void DeveloperPrivateAPI::RegisterNotifications() {
  EventRouter::Get(browser_context_)->RegisterObserver(
      this, developer::OnItemStateChanged::kEventName);
  EventRouter::Get(browser_context_)->RegisterObserver(
      this, developer::OnUserSiteSettingsChanged::kEventName);
}

const DeveloperPrivateAPI::WebContentsData*
DeveloperPrivateAPI::GetWebContentsData(
    content::WebContents* web_contents) const {
  auto iter = web_contents_data_.find(web_contents);
  return iter == web_contents_data_.end() ? nullptr : &iter->second;
}

DeveloperPrivateAPI::WebContentsData*
DeveloperPrivateAPI::GetOrCreateWebContentsData(
    content::WebContents* web_contents) {
  auto iter = web_contents_data_.find(web_contents);
  if (iter != web_contents_data_.end())
    return &iter->second;

  // This is the first we've added this WebContents. Track its lifetime so we
  // can clean up the paths when it is destroyed.
  // WebContentsTracker manages its own lifetime.
  new WebContentsTracker(weak_factory_.GetWeakPtr(), web_contents);
  return &web_contents_data_[web_contents];
}

DeveloperPrivateAPI::~DeveloperPrivateAPI() {}

void DeveloperPrivateAPI::Shutdown() {}

void DeveloperPrivateAPI::OnListenerAdded(
    const EventListenerInfo& details) {
  if (!developer_private_event_router_) {
    developer_private_event_router_ =
        std::make_unique<DeveloperPrivateEventRouter>(browser_context_);
  }

  developer_private_event_router_->AddExtensionId(details.extension_id);
}

void DeveloperPrivateAPI::OnListenerRemoved(
    const EventListenerInfo& details) {
  if (!EventRouter::Get(browser_context_)->HasEventListener(
          developer::OnItemStateChanged::kEventName) &&
      !EventRouter::Get(browser_context_)->HasEventListener(
          developer::OnUserSiteSettingsChanged::kEventName)) {
    developer_private_event_router_.reset(nullptr);
  } else {
    developer_private_event_router_->RemoveExtensionId(details.extension_id);
  }
}

namespace api {

DeveloperPrivateAPIFunction::~DeveloperPrivateAPIFunction() {
}

const Extension* DeveloperPrivateAPIFunction::GetExtensionById(
    const ExtensionId& id) {
  return ExtensionRegistry::Get(browser_context())->GetExtensionById(
      id, ExtensionRegistry::EVERYTHING);
}

const Extension* DeveloperPrivateAPIFunction::GetEnabledExtensionById(
    const ExtensionId& id) {
  return ExtensionRegistry::Get(browser_context())->enabled_extensions().
      GetByID(id);
}

DeveloperPrivateAutoUpdateFunction::~DeveloperPrivateAutoUpdateFunction() {}

ExtensionFunction::ResponseAction DeveloperPrivateAutoUpdateFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void DeveloperPrivateAutoUpdateFunction::OnComplete() {
  Respond(NoArguments());
}

DeveloperPrivateGetExtensionsInfoFunction::
DeveloperPrivateGetExtensionsInfoFunction() {
}

DeveloperPrivateGetExtensionsInfoFunction::
~DeveloperPrivateGetExtensionsInfoFunction() {
}

ExtensionFunction::ResponseAction
DeveloperPrivateGetExtensionsInfoFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void DeveloperPrivateGetExtensionsInfoFunction::OnInfosGenerated(
    std::vector<api::developer_private::ExtensionInfo> list) {
  Respond(ArgumentList(developer::GetExtensionsInfo::Results::Create(list)));
}

DeveloperPrivateGetExtensionInfoFunction::
DeveloperPrivateGetExtensionInfoFunction() {
}

DeveloperPrivateGetExtensionInfoFunction::
~DeveloperPrivateGetExtensionInfoFunction() {
}

ExtensionFunction::ResponseAction
DeveloperPrivateGetExtensionInfoFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void DeveloperPrivateGetExtensionInfoFunction::OnInfosGenerated(
    std::vector<api::developer_private::ExtensionInfo> list) {
  DCHECK_LE(1u, list.size());
  Respond(list.empty() ? Error(kNoSuchExtensionError)
                       : WithArguments(list[0].ToValue()));
}

DeveloperPrivateGetExtensionSizeFunction::
    DeveloperPrivateGetExtensionSizeFunction() {}

DeveloperPrivateGetExtensionSizeFunction::
    ~DeveloperPrivateGetExtensionSizeFunction() {}

ExtensionFunction::ResponseAction
DeveloperPrivateGetExtensionSizeFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void DeveloperPrivateGetExtensionSizeFunction::OnSizeCalculated(
    const std::u16string& size) {
  Respond(WithArguments(size));
}

DeveloperPrivateGetProfileConfigurationFunction::
~DeveloperPrivateGetProfileConfigurationFunction() {
}

ExtensionFunction::ResponseAction
DeveloperPrivateGetProfileConfigurationFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateUpdateProfileConfigurationFunction::
~DeveloperPrivateUpdateProfileConfigurationFunction() {
}

ExtensionFunction::ResponseAction
DeveloperPrivateUpdateProfileConfigurationFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateUpdateExtensionConfigurationFunction::
~DeveloperPrivateUpdateExtensionConfigurationFunction() {}

ExtensionFunction::ResponseAction
DeveloperPrivateUpdateExtensionConfigurationFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateReloadFunction::DeveloperPrivateReloadFunction() = default;
DeveloperPrivateReloadFunction::~DeveloperPrivateReloadFunction() = default;

ExtensionFunction::ResponseAction DeveloperPrivateReloadFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void DeveloperPrivateReloadFunction::OnExtensionLoaded(
    content::BrowserContext* browser_context,
    const Extension* extension) {
  if (extension->path() == reloading_extension_path_) {
    // Reload succeeded!
    Respond(NoArguments());
    ClearObservers();
  }
}

void DeveloperPrivateReloadFunction::OnShutdown(ExtensionRegistry* registry) {
  Respond(Error("Shutting down."));
  ClearObservers();
}

// TODO(mshin): Enable the below code after migrating LoadErrorReporter
// void DeveloperPrivateReloadFunction::OnLoadFailure(
//     content::BrowserContext* browser_context,
//     const base::FilePath& file_path,
//     const std::string& error) {
//   if (file_path == reloading_extension_path_) {
//     // Reload failed - create an error to pass back to the extension.
//     GetManifestError(
//         error, file_path,
//         base::BindOnce(&DeveloperPrivateReloadFunction::OnGotManifestError,
//                        this));  // Creates a reference.
//     ClearObservers();
//   }
// }

void DeveloperPrivateReloadFunction::OnGotManifestError(
    const base::FilePath& file_path,
    const std::string& error,
    size_t line_number,
    const std::string& manifest) {
  DeveloperPrivateAPI::UnpackedRetryId retry_guid =
      DeveloperPrivateAPI::Get(browser_context())
          ->AddUnpackedPath(GetSenderWebContents(), reloading_extension_path_);
  // Respond to the caller with the load error, which allows the caller to retry
  // reloading through developerPrivate.loadUnpacked().
  // TODO(devlin): This is weird. Really, we should allow retrying through this
  // function instead of through loadUnpacked(), but
  // ExtensionService::ReloadExtension doesn't behave well with an extension
  // that failed to reload, and untangling that mess is quite significant.
  // See https://crbug.com/792277.
  Respond(WithArguments(
      CreateLoadError(file_path, error, line_number, manifest, retry_guid)
          .ToValue()));
}

void DeveloperPrivateReloadFunction::ClearObservers() {
  registry_observation_.Reset();
  // TODO(mshin): Enable the below code after migrating LoadErrorReporter
  // error_reporter_observation_.Reset();

  Release();  // Balanced in Run().
}

DeveloperPrivateLoadUnpackedFunction::DeveloperPrivateLoadUnpackedFunction() {}

ExtensionFunction::ResponseAction DeveloperPrivateLoadUnpackedFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// TODO(mshin): Enable the below code after migrating EntryPickerClient
// void DeveloperPrivateLoadUnpackedFunction::FileSelected(
//     const base::FilePath& path) {
//   scoped_refptr<UnpackedInstaller> installer(
//       UnpackedInstaller::Create(GetExtensionService(browser_context())));
//   installer->set_be_noisy_on_failure(!fail_quietly_);
//   installer->set_completion_callback(base::BindOnce(
//       &DeveloperPrivateLoadUnpackedFunction::OnLoadComplete, this));
//   installer->Load(path);

//   retry_guid_ = DeveloperPrivateAPI::Get(browser_context())
//                   ->AddUnpackedPath(GetSenderWebContents(), path);

//   Release();  // Balanced in Run().
// }

// void DeveloperPrivateLoadUnpackedFunction::FileSelectionCanceled() {
//   // This isn't really an error, but we should keep it like this for
//   // backward compatability.
//   Respond(Error(kFileSelectionCanceled));
//   Release();  // Balanced in Run().
// }

void DeveloperPrivateLoadUnpackedFunction::OnLoadComplete(
    const Extension* extension,
    const base::FilePath& file_path,
    const std::string& error) {
  if (extension || !populate_error_) {
    Respond(extension ? NoArguments() : Error(error));
    return;
  }

  GetManifestError(
      error, file_path,
      base::BindOnce(&DeveloperPrivateLoadUnpackedFunction::OnGotManifestError,
                     this));
}

void DeveloperPrivateLoadUnpackedFunction::OnGotManifestError(
    const base::FilePath& file_path,
    const std::string& error,
    size_t line_number,
    const std::string& manifest) {
  DCHECK(!retry_guid_.empty());
  Respond(WithArguments(
      CreateLoadError(file_path, error, line_number, manifest, retry_guid_)
          .ToValue()));
}

DeveloperPrivateInstallDroppedFileFunction::
    DeveloperPrivateInstallDroppedFileFunction() = default;
DeveloperPrivateInstallDroppedFileFunction::
    ~DeveloperPrivateInstallDroppedFileFunction() = default;

ExtensionFunction::ResponseAction
DeveloperPrivateInstallDroppedFileFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateNotifyDragInstallInProgressFunction::
    DeveloperPrivateNotifyDragInstallInProgressFunction() = default;
DeveloperPrivateNotifyDragInstallInProgressFunction::
    ~DeveloperPrivateNotifyDragInstallInProgressFunction() = default;

ExtensionFunction::ResponseAction
DeveloperPrivateNotifyDragInstallInProgressFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// static
void DeveloperPrivateNotifyDragInstallInProgressFunction::SetDropPathForTesting(
    base::FilePath* file_path) {
  g_drop_path_for_testing = file_path;
}

bool DeveloperPrivateChooseEntryFunction::ShowPicker(
    ui::SelectFileDialog::Type picker_type,
    const std::u16string& select_title,
    const ui::SelectFileDialog::FileTypeInfo& info,
    int file_type_index) {
  content::WebContents* web_contents = GetSenderWebContents();
  if (!web_contents)
    return false;

  // The entry picker will hold a reference to this function instance,
  // and subsequent sending of the function response) until the user has
  // selected a file or cancelled the picker. At that point, the picker will
  // delete itself.
  // TODO(mshin): Enable the below code after migrating EntryPicker
  // new EntryPicker(
  //     this, web_contents, picker_type,
  //     DeveloperPrivateAPI::Get(browser_context())->last_unpacked_directory(),
  //     select_title, info, file_type_index);
  return true;
}

DeveloperPrivateChooseEntryFunction::~DeveloperPrivateChooseEntryFunction() {}

// TODO(mshin): Enable the below code after migrating ExtensionPackJob
// void DeveloperPrivatePackDirectoryFunction::OnPackSuccess(
//     const base::FilePath& crx_file,
//     const base::FilePath& pem_file) {
//   developer::PackDirectoryResponse response;
//   response.message = base::UTF16ToUTF8(
//       PackExtensionJob::StandardSuccessMessage(crx_file, pem_file));
//   response.status = developer::PackStatus::kSuccess;
//   Respond(WithArguments(response.ToValue()));
//   pack_job_.reset();
//   Release();  // Balanced in Run().
// }

// void DeveloperPrivatePackDirectoryFunction::OnPackFailure(
//     const std::string& error,
//     ExtensionCreator::ErrorType error_type) {
//   developer::PackDirectoryResponse response;
//   response.message = error;
//   if (error_type == ExtensionCreator::kCRXExists) {
//     response.item_path = item_path_str_;
//     response.pem_path = key_path_str_;
//     response.override_flags = ExtensionCreator::kOverwriteCRX;
//     response.status = developer::PackStatus::kWarning;
//   } else {
//     response.status = developer::PackStatus::kError;
//   }
//   Respond(WithArguments(response.ToValue()));
//   pack_job_.reset();
//   Release();  // Balanced in Run().
// }

ExtensionFunction::ResponseAction DeveloperPrivatePackDirectoryFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivatePackDirectoryFunction::DeveloperPrivatePackDirectoryFunction() {
}

DeveloperPrivatePackDirectoryFunction::
~DeveloperPrivatePackDirectoryFunction() {}

DeveloperPrivateLoadUnpackedFunction::~DeveloperPrivateLoadUnpackedFunction() {}

ExtensionFunction::ResponseAction DeveloperPrivateLoadDirectoryFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction
DeveloperPrivateLoadDirectoryFunction::LoadByFileSystemAPI(
    const storage::FileSystemURL& directory_url) {
  std::string directory_url_str = directory_url.ToGURL().spec();

  size_t pos = 0;
  // Parse the project directory name from the project url. The project url is
  // expected to have project name as the suffix.
  if ((pos = directory_url_str.rfind("/")) == std::string::npos) {
    return RespondNow(Error("Invalid Directory entry."));
  }

  std::string project_name;
  project_name = directory_url_str.substr(pos + 1);
  project_base_url_ = directory_url_str.substr(0, pos + 1);

  base::FilePath project_path(browser_context()->GetPath());
  project_path = project_path.AppendASCII(kUnpackedAppsFolder);
  project_path = project_path.Append(
      base::FilePath::FromUTF8Unsafe(project_name));

  project_base_path_ = project_path;

  base::ThreadPool::PostTask(
      FROM_HERE,
      {base::MayBlock(), base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN},
      base::BindOnce(
          &DeveloperPrivateLoadDirectoryFunction::ClearExistingDirectoryContent,
          this, project_base_path_));
  return RespondLater();
}

void DeveloperPrivateLoadDirectoryFunction::Load() {
  // TODO(mshin): Enable the below code after migrating UnpackedInstaller
  // ExtensionService* service = GetExtensionService(browser_context());
  // UnpackedInstaller::Create(service)->Load(project_base_path_);

  // TODO(grv) : The unpacked installer should fire an event when complete
  // and return the extension_id.
  Respond(WithArguments("-1"));
}

void DeveloperPrivateLoadDirectoryFunction::ClearExistingDirectoryContent(
    const base::FilePath& project_path) {
  // Clear the project directory before copying new files.
  base::DeletePathRecursively(project_path);

  pending_copy_operations_count_ = 1;

  content::GetIOThreadTaskRunner({})->PostTask(
      FROM_HERE,
      base::BindOnce(
          &DeveloperPrivateLoadDirectoryFunction::ReadDirectoryByFileSystemAPI,
          this, project_path, project_path.BaseName()));
}

void DeveloperPrivateLoadDirectoryFunction::ReadDirectoryByFileSystemAPI(
    const base::FilePath& project_path,
    const base::FilePath& destination_path) {
  GURL project_url = GURL(project_base_url_ + destination_path.AsUTF8Unsafe());
  storage::FileSystemURL url =
      context_->CrackURLInFirstPartyContext(project_url);

  context_->operation_runner()->ReadDirectory(
      url, base::BindRepeating(&DeveloperPrivateLoadDirectoryFunction::
                                   ReadDirectoryByFileSystemAPICb,
                               this, project_path, destination_path));
}

void DeveloperPrivateLoadDirectoryFunction::ReadDirectoryByFileSystemAPICb(
    const base::FilePath& project_path,
    const base::FilePath& destination_path,
    base::File::Error status,
    storage::FileSystemOperation::FileEntryList file_list,
    bool has_more) {
  if (status != base::File::FILE_OK) {
    DLOG(ERROR) << "Error in copying files from sync filesystem.";
    return;
  }

  // We add 1 to the pending copy operations for both files and directories. We
  // release the directory copy operation once all the files under the directory
  // are added for copying. We do that to ensure that pendingCopyOperationsCount
  // does not become zero before all copy operations are finished.
  // In case the directory happens to be executing the last copy operation it
  // will call Respond to send the response to the API. The pending copy
  // operations of files are released by the CopyFile function.
  pending_copy_operations_count_ += file_list.size();

  for (auto& file : file_list) {
    if (file.type == filesystem::mojom::FsFileType::DIRECTORY) {
      ReadDirectoryByFileSystemAPI(project_path.Append(file.name),
                                   destination_path.Append(file.name));
      continue;
    }

    GURL project_url = GURL(project_base_url_ +
                            destination_path.Append(file.name).AsUTF8Unsafe());
    storage::FileSystemURL url =
        context_->CrackURLInFirstPartyContext(project_url);

    base::FilePath target_path = project_path;
    target_path = target_path.Append(file.name);

    context_->operation_runner()->CreateSnapshotFile(
        url, base::BindOnce(
                 &DeveloperPrivateLoadDirectoryFunction::SnapshotFileCallback,
                 this, target_path));
  }

  if (!has_more) {
    // Directory copy operation released here.
    pending_copy_operations_count_--;

    if (!pending_copy_operations_count_) {
      ExtensionFunction::ResponseValue response =
          success_ ? NoArguments() : Error(error_);
      content::GetUIThreadTaskRunner({})->PostTask(
          FROM_HERE,
          base::BindOnce(&DeveloperPrivateLoadDirectoryFunction::Respond, this,
                         std::move(response)));
    }
  }
}

void DeveloperPrivateLoadDirectoryFunction::SnapshotFileCallback(
    const base::FilePath& target_path,
    base::File::Error result,
    const base::File::Info& file_info,
    const base::FilePath& src_path,
    scoped_refptr<storage::ShareableFileReference> file_ref) {
  if (result != base::File::FILE_OK) {
    error_ = "Error in copying files from sync filesystem.";
    success_ = false;
    return;
  }

  base::ThreadPool::PostTask(
      FROM_HERE,
      {base::MayBlock(), base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN},
      base::BindOnce(&DeveloperPrivateLoadDirectoryFunction::CopyFile, this,
                     src_path, target_path));
}

void DeveloperPrivateLoadDirectoryFunction::CopyFile(
    const base::FilePath& src_path,
    const base::FilePath& target_path) {
  if (!base::CreateDirectory(target_path.DirName())) {
    error_ = "Error in copying files from sync filesystem.";
    success_ = false;
  }

  if (success_)
    base::CopyFile(src_path, target_path);

  CHECK(pending_copy_operations_count_ > 0);
  pending_copy_operations_count_--;

  if (!pending_copy_operations_count_) {
    content::GetUIThreadTaskRunner({})->PostTask(
        FROM_HERE,
        base::BindOnce(&DeveloperPrivateLoadDirectoryFunction::Load, this));
  }
}

DeveloperPrivateLoadDirectoryFunction::DeveloperPrivateLoadDirectoryFunction()
    : pending_copy_operations_count_(0), success_(true) {}

DeveloperPrivateLoadDirectoryFunction::~DeveloperPrivateLoadDirectoryFunction()
    {}

ExtensionFunction::ResponseAction DeveloperPrivateChoosePathFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// TODO(mshin): Enable the below code after migrating EntryPickerClient
// void DeveloperPrivateChoosePathFunction::FileSelected(
//     const base::FilePath& path) {
//   Respond(WithArguments(path.LossyDisplayName()));
//   Release();
// }

// void DeveloperPrivateChoosePathFunction::FileSelectionCanceled() {
//   // This isn't really an error, but we should keep it like this for
//   // backward compatability.
//   Respond(Error(kFileSelectionCanceled));
//   Release();
// }

DeveloperPrivateChoosePathFunction::~DeveloperPrivateChoosePathFunction() {}

ExtensionFunction::ResponseAction
DeveloperPrivateIsProfileManagedFunction::Run() {
  return RespondNow(Error("Not Implement"));  
}

DeveloperPrivateIsProfileManagedFunction::
    ~DeveloperPrivateIsProfileManagedFunction() {
}

DeveloperPrivateRequestFileSourceFunction::
    DeveloperPrivateRequestFileSourceFunction() {}

DeveloperPrivateRequestFileSourceFunction::
    ~DeveloperPrivateRequestFileSourceFunction() {}

ExtensionFunction::ResponseAction
DeveloperPrivateRequestFileSourceFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void DeveloperPrivateRequestFileSourceFunction::Finish(
    const std::string& file_contents) {
  const developer::RequestFileSourceProperties& properties =
      params_->properties;
  const Extension* extension = GetExtensionById(properties.extension_id);
  if (!extension) {
    Respond(Error(kNoSuchExtensionError));
    return;
  }

  developer::RequestFileSourceResponse response;
  base::FilePath path_suffix =
      base::FilePath::FromUTF8Unsafe(properties.path_suffix);
  base::FilePath path = extension->path().Append(path_suffix);
  response.title = base::StringPrintf("%s: %s",
                                      extension->name().c_str(),
                                      path.BaseName().AsUTF8Unsafe().c_str());
  response.message = properties.message;

  std::unique_ptr<FileHighlighter> highlighter;
  if (properties.path_suffix == kManifestFile) {
    highlighter = std::make_unique<ManifestHighlighter>(
        file_contents, *properties.manifest_key,
        properties.manifest_specific ? *properties.manifest_specific
                                     : std::string());
  } else {
    highlighter = std::make_unique<SourceHighlighter>(
        file_contents, properties.line_number ? *properties.line_number : 0);
  }

  response.before_highlight = highlighter->GetBeforeFeature();
  response.highlight = highlighter->GetFeature();
  response.after_highlight = highlighter->GetAfterFeature();

  Respond(WithArguments(response.ToValue()));
}

DeveloperPrivateOpenDevToolsFunction::DeveloperPrivateOpenDevToolsFunction() {}
DeveloperPrivateOpenDevToolsFunction::~DeveloperPrivateOpenDevToolsFunction() {}

ExtensionFunction::ResponseAction
DeveloperPrivateOpenDevToolsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateDeleteExtensionErrorsFunction::
~DeveloperPrivateDeleteExtensionErrorsFunction() {}

ExtensionFunction::ResponseAction
DeveloperPrivateDeleteExtensionErrorsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateRepairExtensionFunction::
~DeveloperPrivateRepairExtensionFunction() {}

ExtensionFunction::ResponseAction
DeveloperPrivateRepairExtensionFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// TODO(mshin): Enable the below code after migrating WebStore
// void DeveloperPrivateRepairExtensionFunction::OnReinstallComplete(
//     bool success,
//     const std::string& error,
//     webstore_install::Result result) {
//   Respond(success ? NoArguments() : Error(error));
// }

DeveloperPrivateShowOptionsFunction::~DeveloperPrivateShowOptionsFunction() {}

ExtensionFunction::ResponseAction DeveloperPrivateShowOptionsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateShowPathFunction::~DeveloperPrivateShowPathFunction() {}

ExtensionFunction::ResponseAction DeveloperPrivateShowPathFunction::Run() {
  return RespondNow(Error("Not Implement"));  
}

DeveloperPrivateSetShortcutHandlingSuspendedFunction::
~DeveloperPrivateSetShortcutHandlingSuspendedFunction() {}

ExtensionFunction::ResponseAction
DeveloperPrivateSetShortcutHandlingSuspendedFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateUpdateExtensionCommandFunction::
~DeveloperPrivateUpdateExtensionCommandFunction() {}

ExtensionFunction::ResponseAction
DeveloperPrivateUpdateExtensionCommandFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateAddHostPermissionFunction::
    DeveloperPrivateAddHostPermissionFunction() = default;
DeveloperPrivateAddHostPermissionFunction::
    ~DeveloperPrivateAddHostPermissionFunction() = default;

ExtensionFunction::ResponseAction
DeveloperPrivateAddHostPermissionFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void DeveloperPrivateAddHostPermissionFunction::OnRuntimePermissionsGranted() {
  Respond(NoArguments());
}

DeveloperPrivateRemoveHostPermissionFunction::
    DeveloperPrivateRemoveHostPermissionFunction() = default;
DeveloperPrivateRemoveHostPermissionFunction::
    ~DeveloperPrivateRemoveHostPermissionFunction() = default;

ExtensionFunction::ResponseAction
DeveloperPrivateRemoveHostPermissionFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void DeveloperPrivateRemoveHostPermissionFunction::
    OnRuntimePermissionsRevoked() {
  Respond(NoArguments());
}

DeveloperPrivateGetUserSiteSettingsFunction::
    DeveloperPrivateGetUserSiteSettingsFunction() = default;
DeveloperPrivateGetUserSiteSettingsFunction::
    ~DeveloperPrivateGetUserSiteSettingsFunction() = default;

ExtensionFunction::ResponseAction
DeveloperPrivateGetUserSiteSettingsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateAddUserSpecifiedSitesFunction::
    DeveloperPrivateAddUserSpecifiedSitesFunction() = default;
DeveloperPrivateAddUserSpecifiedSitesFunction::
    ~DeveloperPrivateAddUserSpecifiedSitesFunction() = default;

ExtensionFunction::ResponseAction
DeveloperPrivateAddUserSpecifiedSitesFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateRemoveUserSpecifiedSitesFunction::
    DeveloperPrivateRemoveUserSpecifiedSitesFunction() = default;
DeveloperPrivateRemoveUserSpecifiedSitesFunction::
    ~DeveloperPrivateRemoveUserSpecifiedSitesFunction() = default;

ExtensionFunction::ResponseAction
DeveloperPrivateRemoveUserSpecifiedSitesFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateGetUserAndExtensionSitesByEtldFunction::
    DeveloperPrivateGetUserAndExtensionSitesByEtldFunction() = default;
DeveloperPrivateGetUserAndExtensionSitesByEtldFunction::
    ~DeveloperPrivateGetUserAndExtensionSitesByEtldFunction() = default;

ExtensionFunction::ResponseAction
DeveloperPrivateGetUserAndExtensionSitesByEtldFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateGetMatchingExtensionsForSiteFunction::
    DeveloperPrivateGetMatchingExtensionsForSiteFunction() = default;
DeveloperPrivateGetMatchingExtensionsForSiteFunction::
    ~DeveloperPrivateGetMatchingExtensionsForSiteFunction() = default;

ExtensionFunction::ResponseAction
DeveloperPrivateGetMatchingExtensionsForSiteFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

DeveloperPrivateUpdateSiteAccessFunction::
    DeveloperPrivateUpdateSiteAccessFunction() = default;
DeveloperPrivateUpdateSiteAccessFunction::
    ~DeveloperPrivateUpdateSiteAccessFunction() = default;

ExtensionFunction::ResponseAction
DeveloperPrivateUpdateSiteAccessFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void DeveloperPrivateUpdateSiteAccessFunction::OnSiteSettingsUpdated() {
  Respond(NoArguments());
}

DeveloperPrivateRemoveMultipleExtensionsFunction::
    DeveloperPrivateRemoveMultipleExtensionsFunction() = default;
DeveloperPrivateRemoveMultipleExtensionsFunction::
    ~DeveloperPrivateRemoveMultipleExtensionsFunction() = default;

ExtensionFunction::ResponseAction
DeveloperPrivateRemoveMultipleExtensionsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void DeveloperPrivateRemoveMultipleExtensionsFunction::OnDialogCancelled() {
  // Let the consumer end know that the Close button was clicked.
  Respond(Error(kUserCancelledError));
}

void DeveloperPrivateRemoveMultipleExtensionsFunction::OnDialogAccepted() {
  for (const auto& extension_id : extension_ids_) {
    if (!browser_context()) {
      return;
    }
    const Extension* current_extension =
        ExtensionRegistry::Get(browser_context_)->GetExtensionById(
            extension_id, ExtensionRegistry::EVERYTHING);
    // Extensions can be uninstalled externally while the dialog is open. Only
    // uninstall extensions that are still existent.
    if (!current_extension) {
      continue;
    }
    // If an extension fails to be uninstalled, it will not pause the
    // uninstall of the other extensions on the list.
    // TODO(mshin): Enable the below code after migrating ExtensionService
    // ExtensionSystem::Get(browser_context_)->extension_service()->UninstallExtension(
    //     extension_id, UNINSTALL_REASON_USER_INITIATED, nullptr);
  }
  Respond(NoArguments());
}

DeveloperPrivateDismissSafetyHubExtensionsMenuNotificationFunction::
    DeveloperPrivateDismissSafetyHubExtensionsMenuNotificationFunction() =
        default;
DeveloperPrivateDismissSafetyHubExtensionsMenuNotificationFunction::
    ~DeveloperPrivateDismissSafetyHubExtensionsMenuNotificationFunction() =
        default;

ExtensionFunction::ResponseAction
DeveloperPrivateDismissSafetyHubExtensionsMenuNotificationFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace api

}  // namespace extensions
