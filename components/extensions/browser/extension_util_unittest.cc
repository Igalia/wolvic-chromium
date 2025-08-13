// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/extension_util.h"

#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "base/strings/strcat.h"
#include "build/chromeos_buildflags.h"
#include "components/extensions/browser/chrome_test_extension_loader.h"
#include "components/extensions/browser/extension_service.h"
#include "components/extensions/browser/extension_service_test_base.h"
#include "components/policy/core/common/mock_configuration_policy_provider.h"
#include "components/policy/core/common/policy_service_impl.h"
#include "components/sessions/content/session_tab_helper.h"
#include "components/sync_preferences/testing_pref_service_syncable.h"
#include "content/public/test/web_contents_tester.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_util.h"
#include "extensions/browser/pref_names.h"
#include "extensions/browser/test_extension_registry_observer.h"
#include "extensions/common/extension_builder.h"
#include "extensions/common/mojom/manifest.mojom-shared.h"
#include "extensions/common/permissions/permissions_data.h"
#include "extensions/test/test_extension_dir.h"
#include "url/gurl.h"

namespace components_extensions {

namespace disable_reason = extensions::disable_reason;

using extensions::CaptureRequirement;
using extensions::Extension;
using extensions::ExtensionBuilder;
using extensions::TestExtensionDir;
using extensions::TestExtensionRegistryObserver;

namespace {

#if BUILDFLAG(IS_CHROMEOS_ASH)
constexpr char kExtensionUpdateUrl[] =
    "https://clients2.google.com/service/update2/crx";  // URL of Chrome Web
                                                        // Store backend.
#endif

}  // namespace

class ExtensionUtilUnittest : public ExtensionServiceTestBase {
 public:
  void SetUp() override { InitializeEmptyExtensionService(); }
};

TEST_F(ExtensionUtilUnittest, SetAllowFileAccess) {
  constexpr char kManifest[] =
      R"({
           "name": "foo",
           "version": "1.0",
           "manifest_version": 2,
           "permissions": ["<all_urls>"]
         })";

  TestExtensionDir dir;
  dir.WriteManifest(kManifest);

  ChromeTestExtensionLoader loader(browser_context());
  // An unpacked extension would get file access by default, so disabled it on
  // the loader.
  loader.set_allow_file_access(false);

  scoped_refptr<const Extension> extension =
      loader.LoadExtension(dir.UnpackedPath());
  const std::string extension_id = extension->id();

  GURL file_url("file://etc");
  std::unique_ptr<content::WebContents> web_contents(
      content::WebContentsTester::CreateTestWebContents(browser_context(), nullptr));
  int tab_id = sessions::SessionTabHelper::IdForTab(web_contents.get()).id();

  // Initially the file access pref will be false and the extension will not be
  // able to capture a file URL page.
  EXPECT_FALSE(extensions::util::AllowFileAccess(extension_id, browser_context()));
  EXPECT_FALSE(extension->permissions_data()->CanCaptureVisiblePage(
      file_url, tab_id, nullptr, CaptureRequirement::kActiveTabOrAllUrls));

  // Calling SetAllowFileAccess should reload the extension with file access.
  {
    TestExtensionRegistryObserver observer(registry(), extension_id);
    util::SetAllowFileAccess(extension_id, browser_context(), true);
    extension = observer.WaitForExtensionInstalled();
  }

  EXPECT_TRUE(extensions::util::AllowFileAccess(extension_id, browser_context()));
  EXPECT_TRUE(extension->permissions_data()->CanCaptureVisiblePage(
      file_url, tab_id, nullptr, CaptureRequirement::kActiveTabOrAllUrls));

  // Removing the file access should reload the extension again back to not
  // having file access.
  {
    TestExtensionRegistryObserver observer(registry(), extension_id);
    util::SetAllowFileAccess(extension_id, browser_context(), false);
    extension = observer.WaitForExtensionInstalled();
  }

  EXPECT_FALSE(extensions::util::AllowFileAccess(extension_id, browser_context()));
  EXPECT_FALSE(extension->permissions_data()->CanCaptureVisiblePage(
      file_url, tab_id, nullptr, CaptureRequirement::kActiveTabOrAllUrls));
}

TEST_F(ExtensionUtilUnittest, SetAllowFileAccessWhileDisabled) {
  constexpr char kManifest[] =
      R"({
           "name": "foo",
           "version": "1.0",
           "manifest_version": 2,
           "permissions": ["<all_urls>"]
         })";

  TestExtensionDir dir;
  dir.WriteManifest(kManifest);

  ChromeTestExtensionLoader loader(browser_context());
  // An unpacked extension would get file access by default, so disabled it on
  // the loader.
  loader.set_allow_file_access(false);

  scoped_refptr<const Extension> extension =
      loader.LoadExtension(dir.UnpackedPath());
  const std::string extension_id = extension->id();

  GURL file_url("file://etc");
  std::unique_ptr<content::WebContents> web_contents(
      content::WebContentsTester::CreateTestWebContents(browser_context(), nullptr));
  int tab_id = sessions::SessionTabHelper::IdForTab(web_contents.get()).id();

  // Initially the file access pref will be false and the extension will not be
  // able to capture a file URL page.
  EXPECT_FALSE(extensions::util::AllowFileAccess(extension_id, browser_context()));
  EXPECT_FALSE(extension->permissions_data()->CanCaptureVisiblePage(
      file_url, tab_id, nullptr, CaptureRequirement::kActiveTabOrAllUrls));

  // Disabling the extension then calling SetAllowFileAccess should reload the
  // extension with file access.
  service()->DisableExtension(extension_id,
                              disable_reason::DISABLE_USER_ACTION);
  {
    TestExtensionRegistryObserver observer(registry(), extension_id);
    util::SetAllowFileAccess(extension_id, browser_context(), true);
    extension = observer.WaitForExtensionInstalled();
  }
  // The extension should still be disabled.
  EXPECT_FALSE(service()->IsExtensionEnabled(extension_id));

  service()->EnableExtension(extension_id);
  EXPECT_TRUE(extensions::util::AllowFileAccess(extension_id, browser_context()));
  EXPECT_TRUE(extension->permissions_data()->CanCaptureVisiblePage(
      file_url, tab_id, nullptr, CaptureRequirement::kActiveTabOrAllUrls));

  // Disabling the extension and then removing the file access should reload it
  // again back to not having file access. Regression test for
  // crbug.com/1385343.
  service()->DisableExtension(extension_id,
                              disable_reason::DISABLE_USER_ACTION);
  {
    TestExtensionRegistryObserver observer(registry(), extension_id);
    util::SetAllowFileAccess(extension_id, browser_context(), false);
    extension = observer.WaitForExtensionInstalled();
  }
  // The extension should still be disabled.
  EXPECT_FALSE(service()->IsExtensionEnabled(extension_id));

  service()->EnableExtension(extension_id);
  EXPECT_FALSE(extensions::util::AllowFileAccess(extension_id, browser_context()));
  EXPECT_FALSE(extension->permissions_data()->CanCaptureVisiblePage(
      file_url, tab_id, nullptr, CaptureRequirement::kActiveTabOrAllUrls));
}

TEST_F(ExtensionUtilUnittest, HasIsolatedStorage) {
  // Platform apps should have isolated storage.
  scoped_refptr<const Extension> app =
      ExtensionBuilder("foo_app", ExtensionBuilder::Type::PLATFORM_APP).Build();
  EXPECT_TRUE(app->is_platform_app());
  EXPECT_TRUE(util::HasIsolatedStorage(*app.get(), browser_context()));

  // Extensions should not have isolated storage.
  scoped_refptr<const Extension> extension =
      ExtensionBuilder("foo_ext").Build();
  EXPECT_FALSE(extension->is_platform_app());
  EXPECT_FALSE(util::HasIsolatedStorage(*extension.get(), browser_context()));
}

}  // namespace components_extensions
