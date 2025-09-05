// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/active_install_data.h"

#include "extensions/common/extension_id.h"

namespace components_extensions {

ActiveInstallData::ActiveInstallData(const extensions::ExtensionId& extension_id)
    : extension_id(extension_id) {}

}  // namespace components_extensions
