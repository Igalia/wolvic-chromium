// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/chrome_extension_browser_constants.h"

namespace components_extension_constants {

// The link to the help article for runtime host permissions.
const char kRuntimeHostPermissionsHelpURL[] =
    "https://support.google.com/chrome?p=enable_extensions";

// The link to the site permissions settings page.
const char kExtensionsSitePermissionsURL[] =
    "chrome://extensions/sitePermissions";

// The link to the help article for click to script, which contains info on
// enabling extensions to request access to the current site through the
// toolbar.
// TODO(crbug.com/1353587): This link is likely temporary, and may need to be
// replaced later.
const char kShowAccessRequestsInToolbarHelpURL[] =
    "https://support.google.com/chrome_webstore/answer/2664769";

}  // namespace components_extension_constants
