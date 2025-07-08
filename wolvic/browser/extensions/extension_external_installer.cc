// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/browser/extensions/extension_external_installer.h"

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "components/extensions/browser/crx_installer.h"
#include "components/extensions/browser/extension_service.h"
#include "components/extensions/browser/unpacked_installer.h"
#include "extensions/browser/extension_file_task_runner.h"
#include "extensions/browser/zipfile_installer.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/api/declarative_net_request/rules_monitor_service.h"
#include "extensions/browser/api/declarative_net_request/ruleset_manager.h"

using components_extensions::CrxInstaller;
using components_extensions::ExtensionService;
using components_extensions::UnpackedInstaller;
using extensions::Extension;
using extensions::ExtensionSystem;
using extensions::ZipFileInstaller;

namespace wolvic {

ExtensionExternalInstaller::ExtensionExternalInstaller(
    content::WebContents* web_contents)
  : content::WebContentsUserData<ExtensionExternalInstaller>(*web_contents) {}

ExtensionExternalInstaller::~ExtensionExternalInstaller() {}

void ExtensionExternalInstaller::Install(
    const base::FilePath& path,
    base::OnceCallback<void(bool,std::string)> callback) {
  if (callback_) {
    std::move(callback).Run(false, "Installer is still running");
    return;
  }

  callback_ = std::move(callback);

  ExtensionService* service =
      ExtensionSystem::Get(
          GetWebContents().GetBrowserContext())->extension_service();
  if (path.MatchesExtension(FILE_PATH_LITERAL(".crx"))) {
    scoped_refptr<CrxInstaller> crx_installer =
        CrxInstaller::CreateSilent(service);
    crx_installer->set_error_on_unsupported_requirements(true);
    crx_installer->set_off_store_install_allow_reason(
        CrxInstaller::OffStoreInstallAllowedFromSettingsPage);
    crx_installer->set_install_immediately(true);
    crx_installer->AddInstallerCallback(
        base::BindOnce(&ExtensionExternalInstaller::OnCrxInstallerDone,
                        weak_ptr_factory_.GetWeakPtr()));
    crx_installer->InstallCrx(path);
    LOG(ERROR) << "[DEMO] Run install Extension crx " << path;
  } else if (path.MatchesExtension(FILE_PATH_LITERAL(".zip"))) {
      ZipFileInstaller::Create(
          extensions::GetExtensionFileTaskRunner(),
          base::BindOnce(&ExtensionExternalInstaller::OnZipInstallerDone,
                        weak_ptr_factory_.GetWeakPtr()))
              ->InstallZipFileToTempDir(path);
    LOG(ERROR) << "[DEMO] Run install Extension zip " << path;
  } else {
    LOG(ERROR) << "[DEMO] unsupported file format for extension " << path;
    std::move(callback_).Run(false, "unsupported file format for extensions");
  }
}

void ExtensionExternalInstaller::OnCrxInstallerDone(
    const absl::optional<extensions::CrxInstallError>& error) {
  std::string utf8_error;
  bool success = true;
  if (error) {
    success = false;
    utf8_error = base::UTF16ToUTF8(error->message());
  }
  LOG(ERROR) << "[DEMO] Done to install Extension with crx : " << (success ? std::string("Success") : utf8_error);

  DCHECK(callback_);
  std::move(callback_).Run(success, utf8_error);
}

void ExtensionExternalInstaller::OnZipInstallerDone(
    const base::FilePath& zip_file,
    const base::FilePath& unzip_dir,
    const std::string& error) {
  bool success = true;
  if (unzip_dir.empty()) {
    DCHECK(!error.empty());
    success = false;
  } else {
    DCHECK(error.empty());
    ExtensionService* service =
        ExtensionSystem::Get(
            GetWebContents().GetBrowserContext())->extension_service();
    UnpackedInstaller::Create(service)->Load(unzip_dir);
  }
  LOG(ERROR) << "[DEMO] Done to install Extension with zip : " << (success ? "Success" : error);

  std::move(callback_).Run(success, error);
}

void ExtensionExternalInstaller::Unpack(const base::FilePath& unpack_dir) {
  ExtensionService* service =
      ExtensionSystem::Get(
          GetWebContents().GetBrowserContext())->extension_service();

  auto* rules_monitor_service =
    extensions::declarative_net_request::RulesMonitorService::Get(GetWebContents().GetBrowserContext());
  DCHECK(rules_monitor_service && rules_monitor_service->ruleset_manager());

  LOG(ERROR) << "[DEMO] Unpack ExtensionService=" << service;
  scoped_refptr<UnpackedInstaller> installer(UnpackedInstaller::Create(service));
  installer->set_be_noisy_on_failure(true);
  installer->set_allow_file_access(true);
  installer->set_completion_callback(base::BindOnce(
      &ExtensionExternalInstaller::OnUnpackComplete,
      weak_ptr_factory_.GetWeakPtr()));
  installer->Load(unpack_dir);
}

void ExtensionExternalInstaller::OnUnpackComplete(
    const Extension* extension,
    const base::FilePath& file_path,
    const std::string& error) {
  LOG(ERROR) << "[DEMO] Done to install Extension with Unpack : " << (extension ? "Success, path=" + file_path.MaybeAsASCII() : error);
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(ExtensionExternalInstaller);

}  // namespace wolvic
