// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/extension_assets_manager.h"

#include "base/memory/singleton.h"
#include "extensions/common/extension.h"
#include "extensions/common/file_util.h"

using extensions::Extension;

namespace components_extensions {
namespace file_util = extensions::file_util;

namespace {

class ExtensionAssetsManagerImpl :  public ExtensionAssetsManager {
 public:
  ExtensionAssetsManagerImpl(const ExtensionAssetsManagerImpl&) = delete;
  ExtensionAssetsManagerImpl& operator=(const ExtensionAssetsManagerImpl&) =
      delete;

  static ExtensionAssetsManagerImpl* GetInstance() {
    return base::Singleton<ExtensionAssetsManagerImpl>::get();
  }

  // Override from ExtensionAssetsManager.
  void InstallExtension(
      const Extension* extension,
      const base::FilePath& unpacked_extension_root,
      const base::FilePath& local_install_dir,
      content::BrowserContext* browser_context,
      InstallExtensionCallback callback,
      bool updates_from_webstore_or_empty_update_url) override {
    std::move(callback).Run(file_util::InstallExtension(
        unpacked_extension_root, extension->id(), extension->VersionString(),
        local_install_dir));
  }

  void UninstallExtension(const std::string& id,
                          const std::string& profile_user_name,
                          const base::FilePath& extensions_install_dir,
                          const base::FilePath& extension_dir_to_delete,
                          const base::FilePath& profile_dir) override {
    file_util::UninstallExtension(profile_dir, extensions_install_dir,
                                  extension_dir_to_delete);
  }

 private:
  friend struct base::DefaultSingletonTraits<ExtensionAssetsManagerImpl>;

  ExtensionAssetsManagerImpl() {}
  ~ExtensionAssetsManagerImpl() override {}
};

}  // namespace

// static
ExtensionAssetsManager* ExtensionAssetsManager::GetInstance() {
  // If not Chrome OS, use trivial implementation that doesn't share anything.
  return ExtensionAssetsManagerImpl::GetInstance();
}

}  // namespace components_extensions
