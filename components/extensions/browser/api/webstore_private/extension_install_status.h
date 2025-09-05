// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_WEBSTORE_PRIVATE_EXTENSION_INSTALL_STATUS_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_WEBSTORE_PRIVATE_EXTENSION_INSTALL_STATUS_H_

#include "extensions/common/extension_id.h"
#include "extensions/common/manifest.h"

namespace content {
class BrowserContext;
}

namespace extensions {
class PermissionSet;
}

namespace components_extensions {

enum ExtensionInstallStatus {
  // Extension is blocked by policy but can be requested.
  kCanRequest,
  // Extension install request has been sent and is waiting to be reviewed.
  kRequestPending,
  // Extension is blocked by policy and can not be requested.
  kBlockedByPolicy,
  // Extension is not installed and has not not been blocked by policy.
  kInstallable,
  // Extension has been installed and it's enabled.
  kEnabled,
  // Extension has been installed but it's disabled and not blocked by policy.
  kDisabled,
  // Extension has been installed but it's terminated.
  kTerminated,
  // Extension is blocklisted.
  kBlocklisted,
  // Extension requires custodian approval to enable.
  kCustodianApprovalRequired,
  // Extension is force installed or recommended by policy.
  kForceInstalled
};

// Returns the Extension install status for a Chrome web store extension with
// |extension_id| in |browser_context|. Note that this function won't check whether the
// extension's manifest type, required permissions are blocked by enterprise
// policy. type blocking or permission blocking or manifest version. Please use
// this function only if manifest file is not available.
ExtensionInstallStatus GetWebstoreExtensionInstallStatus(
    const extensions::ExtensionId& extension_id,
    content::BrowserContext* browser_context);

// Returns the Extension install status for a Chrome web store extension with
// `extension_id` in `browser_context`. Also check if `manifest_type`, any permission
// in `required_permission_set` is blocked by enterprise policy or
// `manifest_version` is allowed.  `manifest_version` is only valid for
// TYPE_EXTENSION.
ExtensionInstallStatus GetWebstoreExtensionInstallStatus(
    const extensions::ExtensionId& extension_id,
    content::BrowserContext* browser_context,
    const extensions::Manifest::Type manifest_type,
    const extensions::PermissionSet& required_permission_set,
    int manifest_version = 3);

}  // namespace components_extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_WEBSTORE_PRIVATE_EXTENSION_INSTALL_STATUS_H_
