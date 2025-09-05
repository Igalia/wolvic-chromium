// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Download code which handles CRX files (extensions, themes, apps, ...).

#include "components/extensions/browser/download_crx_util.h"

#include <memory>

#include "components/extensions/browser/crx_installer.h"
#include "components/extensions/browser/extension_install_prompt.h"
#include "components/extensions/browser/extension_management.h"
#include "components/extensions/browser/webstore_installer.h"
#include "components/download/public/common/download_item.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_item_utils.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/user_script.h"

using content::BrowserContext;
using content::BrowserThread;
using download::DownloadItem;
using components_extensions::WebstoreInstaller;

namespace download_crx_util {

namespace {

bool g_allow_offstore_install_for_testing = false;

// Hold a mock ExtensionInstallPrompt object that will be used when the
// download system opens a CRX.
ExtensionInstallPrompt* mock_install_prompt_for_testing = nullptr;

// Called to get an extension install UI object.  In tests, will return
// a mock if the test calls download_util::SetMockInstallPromptForTesting()
// to set one.
std::unique_ptr<ExtensionInstallPrompt> CreateExtensionInstallPrompt(
    BrowserContext* browser_context,
    const DownloadItem& download_item) {
  // Use a mock if one is present.  Otherwise, create a real extensions
  // install UI.
  if (mock_install_prompt_for_testing) {
    ExtensionInstallPrompt* result = mock_install_prompt_for_testing;
    mock_install_prompt_for_testing = nullptr;
    return std::unique_ptr<ExtensionInstallPrompt>(result);
  } else {
    content::WebContents* web_contents =
        content::DownloadItemUtils::GetWebContents(
            const_cast<DownloadItem*>(&download_item));
    if (!web_contents) {
      // TODO(mshin): Enable the below code after supporting Profile
      // Browser* browser = chrome::FindLastActiveWithProfile(browser_context);
      // if (!browser) {
      //   browser = Browser::Create(
      //       Browser::CreateParams(Browser::TYPE_NORMAL, browser_context, true));
      // }
      // web_contents = browser->tab_strip_model()->GetActiveWebContents();
      DCHECK(false);
    }
    return std::make_unique<ExtensionInstallPrompt>(web_contents);
  }
}

}  // namespace

bool OffStoreInstallAllowedByPrefs(BrowserContext* browser_context, const DownloadItem& item) {
  return g_allow_offstore_install_for_testing ||
         components_extensions::ExtensionManagementFactory::GetForBrowserContext(browser_context)
             ->IsOffstoreInstallAllowed(item.GetURL(), item.GetReferrerUrl());
}

// Tests can call this method to inject a mock ExtensionInstallPrompt
// to be used to confirm permissions on a downloaded CRX.
void SetMockInstallPromptForTesting(
    std::unique_ptr<ExtensionInstallPrompt> mock_prompt) {
  mock_install_prompt_for_testing = mock_prompt.release();
}

scoped_refptr<components_extensions::CrxInstaller> CreateCrxInstaller(
    BrowserContext* browser_context,
    const download::DownloadItem& download_item) {
  components_extensions::ExtensionService* service =
      extensions::ExtensionSystem::Get(browser_context)->extension_service();
  CHECK(service);

  scoped_refptr<components_extensions::CrxInstaller> installer(
      components_extensions::CrxInstaller::Create(
          service,
          CreateExtensionInstallPrompt(browser_context, download_item),
          WebstoreInstaller::GetAssociatedApproval(download_item)));

  installer->set_error_on_unsupported_requirements(true);
  installer->set_delete_source(true);
  installer->set_install_cause(extension_misc::INSTALL_CAUSE_USER_DOWNLOAD);
  installer->set_original_mime_type(download_item.GetOriginalMimeType());
  installer->set_apps_require_extension_mime_type(true);

  return installer;
}

bool IsExtensionDownload(const DownloadItem& download_item) {
  if (download_item.GetTargetDisposition() ==
      DownloadItem::TARGET_DISPOSITION_PROMPT)
    return false;

  if (download_item.GetMimeType() == extensions::Extension::kMimeType ||
      extensions::UserScript::IsURLUserScript(download_item.GetURL(),
                                              download_item.GetMimeType())) {
    return true;
  } else {
    return false;
  }
}

bool IsTrustedExtensionDownload(BrowserContext* browser_context, const DownloadItem& item) {
  return IsExtensionDownload(item) &&
         OffStoreInstallAllowedByPrefs(browser_context, item);
}

std::unique_ptr<base::AutoReset<bool>> OverrideOffstoreInstallAllowedForTesting(
    bool allowed) {
  return std::make_unique<base::AutoReset<bool>>(
      &g_allow_offstore_install_for_testing, allowed);
}

}  // namespace download_crx_util
