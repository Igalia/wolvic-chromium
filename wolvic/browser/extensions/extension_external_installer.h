// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WOLVIC_BROWSER_EXTENSIONS_EXTENSION_EXTERNAL_INSTALLER_H_
#define WOLVIC_BROWSER_EXTENSIONS_EXTENSION_EXTERNAL_INSTALLER_H_

#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_contents_user_data.h"

namespace base {
class FilePath;
}

namespace content {
class WebContents;
}

namespace extensions {
class Extension;
class CrxInstallError;
}

namespace wolvic {

class ExtensionExternalInstaller : public content::WebContentsUserData<ExtensionExternalInstaller> {
 public:
  ExtensionExternalInstaller(const ExtensionExternalInstaller&) = delete;
  ExtensionExternalInstaller& operator=(const ExtensionExternalInstaller&) = delete;
  ~ExtensionExternalInstaller() override;

  void Install(const base::FilePath& path,
    base::OnceCallback<void(bool,std::string)> callback);
  void Unpack(const base::FilePath& unpack_dir);

 private:
  friend class content::WebContentsUserData<ExtensionExternalInstaller>;
  explicit ExtensionExternalInstaller(content::WebContents* web_contents);

  void OnCrxInstallerDone(const absl::optional<extensions::CrxInstallError>& error);
  void OnZipInstallerDone(const base::FilePath& zip_file,
                          const base::FilePath& unzip_dir,
                          const std::string& error);
  void OnUnpackComplete(const extensions::Extension* extension,
                        const base::FilePath& file_path,
                        const std::string& error);

  base::OnceCallback<void(bool,std::string)> callback_;
  base::WeakPtrFactory<ExtensionExternalInstaller> weak_ptr_factory_{this};

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

}  // namespace wolvic

#endif  // WOLVIC_BROWSER_EXTENSIONS_EXTENSION_EXTERNAL_INSTALLER_H_
