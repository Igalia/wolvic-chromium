// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/webstore_reinstaller.h"

#include <utility>

#include "base/memory/ref_counted.h"
#include "components/extensions/browser/extension_install_prompt.h"
#include "components/extensions/browser/extension_service.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/extension_system.h"

using extensions::ExtensionPrefs;
using extensions::ExtensionSystem;

namespace components_extensions {

namespace disable_reason = extensions::disable_reason;

namespace {
const char kCouldNotUninstallExtension[] = "Failed to uninstall the extension.";
const char kTabClosed[] = "Tab was closed.";
}

WebstoreReinstaller::WebstoreReinstaller(
    content::WebContents* web_contents,
    const std::string& extension_id,
    WebstoreStandaloneInstaller::Callback callback)
    : WebstoreStandaloneInstaller(
          extension_id,
          web_contents->GetBrowserContext(),
          std::move(callback)),
      content::WebContentsObserver(web_contents) {
  DCHECK(
      ExtensionPrefs::Get(web_contents->GetBrowserContext())
          ->HasDisableReason(extension_id, disable_reason::DISABLE_CORRUPTED));
}

WebstoreReinstaller::~WebstoreReinstaller() {
}

void WebstoreReinstaller::BeginReinstall() {
  WebstoreStandaloneInstaller::BeginInstall();
}

bool WebstoreReinstaller::CheckRequestorAlive() const {
  return web_contents() != nullptr;
}

std::unique_ptr<ExtensionInstallPrompt::Prompt>
WebstoreReinstaller::CreateInstallPrompt() const {
  std::unique_ptr<ExtensionInstallPrompt::Prompt> prompt(
      new ExtensionInstallPrompt::Prompt(
          ExtensionInstallPrompt::REPAIR_PROMPT));
  prompt->SetWebstoreData(localized_user_count(),
                          show_user_count(),
                          average_rating(),
                          rating_count());
  return prompt;
}

bool WebstoreReinstaller::ShouldShowPostInstallUI() const {
  return false;
}

content::WebContents* WebstoreReinstaller::GetWebContents() const {
  return web_contents();
}

void WebstoreReinstaller::WebContentsDestroyed() {
  // Run the callback now, because AbortInstall() doesn't do it.
  RunCallback(false, kTabClosed, webstore_install::ABORTED);
  AbortInstall();
}

void WebstoreReinstaller::OnInstallPromptDone(
    ExtensionInstallPrompt::DoneCallbackPayload payload) {
  // This dialog doesn't support the "withhold permissions" checkbox.
  DCHECK_NE(payload.result,
            ExtensionInstallPrompt::Result::ACCEPTED_WITH_WITHHELD_PERMISSIONS);

  if (payload.result != ExtensionInstallPrompt::Result::ACCEPTED) {
    WebstoreStandaloneInstaller::OnInstallPromptDone(std::move(payload));
    return;
  }

  if (!ExtensionSystem::Get(browser_context())->extension_service()->UninstallExtension(
          id(), extensions::UNINSTALL_REASON_REINSTALL, nullptr)) {
    // Run the callback now, because AbortInstall() doesn't do it.
    RunCallback(
        false, kCouldNotUninstallExtension, webstore_install::OTHER_ERROR);
    AbortInstall();
    return;
  }
  WebstoreStandaloneInstaller::OnInstallPromptDone(std::move(payload));
}

}  // namespace components_extensions
