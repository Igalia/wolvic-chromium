// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_COMMON_PERMISSIONS_CHROME_API_PERMISSIONS_H_
#define COMPONENTS_EXTENSIONS_COMMON_PERMISSIONS_CHROME_API_PERMISSIONS_H_

#include "base/containers/span.h"
#include "extensions/common/alias.h"
#include "extensions/common/permissions/api_permission.h"

namespace components_extensions {
namespace chrome_api_permissions {

// Returns the information necessary to construct chrome-layer extension
// APIPermissions.
base::span<const extensions::APIPermissionInfo::InitInfo> GetPermissionInfos();

// Returns the list of aliases for chrome-layer extension APIPermissions.
base::span<const extensions::Alias> GetPermissionAliases();

}  // namespace chrome_api_permissions
}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_COMMON_PERMISSIONS_CHROME_API_PERMISSIONS_H_
