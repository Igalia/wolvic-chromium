// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_TEST_EXTENSION_SYSTEM_H_
#define COMPONENTS_EXTENSIONS_BROWSER_TEST_EXTENSION_SYSTEM_H_

#include <memory>

#include "base/memory/raw_ptr.h"
#include "base/one_shot_event.h"
#include "extensions/browser/content_verifier/content_verifier.h"
#include "extensions/browser/extension_system.h"
#include "services/data_decoder/public/cpp/test_support/in_process_data_decoder.h"

namespace base {
class CommandLine;
class FilePath;
}

namespace content {
class BrowserContext;
}

namespace value_store {
class TestingValueStore;
class TestValueStoreFactory;
}  // namespace value_store

namespace components_extensions {

// Test ExtensionSystem, for use with TestingBrowserContext.
class TestExtensionSystem : public extensions::ExtensionSystem {
 public:
  using InstallUpdateCallback = extensions::ExtensionSystem::InstallUpdateCallback;
  explicit TestExtensionSystem(content::BrowserContext* browser_context);
  ~TestExtensionSystem() override;

  // KeyedService implementation.
  void Shutdown() override;

  // Creates an ExtensionService initialized with the testing browser context and
  // returns it, and creates ExtensionPrefs if it hasn't been created yet.
  ExtensionService* CreateExtensionService(
      const base::CommandLine* command_line,
      const base::FilePath& install_directory,
      bool autoupdate_enabled,
      bool enable_extensions = true);

  // Similar to the above, but also allows specifying unpacked install directory
  // if needed.
  ExtensionService* CreateExtensionService(
      const base::CommandLine* command_line,
      const base::FilePath& install_directory,
      const base::FilePath& unpacked_install_directory,
      bool autoupdate_enabled,
      bool enable_extensions = true);

  void CreateSocketManager();

  // Creates a UserScriptManager initialized with the testing profile,
  void CreateUserScriptManager();

  void InitForRegularProfile(bool extensions_enabled) override {}
  void SetExtensionService(ExtensionService* service);
  ExtensionService* extension_service() override;
  extensions::ManagementPolicy* management_policy() override;
  extensions::ServiceWorkerManager* service_worker_manager() override;
  extensions::UserScriptManager* user_script_manager() override;
  extensions::StateStore* state_store() override;
  extensions::StateStore* rules_store() override;
  extensions::StateStore* dynamic_user_scripts_store() override;
  scoped_refptr<value_store::ValueStoreFactory> store_factory() override;
  value_store::TestingValueStore* value_store();
  extensions::QuotaService* quota_service() override;
  extensions::AppSorting* app_sorting() override;
  const base::OneShotEvent& ready() const override;
  bool is_ready() const override;
  extensions::ContentVerifier* content_verifier() override;
  std::unique_ptr<extensions::ExtensionSet> GetDependentExtensions(
      const extensions::Extension* extension) override;
  void InstallUpdate(const std::string& extension_id,
                     const std::string& public_key,
                     const base::FilePath& temp_dir,
                     bool install_immediately,
                     InstallUpdateCallback install_update_callback) override;
  void PerformActionBasedOnOmahaAttributes(
      const std::string& extension_id,
      const base::Value::Dict& attributes) override;
  bool FinishDelayedInstallationIfReady(const std::string& extension_id,
                                        bool install_immediately) override;

  // Note that you probably want to use base::RunLoop().RunUntilIdle() right
  // after this to run all the accumulated tasks.
  void SetReady() { ready_.Signal(); }

  // Factory method for tests to use with SetTestingBrowserContext.
  static std::unique_ptr<KeyedService> Build(content::BrowserContext* browser_context);

  // Used by ExtensionPrefsTest to re-create the AppSorting after it has
  // re-created the ExtensionPrefs instance (this can never happen in non-test
  // code).
  void RecreateAppSorting();

  void set_content_verifier(extensions::ContentVerifier* verifier) {
    content_verifier_ = verifier;
  }

 protected:
  raw_ptr<content::BrowserContext> browser_context_;

 private:
  scoped_refptr<value_store::TestValueStoreFactory> store_factory_;
  // This depends on store_factory_.
  std::unique_ptr<extensions::StateStore> state_store_;
  std::unique_ptr<extensions::ManagementPolicy> management_policy_;
  std::unique_ptr<ExtensionService> extension_service_;
  std::unique_ptr<extensions::QuotaService> quota_service_;
  std::unique_ptr<extensions::AppSorting> app_sorting_;
  std::unique_ptr<extensions::UserScriptManager> user_script_manager_;
  base::OneShotEvent ready_;

  std::unique_ptr<data_decoder::test::InProcessDataDecoder>
      in_process_data_decoder_;

  scoped_refptr<extensions::ContentVerifier> content_verifier_;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_TEST_EXTENSION_SYSTEM_H_
