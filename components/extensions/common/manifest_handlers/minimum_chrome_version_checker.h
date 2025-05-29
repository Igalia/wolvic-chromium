// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_COMMON_MANIFEST_HANDLERS_MINIMUM_CHROME_VERSION_CHECKER_H_
#define COMPONENTS_EXTENSIONS_COMMON_MANIFEST_HANDLERS_MINIMUM_CHROME_VERSION_CHECKER_H_

#include "extensions/common/manifest_handler.h"

namespace components_extensions {

// Checks that the "minimum_chrome_version" requirement is met.
class MinimumChromeVersionChecker : public extensions::ManifestHandler {
 public:
  MinimumChromeVersionChecker();

  MinimumChromeVersionChecker(const MinimumChromeVersionChecker&) = delete;
  MinimumChromeVersionChecker& operator=(const MinimumChromeVersionChecker&) =
      delete;

  ~MinimumChromeVersionChecker() override;

  // Validate minimum Chrome version. We don't need to store this, since the
  // extension is not valid if it is incorrect.
  bool Parse(extensions::Extension* extension, std::u16string* error) override;

 private:
  base::span<const char* const> Keys() const override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_COMMON_MANIFEST_HANDLERS_MINIMUM_CHROME_VERSION_CHECKER_H_
