// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/unpacked_installer.h"

#include "base/test/scoped_feature_list.h"
#include "components/extensions/browser/extension_service_test_base.h"
#include "components/crx_file/id_util.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/permissions_manager.h"
#include "extensions/browser/test_extension_registry_observer.h"
#include "extensions/common/extension_features.h"

namespace components_extensions {

using extensions::Extension;
using extensions::PermissionsManager;
using extensions::TestExtensionRegistryObserver;

class UnpackedInstallerUnitTest : public ExtensionServiceTestBase,
                                  public testing::WithParamInterface<bool> {
 public:
  UnpackedInstallerUnitTest() {
    if (GetParam()) {
      feature_list_.InitAndEnableFeature(
          extensions_features::kAllowWithholdingExtensionPermissionsOnInstall);
    } else {
      feature_list_.InitAndDisableFeature(
          extensions_features::kAllowWithholdingExtensionPermissionsOnInstall);
    }
  }
  ~UnpackedInstallerUnitTest() override = default;

 private:
  base::test::ScopedFeatureList feature_list_;
};

// Tests host permissions are withheld by default at installation when flag is
// enabled.
TEST_P(UnpackedInstallerUnitTest, WithheldHostPermissionsWithFlag) {
  InitializeEmptyExtensionService();

  // Load extension.
  TestExtensionRegistryObserver observer(registry());
  base::FilePath extension_path =
      data_dir().AppendASCII("api_test/simple_all_urls");
  UnpackedInstaller::Create(service())->Load(extension_path);
  scoped_refptr<const Extension> loaded_extension =
      observer.WaitForExtensionLoaded();

  // Verify extension was installed.
  EXPECT_EQ(loaded_extension->name(), "All Urls Extension");

  // Host permissions should be withheld at installation only when flag is
  // enabled.
  bool flag_enabled = GetParam();
  PermissionsManager* permissions_manager =
      PermissionsManager::Get(browser_context());
  EXPECT_EQ(permissions_manager->HasWithheldHostPermissions(*loaded_extension),
            flag_enabled);
}

INSTANTIATE_TEST_SUITE_P(All, UnpackedInstallerUnitTest, testing::Bool());

}  // namespace components_extensions
