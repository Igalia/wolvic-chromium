// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_ACTIVE_INSTALL_DATA_H_
#define COMPONENTS_EXTENSIONS_BROWSER_ACTIVE_INSTALL_DATA_H_

#include "extensions/common/extension_id.h"

namespace components_extensions {

// Details of an active extension install.
struct ActiveInstallData {
  ActiveInstallData() = default;
  explicit ActiveInstallData(const extensions::ExtensionId& extension_id);

  extensions::ExtensionId extension_id;
  int percent_downloaded = 0;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_ACTIVE_INSTALL_DATA_H_
