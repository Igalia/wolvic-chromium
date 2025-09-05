// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_FORCED_EXTENSIONS_FORCE_INSTALLED_TEST_BASE_H_
#define COMPONENTS_EXTENSIONS_BROWSER_FORCED_EXTENSIONS_FORCE_INSTALLED_TEST_BASE_H_

#include "base/memory/raw_ptr.h"
#include "components/extensions/browser/forced_extensions/force_installed_tracker.h"
#include "components/extensions/test/test_extension_environment.h"
#include "components/policy/core/common/mock_configuration_policy_provider.h"
#include "content/public/test/browser_task_environment.h"
#include "content/public/test/test_browser_context.h"
#include "extensions/common/extension.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace sync_preferences {
class TestingPrefServiceSyncable;
}

namespace extensions {
class ExtensionRegistry;
}

namespace components_extensions {

class InstallStageTracker;

// This class is extended by tests to provide a setup for tracking installation
// of force extensions. It also provides helper functions for creating and
// setting ExtensionInstallForcelist policy value.
class ForceInstalledTestBase : public testing::Test {
 public:
  enum class ExtensionOrigin {
    kWebStore,
    kOffStore,
  };

  ForceInstalledTestBase();
  ~ForceInstalledTestBase() override;

  ForceInstalledTestBase(const ForceInstalledTestBase&) = delete;
  ForceInstalledTestBase& operator=(const ForceInstalledTestBase&) = delete;

 protected:
  void SetUp() override;

  // Creates and sets value for ExtensionInstallForcelist policy and
  // kInstallForceList preference. `source` tells whether the extensions
  // specified in the policy should have an update URL from CWS or not.
  void SetupForceList(ExtensionOrigin origin);

  // Creates and sets empty value for ExtensionInstallForcelist policy and
  // kInstallForceList preference.
  void SetupEmptyForceList();

  // Creates a new extension with `extension_id` and `extension_name` and fakes
  // its status by calling one of ForceInstalledTracker's
  // ExtensionRegistryObserver override.
  scoped_refptr<const extensions::Extension> CreateNewExtension(
      const std::string& extension_name,
      const std::string& extension_id,
      const ForceInstalledTracker::ExtensionStatus& status);

  content::BrowserContext* browser_context() const { return browser_context_.get(); }

  sync_preferences::TestingPrefServiceSyncable* prefs() const { return prefs_; }

  extensions::ExtensionRegistry* registry() const { return registry_; }

  InstallStageTracker* install_stage_tracker() const {
    return install_stage_tracker_;
  }

  ForceInstalledTracker* force_installed_tracker() const {
    return force_installed_tracker_.get();
  }

  static const char kExtensionId1[];
  static const char kExtensionId2[];
  static const char kExtensionName1[];
  static const char kExtensionName2[];
  static const char kExtensionUpdateUrl[];
  static const char kOffStoreUpdateUrl[];

  content::BrowserTaskEnvironment task_environment_{
      base::test::TaskEnvironment::TimeSource::MOCK_TIME};

 private:
  testing::NiceMock<policy::MockConfigurationPolicyProvider> policy_provider_;
  std::unique_ptr<content::TestBrowserContext> browser_context_;
  std::unique_ptr<TestExtensionEnvironment> env_;
  raw_ptr<sync_preferences::TestingPrefServiceSyncable> prefs_;
  raw_ptr<extensions::ExtensionRegistry> registry_;
  raw_ptr<InstallStageTracker> install_stage_tracker_;
  std::unique_ptr<ForceInstalledTracker> force_installed_tracker_;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_FORCED_EXTENSIONS_FORCE_INSTALLED_TEST_BASE_H_
