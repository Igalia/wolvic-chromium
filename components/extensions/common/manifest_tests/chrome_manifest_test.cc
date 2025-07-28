// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/common/manifest_tests/chrome_manifest_test.h"

#include "base/files/file_path.h"
#include "base/path_service.h"
#include "components/extensions/test/test_extension_paths.h"
#include "components/version_info/version_info.h"

ChromeManifestTest::ChromeManifestTest()
    // CHANNEL_UNKNOWN == trunk.
    : current_channel_(version_info::Channel::UNKNOWN) {}

ChromeManifestTest::~ChromeManifestTest() {
}

base::FilePath ChromeManifestTest::GetTestDataDir() {
  base::FilePath path;
  base::PathService::Get(components_extensions::DIR_TEST_DATA, &path);
  return path.AppendASCII("extensions").AppendASCII("manifest_tests");
}
