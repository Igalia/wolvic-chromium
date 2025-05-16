// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_COMMON_PERMISSIONS_CHROME_PERMISSION_MESSAGE_PROVIDER_H_
#define COMPONENTS_EXTENSIONS_COMMON_PERMISSIONS_CHROME_PERMISSION_MESSAGE_PROVIDER_H_

#include <set>
#include <vector>

#include "extensions/common/permissions/permission_message_provider.h"

namespace components_extensions {

// Tested in two places:
// 1. chrome_permission_message_provider_unittest.cc, which is a regular unit
//    test for this class
// 2. components/extensions/browser/permission_messages_unittest.cc, which is an
//    integration test that ensures messages are correctly generated for
//    extensions created through the extension system.
class ChromePermissionMessageProvider : public extensions::PermissionMessageProvider {
 public:
  ChromePermissionMessageProvider();

  ChromePermissionMessageProvider(const ChromePermissionMessageProvider&) =
      delete;
  ChromePermissionMessageProvider& operator=(
      const ChromePermissionMessageProvider&) = delete;

  ~ChromePermissionMessageProvider() override;

  // PermissionMessageProvider implementation.
  extensions::PermissionMessages GetPermissionMessages(
      const extensions::PermissionIDSet& permissions) const override;
  bool IsPrivilegeIncrease(const extensions::PermissionSet& granted_permissions,
                           const extensions::PermissionSet& requested_permissions,
                           extensions::Manifest::Type extension_type) const override;
  extensions::PermissionIDSet GetAllPermissionIDs(
      const extensions::PermissionSet& permissions,
      extensions::Manifest::Type extension_type) const override;

  // Returns the permissions IDs which should trigger a warning for the user in
  // chrome://management page.
  extensions::PermissionIDSet GetManagementUIPermissionIDs(
      const extensions::PermissionSet& permissions,
      extensions::Manifest::Type extension_type) const override;

 private:
  // Adds any permission IDs from API permissions to |permission_ids|.
  void AddAPIPermissions(const extensions::PermissionSet& permissions,
                         extensions::PermissionIDSet* permission_ids) const;

  // Adds any permission IDs from manifest permissions to |permission_ids|.
  void AddManifestPermissions(const extensions::PermissionSet& permissions,
                              extensions::PermissionIDSet* permission_ids) const;

  // Adds any permission IDs from host permissions to |permission_ids|.
  void AddHostPermissions(const extensions::PermissionSet& permissions,
                          extensions::PermissionIDSet* permission_ids,
                          extensions::Manifest::Type extension_type) const;

  // Adds the IDs of API permissions which should trigger a warning in
  // chrome://management.
  void AddAPIPermissionsForManagementUIWarning(
      const extensions::PermissionSet& permissions,
      extensions::PermissionIDSet* permission_ids) const;

  // Adds the IDs of manifest permissions which should trigger a warning in
  // chrome://management.
  void AddManifestPermissionsForManagementUIWarning(
      const extensions::PermissionSet& permissions,
      extensions::PermissionIDSet* permission_ids) const;

  // Returns true if |requested_permissions| has an elevated API or manifest
  // privilege level compared to |granted_permissions|.
  bool IsAPIOrManifestPrivilegeIncrease(
      const extensions::PermissionSet& granted_permissions,
      const extensions::PermissionSet& requested_permissions) const;

  // Returns true if |requested_permissions| has more host permissions compared
  // to |granted_permissions|.
  bool IsHostPrivilegeIncrease(const extensions::PermissionSet& granted_permissions,
                               const extensions::PermissionSet& requested_permissions,
                               extensions::Manifest::Type extension_type) const;

// TODO(mshin): Enable the below code after migrating ChromePermissionMessageRule
//   extensions::PermissionMessages GetPermissionMessagesHelper(
//       const extensions::PermissionIDSet& permissions,
//       const std::vector<extensions::ChromePermissionMessageRule>& rules) const;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_COMMON_PERMISSIONS_CHROME_PERMISSION_MESSAGE_PROVIDER_H_
