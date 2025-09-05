// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/webstore_install_with_prompt.h"

#include <utility>

#include "components/extensions/browser/webstore_installer.h"
#include "content/public/browser/web_contents.h"

using content::BrowserContext;
using content::WebContents;

namespace components_extensions {

WebstoreInstallWithPrompt::WebstoreInstallWithPrompt(
    const std::string& webstore_item_id,
    BrowserContext* browser_context,
    Callback callback)
    : WebstoreStandaloneInstaller(webstore_item_id,
                                  browser_context,
                                  std::move(callback)),
      show_post_install_ui_(true),
      dummy_web_contents_(
          WebContents::Create(WebContents::CreateParams(browser_context))),
      parent_window_(nullptr) {
  dummy_web_contents_->SetOwnerLocationForDebug(FROM_HERE);
  set_install_source(WebstoreInstaller::INSTALL_SOURCE_OTHER);
}

WebstoreInstallWithPrompt::WebstoreInstallWithPrompt(
    const std::string& webstore_item_id,
    BrowserContext* browser_context,
    gfx::NativeWindow parent_window,
    Callback callback)
    : WebstoreStandaloneInstaller(webstore_item_id,
                                  browser_context,
                                  std::move(callback)),
      show_post_install_ui_(true),
      dummy_web_contents_(
          WebContents::Create(WebContents::CreateParams(browser_context))),
      parent_window_(parent_window) {
  if (parent_window_)
    parent_window_tracker_ = views::NativeWindowTracker::Create(parent_window);
  dummy_web_contents_->SetOwnerLocationForDebug(FROM_HERE);
  set_install_source(WebstoreInstaller::INSTALL_SOURCE_OTHER);
}

WebstoreInstallWithPrompt::~WebstoreInstallWithPrompt() {
}

bool WebstoreInstallWithPrompt::CheckRequestorAlive() const {
  if (!parent_window_) {
    // Assume the requestor is always alive if |parent_window_| is null.
    return true;
  }
  return !parent_window_tracker_->WasNativeWindowDestroyed();
}

std::unique_ptr<ExtensionInstallPrompt::Prompt>
WebstoreInstallWithPrompt::CreateInstallPrompt() const {
  return std::make_unique<ExtensionInstallPrompt::Prompt>(
      ExtensionInstallPrompt::INSTALL_PROMPT);
}

std::unique_ptr<ExtensionInstallPrompt>
WebstoreInstallWithPrompt::CreateInstallUI() {
  // Create an ExtensionInstallPrompt. If the parent window is NULL, the dialog
  // will be placed in the middle of the screen.
  return std::make_unique<ExtensionInstallPrompt>(browser_context(), parent_window_);
}

bool WebstoreInstallWithPrompt::ShouldShowPostInstallUI() const {
  return show_post_install_ui_;
}

WebContents* WebstoreInstallWithPrompt::GetWebContents() const {
  return dummy_web_contents_.get();
}

}  // namespace components_extensions
