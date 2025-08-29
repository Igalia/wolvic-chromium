// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/component_extensions_allowlist/allowlist.h"

#include <stddef.h>

#include "base/logging.h"
#include "base/notreached.h"
#include "build/build_config.h"
#include "components/extensions/common/extension_constants.h"
#include "components/grit/component_extension_resources.h"
#include "extensions/common/constants.h"
#include "printing/buildflags/buildflags.h"


namespace components_extensions {

bool IsComponentExtensionAllowlisted(const std::string& extension_id) {
  const char* const kAllowed[] = {
    extension_misc::kInAppPaymentsSupportAppId,
    extension_misc::kPdfExtensionId,
  };

  for (size_t i = 0; i < std::size(kAllowed); ++i) {
    if (extension_id == kAllowed[i])
      return true;
  }

  LOG(ERROR) << "Component extension with id " << extension_id << " not in "
             << "allowlist and is not being loaded as a result.";
  NOTREACHED();
  return false;
}

bool IsComponentExtensionAllowlisted(int manifest_resource_id) {
  switch (manifest_resource_id) {
    // Please keep the list in alphabetical order.
    case IDR_NETWORK_SPEECH_SYNTHESIS_MANIFEST:
    case IDR_WEBSTORE_MANIFEST:
      return true;
  }

  LOG(ERROR) << "Component extension with manifest resource id "
             << manifest_resource_id << " not in allowlist and is not being "
             << "loaded as a result.";
  NOTREACHED();
  return false;
}

}  // namespace components_extensions
