// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Download code which handles CRX files (extensions, themes, apps, ...).

#ifndef COMPONENTS_EXTENSIONS_BROWSER_DOWNLOAD_CRX_UTIL_H_
#define COMPONENTS_EXTENSIONS_BROWSER_DOWNLOAD_CRX_UTIL_H_

#include <memory>

#include "base/auto_reset.h"
#include "base/memory/ref_counted.h"

class ExtensionInstallPrompt;

namespace content {
class BrowserContext;
}

namespace download {
class DownloadItem;
}

namespace components_extensions {
class CrxInstaller;
}

namespace download_crx_util {

// Allow tests to install a mock ExtensionInstallPrompt object, to fake
// user clicks on the permissions dialog.
void SetMockInstallPromptForTesting(
    std::unique_ptr<ExtensionInstallPrompt> mock_prompt);

// Create and pre-configure a CrxInstaller for a given |download_item|.
scoped_refptr<components_extensions::CrxInstaller> CreateCrxInstaller(
    content::BrowserContext* browser_context,
    const download::DownloadItem& download_item);

// Returns true if this is an extension download. This also considers user
// scripts to be extension downloads, since we convert those automatically.
bool IsExtensionDownload(const download::DownloadItem& download_item);

// Checks whether a download is an extension from a whitelisted site in prefs.
bool IsTrustedExtensionDownload(content::BrowserContext* browser_context,
                                const download::DownloadItem& item);

// Allows tests to override whether offstore extension installs are allowed
// for testing purposes.
std::unique_ptr<base::AutoReset<bool>> OverrideOffstoreInstallAllowedForTesting(
    bool allowed);

// Returns true if an offstore extension download should be allowed to proceed.
// Takes into consideration what's set in
// OverrideOffstoreInstallAllowedForTesting
bool OffStoreInstallAllowedByPrefs(content::BrowserContext* browser_context,
                                   const download::DownloadItem& item);

}  // namespace download_crx_util

#endif  // COMPONENTS_EXTENSIONS_BROWSER_DOWNLOAD_CRX_UTIL_H_
