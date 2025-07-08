// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/component_loader.h"

#include <optional>
#include <string>

#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/functional/callback_helpers.h"
#include "base/json/json_string_value_serializer.h"
#include "base/metrics/histogram_macros.h"
#include "base/path_service.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"
#include "base/values.h"
#include "build/build_config.h"
#include "components/extensions/browser/extension_service.h"
#include "components/extensions/common/extension_constants.h"
#include "components/grit/components_resources.h"
#include "components/crx_file/id_util.h"
#include "components/nacl/common/buildflags.h"
#include "components/version_info/version_info.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/content_switches.h"
#include "extensions/browser/extension_file_task_runner.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/pref_names.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_features.h"
#include "extensions/common/extension_id.h"
#include "extensions/common/extension_l10n_util.h"
#include "extensions/common/file_util.h"
#include "extensions/common/manifest_constants.h"
#include "pdf/buildflags.h"
#include "printing/buildflags/buildflags.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"

#if BUILDFLAG(ENABLE_PDF)
#include "chrome/browser/pdf/pdf_extension_util.h"
#endif

using content::BrowserContext;
using extensions::Extension;
using extensions::ExtensionId;
using extensions::ExtensionSystem;

namespace components_extensions {

namespace file_util = extensions::file_util;
namespace manifest_keys = extensions::manifest_keys;
namespace mojom = extensions::mojom;

namespace {

bool g_enable_background_extensions_during_testing = false;

ExtensionId GenerateId(const base::Value::Dict& manifest,
                       const base::FilePath& path) {
  std::string id_input;
  const std::string* raw_key = manifest.FindString(manifest_keys::kPublicKey);
  CHECK(raw_key != nullptr);
  CHECK(Extension::ParsePEMKeyBytes(*raw_key, &id_input));
  ExtensionId id = crx_file::id_util::GenerateId(id_input);
  return id;
}

}  // namespace

ComponentLoader::ComponentExtensionInfo::ComponentExtensionInfo(
    base::Value::Dict manifest_param,
    const base::FilePath& directory)
    : manifest(std::move(manifest_param)), root_directory(directory) {
  if (!root_directory.IsAbsolute()) {
#if BUILDFLAG(IS_ANDROID)
    CHECK(base::PathService::Get(base::DIR_ANDROID_APP_DATA, &root_directory));
#else
    CHECK(base::PathService::Get(base::DIR_ASSETS, &root_directory));
#endif
    root_directory = root_directory.Append(directory);
  }
  extension_id = GenerateId(manifest, root_directory);
}

ComponentLoader::ComponentExtensionInfo::ComponentExtensionInfo(
    ComponentExtensionInfo&& other)
    : manifest(std::move(other.manifest)),
      root_directory(std::move(other.root_directory)),
      extension_id(std::move(other.extension_id)) {}

ComponentLoader::ComponentExtensionInfo&
ComponentLoader::ComponentExtensionInfo::operator=(
    ComponentExtensionInfo&& other) {
  manifest = std::move(other.manifest);
  root_directory = std::move(other.root_directory);
  extension_id = std::move(other.extension_id);
  return *this;
}

ComponentLoader::ComponentExtensionInfo::~ComponentExtensionInfo() = default;

ComponentLoader::ComponentLoader(ExtensionSystem* extension_system,
                                 content::BrowserContext* browser_context)
    : browser_context_(browser_context),
      extension_system_(extension_system),
      ignore_allowlist_for_testing_(false) {}

ComponentLoader::~ComponentLoader() = default;

void ComponentLoader::LoadAll() {
  TRACE_EVENT0("browser,startup", "ComponentLoader::LoadAll");
  // TODO(mshin): Support the mutiple profile
  bool is_user_profile = true;
  const base::TimeTicks load_start_time = base::TimeTicks::Now();

  for (const auto& component_extension : component_extensions_) {
    Load(component_extension);
  }

  const base::TimeDelta load_all_component_time =
      base::TimeTicks::Now() - load_start_time;
  UMA_HISTOGRAM_TIMES("Extensions.LoadAllComponentTime",
                      load_all_component_time);
  if (is_user_profile) {
    UMA_HISTOGRAM_TIMES("Extensions.LoadAllComponentTime.User",
                        load_all_component_time);
  } else {
    UMA_HISTOGRAM_TIMES("Extensions.LoadAllComponentTime.NonUser",
                        load_all_component_time);
  }
}

std::optional<base::Value::Dict> ComponentLoader::ParseManifest(
    base::StringPiece manifest_contents) const {
  JSONStringValueDeserializer deserializer(manifest_contents);
  std::unique_ptr<base::Value> manifest =
      deserializer.Deserialize(nullptr, nullptr);

  if (!manifest.get() || !manifest->is_dict()) {
    LOG(ERROR) << "Failed to parse extension manifest.";
    return std::nullopt;
  }

  return std::move(*manifest).TakeDict();
}

ExtensionId ComponentLoader::Add(int manifest_resource_id,
                                 const base::FilePath& root_directory) {
  // TODO(mshin): Enable the below code after migrating allow_list
  // if (!ignore_allowlist_for_testing_ &&
  //     !IsComponentExtensionAllowlisted(manifest_resource_id)) {
  //   return std::string();
  // }

  base::StringPiece manifest_contents =
      ui::ResourceBundle::GetSharedInstance().GetRawDataResource(
          manifest_resource_id);
  return Add(manifest_contents, root_directory, true);
}

ExtensionId ComponentLoader::Add(base::Value::Dict manifest,
                                 const base::FilePath& root_directory) {
  return Add(std::move(manifest), root_directory, false);
}

ExtensionId ComponentLoader::Add(const base::StringPiece& manifest_contents,
                                 const base::FilePath& root_directory) {
  return Add(manifest_contents, root_directory, false);
}

ExtensionId ComponentLoader::Add(const base::StringPiece& manifest_contents,
                                 const base::FilePath& root_directory,
                                 bool skip_allowlist) {
  // The Value is kept for the lifetime of the ComponentLoader. This is
  // required in case LoadAll() is called again.
  std::optional<base::Value::Dict> manifest = ParseManifest(manifest_contents);
  if (manifest) {
    return Add(std::move(*manifest), root_directory, skip_allowlist);
  }
  return std::string();
}

ExtensionId ComponentLoader::Add(base::Value::Dict parsed_manifest,
                                 const base::FilePath& root_directory,
                                 bool skip_allowlist) {
  ComponentExtensionInfo info(std::move(parsed_manifest), root_directory);
  // TODO(mshin): Enable the below code after migrating allow_list
  // if (!ignore_allowlist_for_testing_ && !skip_allowlist &&
  //     !IsComponentExtensionAllowlisted(info.extension_id)) {
  //   return std::string();
  // }

  component_extensions_.push_back(std::move(info));
  ComponentExtensionInfo& added_info = component_extensions_.back();
  if (extension_system_->is_ready()) {
    Load(added_info);
  }
  return added_info.extension_id;
}

ExtensionId ComponentLoader::AddOrReplace(const base::FilePath& path) {
  base::FilePath absolute_path = base::MakeAbsoluteFilePath(path);
  std::string error;
  std::optional<base::Value::Dict> manifest(
      file_util::LoadManifest(absolute_path, &error));
  if (!manifest) {
    LOG(ERROR) << "Could not load extension from '" << absolute_path.value()
               << "'. " << error;
    return std::string();
  }
  Remove(GenerateId(*manifest, absolute_path));

  // We don't check component extensions loaded by path because this is only
  // used by developers for testing.
  return Add(std::move(*manifest), absolute_path, true);
}

void ComponentLoader::Reload(const ExtensionId& extension_id) {
  for (const auto& component_extension : component_extensions_) {
    if (component_extension.extension_id == extension_id) {
      Load(component_extension);
      break;
    }
  }
}

void ComponentLoader::Load(const ComponentExtensionInfo& info) {
  std::string error;
  scoped_refptr<const Extension> extension(CreateExtension(info, &error));
  if (!extension.get()) {
    LOG(ERROR) << error;
    return;
  }

  CHECK_EQ(info.extension_id, extension->id()) << extension->name();
  extension_system_->extension_service()->AddComponentExtension(
      extension.get());
}

void ComponentLoader::Remove(const base::FilePath& root_directory) {
  // Find the ComponentExtensionInfo for the extension.
  for (const auto& component_extension : component_extensions_) {
    if (component_extension.root_directory == root_directory) {
      Remove(GenerateId(component_extension.manifest, root_directory));
      break;
    }
  }
}

void ComponentLoader::Remove(const ExtensionId& id) {
  for (auto it = component_extensions_.begin();
       it != component_extensions_.end(); ++it) {
    if (it->extension_id == id) {
      UnloadComponent(&(*it));
      component_extensions_.erase(it);
      break;
    }
  }
}

bool ComponentLoader::Exists(const ExtensionId& id) const {
  for (const auto& component_extension : component_extensions_) {
    if (component_extension.extension_id == id) {
      return true;
    }
  }
  return false;
}

std::vector<ExtensionId> ComponentLoader::GetRegisteredComponentExtensionsIds()
    const {
  std::vector<ExtensionId> result;
  for (const auto& el : component_extensions_) {
    result.push_back(el.extension_id);
  }
  return result;
}

void ComponentLoader::AddNetworkSpeechSynthesisExtension() {
  // TODO(mshin): Enable the below code after migrating speech synthesis"
  // Add(IDR_NETWORK_SPEECH_SYNTHESIS_MANIFEST,
  //     base::FilePath(FILE_PATH_LITERAL("network_speech_synthesis")));
}

void ComponentLoader::AddWithNameAndDescription(
    int manifest_resource_id,
    const base::FilePath& root_directory,
    const std::string& name_string,
    const std::string& description_string) {
  // TODO(mshin): Enable the below code after migrating allow_list
  // if (!ignore_allowlist_for_testing_ &&
  //     !IsComponentExtensionAllowlisted(manifest_resource_id)) {
  //   return;
  // }

  base::StringPiece manifest_contents =
      ui::ResourceBundle::GetSharedInstance().GetRawDataResource(
          manifest_resource_id);

  // The Value is kept for the lifetime of the ComponentLoader. This is
  // required in case LoadAll() is called again.
  std::optional<base::Value::Dict> manifest = ParseManifest(manifest_contents);

  if (manifest) {
    manifest->Set(manifest_keys::kName, name_string);
    manifest->Set(manifest_keys::kDescription, description_string);
    Add(std::move(*manifest), root_directory, true);
  }
}

void ComponentLoader::AddWebStoreApp() {
  // TODO(mshin): Enable the below code after migrating WebStore
  // AddWithNameAndDescription(
  //     IDR_WEBSTORE_MANIFEST, base::FilePath(FILE_PATH_LITERAL("web_store")),
  //     l10n_util::GetStringUTF8(IDS_WEBSTORE_NAME_STORE),
  //     l10n_util::GetStringUTF8(IDS_WEBSTORE_APP_DESCRIPTION));
}

scoped_refptr<const Extension> ComponentLoader::CreateExtension(
    const ComponentExtensionInfo& info,
    std::string* utf8_error) {
  // TODO(abarth): We should REQUIRE_MODERN_MANIFEST_VERSION once we've updated
  //               our component extensions to the new manifest version.
  int flags = Extension::REQUIRE_KEY;
  return Extension::Create(info.root_directory,
                           mojom::ManifestLocation::kComponent, info.manifest,
                           flags, utf8_error);
}

// static
void ComponentLoader::EnableBackgroundExtensionsForTesting() {
  g_enable_background_extensions_during_testing = true;
}

void ComponentLoader::AddDefaultComponentExtensions(
    bool skip_session_components) {
  // Do not add component extensions that have background pages here -- add them
  // to AddDefaultComponentExtensionsWithBackgroundPages.
  DCHECK(!skip_session_components);

  if (!skip_session_components) {
    AddWebStoreApp();
#if BUILDFLAG(ENABLE_PDF)
    Add(pdf_extension_util::GetManifest(),
        base::FilePath(FILE_PATH_LITERAL("pdf")));
#endif  // BUILDFLAG(ENABLE_PDF)
  }

  AddDefaultComponentExtensionsWithBackgroundPages(skip_session_components);
}

void ComponentLoader::AddDefaultComponentExtensionsForKioskMode(
    bool skip_session_components) {
  // Do not add component extensions that have background pages here -- add them
  // to AddDefaultComponentExtensionsWithBackgroundPagesForKioskMode.

  // No component extension for kiosk app launch splash screen.
  if (skip_session_components) {
    return;
  }

  AddDefaultComponentExtensionsWithBackgroundPagesForKioskMode();

#if BUILDFLAG(ENABLE_PDF)
  Add(pdf_extension_util::GetManifest(),
      base::FilePath(FILE_PATH_LITERAL("pdf")));
#endif
}

void ComponentLoader::AddDefaultComponentExtensionsWithBackgroundPages(
    bool skip_session_components) {
  // TODO(mshin): Support Hangout Service 
}

void ComponentLoader::
    AddDefaultComponentExtensionsWithBackgroundPagesForKioskMode() {
  // Not support Kiosk mode
}

void ComponentLoader::UnloadComponent(ComponentExtensionInfo* component) {
  if (extension_system_->is_ready()) {
    extension_system_->extension_service()->RemoveComponentExtension(
        component->extension_id);
  }
}

}  // namespace components_extensions
