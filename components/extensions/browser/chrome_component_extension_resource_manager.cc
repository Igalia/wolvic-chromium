// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/chrome_component_extension_resource_manager.h"

#include <map>
#include <string>

#include "base/check.h"
#include "base/containers/contains.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/values.h"
#include "build/build_config.h"
#include "content/public/browser/browser_thread.h"
#include "components/grit/components_scaled_resources.h"
#include "components/grit/component_extension_resources_map.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension_id.h"
#include "pdf/buildflags.h"
#include "ppapi/buildflags/buildflags.h"
#include "ui/base/resource/resource_bundle.h"

#if 0//BUILDFLAG(ENABLE_PDF)
#include <utility>
#include "chrome/browser/pdf/pdf_extension_util.h"
#include "chrome/grit/pdf_resources_map.h"
#endif  // BUILDFLAG(ENABLE_PDF)

using extensions::ExtensionId;

namespace components_extensions {

class ChromeComponentExtensionResourceManager::Data {
 public:
  using TemplateReplacementMap =
      std::map<ExtensionId, ui::TemplateReplacements>;

  Data();
  Data(const Data&) = delete;
  Data& operator=(const Data&) = delete;
  ~Data() = default;

  const std::map<base::FilePath, int>& path_to_resource_id() const {
    return path_to_resource_id_;
  }

  const TemplateReplacementMap& template_replacements() const {
    return template_replacements_;
  }

 private:
  void AddComponentResourceEntries(const webui::ResourcePath* entries,
                                   size_t size);

  // A map from a resource path to the resource ID. Used by
  // ChromeComponentExtensionResourceManager::IsComponentExtensionResource().
  std::map<base::FilePath, int> path_to_resource_id_;

  // A map from an extension ID to its i18n template replacements.
  TemplateReplacementMap template_replacements_;
};

ChromeComponentExtensionResourceManager::Data::Data() {
  static const webui::ResourcePath kExtraComponentExtensionResources[] = {
    {"web_store/webstore_icon_128.png", IDR_WEBSTORE_ICON},
    {"web_store/webstore_icon_16.png", IDR_WEBSTORE_ICON_16},
  };

  AddComponentResourceEntries(kComponentExtensionResources,
                              kComponentExtensionResourcesSize);
  AddComponentResourceEntries(kExtraComponentExtensionResources,
                              std::size(kExtraComponentExtensionResources));

#if BUILDFLAG(ENABLE_PDF)
  AddComponentResourceEntries(kPdfResources, kPdfResourcesSize);

  // ResourceBundle is not always initialized in unit tests.
  if (ui::ResourceBundle::HasSharedInstance()) {
    base::Value::Dict dict;
    pdf_extension_util::AddStrings(
        pdf_extension_util::PdfViewerContext::kPdfViewer, &dict);

    ui::TemplateReplacements pdf_viewer_replacements;
    ui::TemplateReplacementsFromDictionaryValue(dict, &pdf_viewer_replacements);
    template_replacements_[extension_misc::kPdfExtensionId] =
        std::move(pdf_viewer_replacements);
  }
#endif
}

void ChromeComponentExtensionResourceManager::Data::AddComponentResourceEntries(
    const webui::ResourcePath* entries,
    size_t size) {
  for (size_t i = 0; i < size; ++i) {
    base::FilePath resource_path =
        base::FilePath().AppendASCII(entries[i].path);
    resource_path = resource_path.NormalizePathSeparators();

    DCHECK(!base::Contains(path_to_resource_id_, resource_path));
    path_to_resource_id_[resource_path] = entries[i].id;
  }
}

ChromeComponentExtensionResourceManager::
    ChromeComponentExtensionResourceManager() = default;

ChromeComponentExtensionResourceManager::
    ~ChromeComponentExtensionResourceManager() = default;

bool ChromeComponentExtensionResourceManager::IsComponentExtensionResource(
    const base::FilePath& extension_path,
    const base::FilePath& resource_path,
    int* resource_id) const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  base::FilePath directory_path = extension_path;
  base::FilePath resources_dir;
  base::FilePath relative_path;

#if BUILDFLAG(IS_ANDROID)
  if (!base::PathService::Get(base::DIR_ANDROID_APP_DATA, &resources_dir)) {
    return false;
  }
#else
  if (!base::PathService::Get(base::DIR_ASSETS, &resources_dir)) {
    return false;
  }
#endif
  resources_dir = resources_dir.Append(FILE_PATH_LITERAL("resources"));
  if (!resources_dir.AppendRelativePath(directory_path, &relative_path)) {
    return false;
  }
  relative_path = relative_path.Append(resource_path);
  relative_path = relative_path.NormalizePathSeparators();

  LazyInitData();
  auto entry = data_->path_to_resource_id().find(relative_path);
  if (entry == data_->path_to_resource_id().end())
    return false;

  *resource_id = entry->second;
  return true;
}

const ui::TemplateReplacements*
ChromeComponentExtensionResourceManager::GetTemplateReplacementsForExtension(
    const ExtensionId& extension_id) const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  LazyInitData();

  auto it = data_->template_replacements().find(extension_id);
  return it != data_->template_replacements().end() ? &it->second : nullptr;
}

void ChromeComponentExtensionResourceManager::LazyInitData() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  if (!data_)
    data_ = std::make_unique<Data>();
}

}  // namespace components_extensions
