// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_CHROME_PROCESS_MANAGER_DELEGATE_H_
#define COMPONENTS_EXTENSIONS_BROWSER_CHROME_PROCESS_MANAGER_DELEGATE_H_

#include "base/scoped_multi_source_observation.h"
#include "base/scoped_observation.h"
#include "extensions/browser/process_manager_delegate.h"


namespace components_extensions {

// Support for ProcessManager. Controls cases where Chrome wishes to disallow
// extension background pages or defer their creation.
// TODO(mshin): Support the multiple profiles
class ChromeProcessManagerDelegate : public extensions::ProcessManagerDelegate {
 public:
  ChromeProcessManagerDelegate();

  ChromeProcessManagerDelegate(const ChromeProcessManagerDelegate&) = delete;
  ChromeProcessManagerDelegate& operator=(const ChromeProcessManagerDelegate&) =
      delete;

  ~ChromeProcessManagerDelegate() override;

  // ProcessManagerDelegate:
  bool AreBackgroundPagesAllowedForContext(
      content::BrowserContext* context) const override;
  bool IsExtensionBackgroundPageAllowed(
      content::BrowserContext* context,
      const extensions::Extension& extension) const override;
  bool DeferCreatingStartupBackgroundHosts(
      content::BrowserContext* context) const override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_CHROME_PROCESS_MANAGER_DELEGATE_H_
