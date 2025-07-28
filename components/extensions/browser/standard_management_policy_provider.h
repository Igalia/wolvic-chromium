// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_STANDARD_MANAGEMENT_POLICY_PROVIDER_H_
#define COMPONENTS_EXTENSIONS_BROWSER_STANDARD_MANAGEMENT_POLICY_PROVIDER_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "extensions/browser/management_policy.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace extensions {
class Extension;
}

namespace components_extensions {

class ExtensionManagement;

// The standard management policy provider, which takes into account the
// extension block/allowlists and admin block/allowlists.
class StandardManagementPolicyProvider : public extensions::ManagementPolicy::Provider {
 public:
  explicit StandardManagementPolicyProvider(ExtensionManagement* settings,
                                            content::BrowserContext* context);

  ~StandardManagementPolicyProvider() override;

  // ManagementPolicy::Provider implementation.
  std::string GetDebugPolicyProviderName() const override;
  bool UserMayLoad(const extensions::Extension* extension,
                   std::u16string* error) const override;
  bool UserMayInstall(const extensions::Extension* extension,
                      std::u16string* error) const override;
  bool UserMayModifySettings(const extensions::Extension* extension,
                             std::u16string* error) const override;
  bool ExtensionMayModifySettings(const extensions::Extension* source_extension,
                                  const extensions::Extension* extension,
                                  std::u16string* error) const override;
  bool MustRemainEnabled(const extensions::Extension* extension,
                         std::u16string* error) const override;
  bool MustRemainDisabled(const extensions::Extension* extension,
                          extensions::disable_reason::DisableReason* reason,
                          std::u16string* error) const override;
  bool MustRemainInstalled(const extensions::Extension* extension,
                           std::u16string* error) const override;
  bool ShouldForceUninstall(const extensions::Extension* extension,
                            std::u16string* error) const override;

 private:
  raw_ptr<content::BrowserContext> browser_context_;
  raw_ptr<ExtensionManagement> settings_;
  bool ReturnLoadError(const extensions::Extension* extension,
                       std::u16string* error) const;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_STANDARD_MANAGEMENT_POLICY_PROVIDER_H_
