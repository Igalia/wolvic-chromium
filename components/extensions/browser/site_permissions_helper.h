// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_SITE_PERMISSIONS_HELPER_H_
#define COMPONENTS_EXTENSIONS_BROWSER_SITE_PERMISSIONS_HELPER_H_

#include "base/memory/raw_ptr.h"
#include "extensions/browser/permissions_manager.h"

namespace content {
class BrowserContext;
class WebContents;
}

namespace extensions {
class Extension;
}

namespace components_extensions {

// A helper class responsible for providing the permissions data to models used
// in the Extensions toolbar (e.g: ExtensionContextMenuModel).
class SitePermissionsHelper {
 public:
  // The interaction of the extension with the site. This is independent
  // of the action's clickability.
  // TODO(crbug.com/1289441): Move enum and related methods to
  // PermissionsManager.
  enum class SiteInteraction {
    // The extension cannot run on the site.
    kNone,
    // The extension has withheld site access by the user.
    kWithheld,
    // The extension has activeTab permission to run on the site, but is pending
    // user action to run.
    kActiveTab,
    // The extension has permission to run on the site.
    kGranted,
  };

  explicit SitePermissionsHelper(content::BrowserContext* browser_context);
  SitePermissionsHelper(const SitePermissionsHelper&) = delete;
  const SitePermissionsHelper& operator=(const SitePermissionsHelper&) = delete;
  ~SitePermissionsHelper();

  // Returns the site interaction for `extension` in the current site pointed by
  // `web_contents`.
  SiteInteraction GetSiteInteraction(const extensions::Extension& extension,
                                     content::WebContents* web_contents) const;

  // Updates the site access pointed to by `web_contents` to `new_access` for
  // `extension`. If relevant, this will run any pending extension actions on
  // that site.
  void UpdateSiteAccess(const extensions::Extension& extension,
                        content::WebContents* web_contents,
                        extensions::PermissionsManager::UserSiteAccess new_access);

  // Returns whether the `extension` has been blocked on the given
  // `web_contents`.
  bool HasBeenBlocked(const extensions::Extension& extension,
                      content::WebContents* web_contents) const;

  // Returns whether the `blocked_actions` need a page refresh to run.
  bool PageNeedsRefreshToRun(int blocked_actions);

  // Returns true if `extension_id` can show site access requests in the
  // toolbar.
  bool ShowAccessRequestsInToolbar(const std::string& extension_id);

  // Sets whether `extenson_id` can show site access requests in the toolbar.
  void SetShowAccessRequestsInToolbar(const std::string& extension_id,
                                      bool show_access_requests_in_toolbar);

 private:
  raw_ptr<content::BrowserContext> browser_context_;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_SITE_PERMISSIONS_HELPER_H_
