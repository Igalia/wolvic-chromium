// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/chrome_process_manager_delegate.h"

#include "base/check_op.h"
#include "base/command_line.h"
#include "components/user_manager/user_manager.h"
#include "extensions/browser/process_manager.h"
#include "extensions/browser/process_manager_factory.h"
#include "extensions/common/extension.h"
#include "extensions/common/permissions/permissions_data.h"

using extensions::Extension;

namespace components_extensions {

ChromeProcessManagerDelegate::ChromeProcessManagerDelegate() {
}

ChromeProcessManagerDelegate::~ChromeProcessManagerDelegate() {
}

bool ChromeProcessManagerDelegate::AreBackgroundPagesAllowedForContext(
    content::BrowserContext* context) const {
  return true;
}

bool ChromeProcessManagerDelegate::IsExtensionBackgroundPageAllowed(
    content::BrowserContext* context,
    const Extension& extension) const {
  return AreBackgroundPagesAllowedForContext(context);
}

bool ChromeProcessManagerDelegate::DeferCreatingStartupBackgroundHosts(
    content::BrowserContext* context) const {
  return true;
}

}  // namespace components_extensions
