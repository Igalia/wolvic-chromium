// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_UTIL_H_
#define COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_UTIL_H_

#include <memory>
#include <string>

#include "base/values.h"
#include "extensions/common/constants.h"
#include "ui/base/window_open_disposition.h"

namespace content {
class BrowserContext;
class WebContents;
}

namespace extensions {
class Extension;
class PermissionSet;
}

class GURL;

namespace components_extensions {


namespace util {

// Returns true if the extension associated with |extension_id| has isolated
// storage. This can be either because it is an app that requested this in its
// manifest, or because it is a policy-installed app or extension running on
// the Chrome OS sign-in profile.
bool HasIsolatedStorage(const std::string& extension_id,
                        content::BrowserContext* context);
bool HasIsolatedStorage(const extensions::Extension& extension,
                        content::BrowserContext* context);

// Sets whether |extension_id| can run in an incognito window. Reloads the
// extension if it's enabled since this permission is applied at loading time
// only. Note that an ExtensionService must exist.
void SetIsIncognitoEnabled(const std::string& extension_id,
                           content::BrowserContext* context,
                           bool enabled);

// Sets whether |extension_id| can inject scripts into pages with file URLs.
// Reloads the extension if it's enabled since this permission is applied at
// loading time only. Note than an ExtensionService must exist.
void SetAllowFileAccess(const std::string& extension_id,
                        content::BrowserContext* context,
                        bool allow);

// Returns true if |extension| should be synced.
bool ShouldSync(const extensions::Extension* extension, content::BrowserContext* context);

// Returns true if |extension_id| is idle and it is safe to perform actions such
// as updating.
bool IsExtensionIdle(const std::string& extension_id,
                     content::BrowserContext* context);

// Sets the name, id, and icon resource path of the given extension into the
// returned dictionary.
base::Value::Dict GetExtensionInfo(const extensions::Extension* extension);

// Returns a PermissionSet configured with the permissions that should be
// displayed in an extension installation prompt for the specified |extension|.
std::unique_ptr<const extensions::PermissionSet> GetInstallPromptPermissionSetForExtension(
    const extensions::Extension* extension,
    content::BrowserContext* context,
    bool include_optional_permissions);

// Returns all profiles affected by permissions of an extension running in
// "spanning" (rather than "split) mode.
std::vector<content::BrowserContext*> GetAllRelatedProfiles(
    content::BrowserContext* context,
    const extensions::Extension& extension);

// Sets whether the given `profile` is in developer mode and notifies
// relevant subsystems.
void SetDeveloperModeForProfile(content::BrowserContext* context,
                                bool in_developer_mode);

// Implement function from chrome/browser/ui/browser_navigator.cc
void Navigate(
    const GURL& url,
    content::BrowserContext* context,
    content::WebContents* target_contents = nullptr,
    WindowOpenDisposition disposition = WindowOpenDisposition::CURRENT_TAB);

}  // namespace util
}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_UTIL_H_
