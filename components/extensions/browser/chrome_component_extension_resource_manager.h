// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_CHROME_COMPONENT_EXTENSION_RESOURCE_MANAGER_H_
#define COMPONENTS_EXTENSIONS_BROWSER_CHROME_COMPONENT_EXTENSION_RESOURCE_MANAGER_H_

#include <memory>

#include "extensions/browser/component_extension_resource_manager.h"
#include "extensions/common/extension_id.h"

namespace components_extensions {

class ChromeComponentExtensionResourceManager
    : public extensions::ComponentExtensionResourceManager {
 public:
  ChromeComponentExtensionResourceManager();

  ChromeComponentExtensionResourceManager(
      const ChromeComponentExtensionResourceManager&) = delete;
  ChromeComponentExtensionResourceManager& operator=(
      const ChromeComponentExtensionResourceManager&) = delete;

  ~ChromeComponentExtensionResourceManager() override;

  // Overridden from ComponentExtensionResourceManager:
  bool IsComponentExtensionResource(const base::FilePath& extension_path,
                                    const base::FilePath& resource_path,
                                    int* resource_id) const override;
  const ui::TemplateReplacements* GetTemplateReplacementsForExtension(
      const extensions::ExtensionId& extension_id) const override;

 private:
  class Data;

  void LazyInitData() const;

  // Logically const. Initialized on demand to keep browser start-up fast.
  mutable std::unique_ptr<const Data> data_;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_CHROME_COMPONENT_EXTENSION_RESOURCE_MANAGER_H_
