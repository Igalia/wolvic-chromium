// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/renderer/chrome_extensions_dispatcher_delegate.h"

#include "base/command_line.h"
#include "content/public/common/content_switches.h"

namespace components_extensions {

ChromeExtensionsDispatcherDelegate::ChromeExtensionsDispatcherDelegate() {}

ChromeExtensionsDispatcherDelegate::~ChromeExtensionsDispatcherDelegate() {}

void ChromeExtensionsDispatcherDelegate::OnActiveExtensionsUpdated(
    const std::set<std::string>& extension_ids) {
  // In single-process mode, the browser process reports the active extensions.
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          ::switches::kSingleProcess))
    return;
  // TODO(mshin): Enable below code after supporting Crash Report
  // crash_keys::SetActiveExtensions(extension_ids);
}

}  // components_extensions
