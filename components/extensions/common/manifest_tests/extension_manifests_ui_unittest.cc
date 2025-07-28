// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/common/manifest_tests/chrome_manifest_test.h"
#include "extensions/common/manifest_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace components_extensions {

using UIManifestTest = ChromeManifestTest;
namespace manifest_errors = extensions::manifest_errors;

TEST_F(UIManifestTest, DisallowMultipleUISurfaces) {
  LoadAndExpectError("multiple_ui_surfaces.json",
                     manifest_errors::kOneUISurfaceOnly);
}

}  // namespace components_extensions
