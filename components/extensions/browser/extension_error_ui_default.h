// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_ERROR_UI_DEFAULT_H_
#define COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_ERROR_UI_DEFAULT_H_

#include <memory>

#include "base/memory/raw_ptr.h"
#include "components/extensions/browser/extension_error_ui.h"

namespace extensions {
class ManagementPolicy;
}

namespace components_extensions {

class ExtensionErrorUIDefault : public ExtensionErrorUI {
 public:
  explicit ExtensionErrorUIDefault(ExtensionErrorUI::Delegate* delegate);

  ExtensionErrorUIDefault(const ExtensionErrorUIDefault&) = delete;
  ExtensionErrorUIDefault& operator=(const ExtensionErrorUIDefault&) = delete;

  ~ExtensionErrorUIDefault() override;

  bool ShowErrorInBubbleView() override;
  void ShowExtensions() override;
  void Close() override;

  // TODO(mshin): Support GlobalError UI
  // GlobalErrorWithStandardBubble* GetErrorForTesting();
  void SetManagementPolicyForTesting(extensions::ManagementPolicy* management_policy);

 private:
  // The profile associated with this error.
  raw_ptr<content::BrowserContext> browser_context_ = nullptr;

  // TODO(mshin): Support GlobalError UI
  // std::unique_ptr<ExtensionGlobalError> global_error_;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_ERROR_UI_DEFAULT_H_
