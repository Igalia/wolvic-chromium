// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_PERMISSIONS_TEST_UTIL_H_
#define COMPONENTS_EXTENSIONS_BROWSER_PERMISSIONS_TEST_UTIL_H_

#include <string>
#include <vector>

#include "components/extensions/browser/permissions_updater.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace extensions {
class Extension;
class PermissionSet;
class URLPatternSet;
}

namespace components_extensions {

namespace permissions_test_util {

// Returns a list of |patterns| as strings, making it easy to compare for
// equality with readable errors. This will omit the chrome://favicon host, if
// present, from the result.
std::vector<std::string> GetPatternsAsStrings(const extensions::URLPatternSet& patterns);

// Calls corresponding PermissionsUpdater method respectively and wait for its
// asynchronous completion.
void GrantOptionalPermissionsAndWaitForCompletion(
    content::BrowserContext* browser_context,
    const extensions::Extension& extension,
    const extensions::PermissionSet& permissions);
void GrantRuntimePermissionsAndWaitForCompletion(
    content::BrowserContext* browser_context,
    const extensions::Extension& extension,
    const extensions::PermissionSet& permissions);
void RevokeOptionalPermissionsAndWaitForCompletion(
    content::BrowserContext* browser_context,
    const extensions::Extension& extension,
    const extensions::PermissionSet& permissions,
    PermissionsUpdater::RemoveType remove_type);
void RevokeRuntimePermissionsAndWaitForCompletion(
    content::BrowserContext* browser_context,
    const extensions::Extension& extension,
    const extensions::PermissionSet& permissions);

}  // namespace permissions_test_util
}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_PERMISSIONS_TEST_UTIL_H_
