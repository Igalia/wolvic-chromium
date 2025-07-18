// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_TEST_TEST_EXTENSION_ENVIRONMENT_H_
#define COMPONENTS_EXTENSIONS_TEST_TEST_EXTENSION_ENVIRONMENT_H_

#include <memory>
#include <string>

#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "build/build_config.h"
#include "content/public/test/test_browser_context.h"
#include "extensions/common/extension.h"

#if BUILDFLAG(IS_WIN)
#include "ui/base/win/scoped_ole_initializer.h"
#endif

class PrefService;

namespace base {
class Value;
}

namespace content {
class BrowserTaskEnvironment;
class WebContents;
}

namespace extensions {
class Extension;
class ExtensionPrefs;
}

namespace components_extensions {

class ExtensionService;
class TestExtensionSystem;

// This class provides a minimal environment in which to create
// extensions and tabs for extension-related unittests.
class TestExtensionEnvironment {
 public:
  // Fetches the TestExtensionSystem in |browser_context| and creates a default
  // ExtensionService there,
  static ExtensionService* CreateExtensionServiceForBrowserContext(
      content::TestBrowserContext* browser_context);

  enum class Type {
    // A TestExtensionEnvironment which will provide a BrowserTaskEnvironment
    // in its scope.
    kWithTaskEnvironment,
    // A TestExtensionEnvironment which will run on top of the existing task
    // environment without trying to provide one.
    kInheritExistingTaskEnvironment,
  };

  enum class ProfileCreationType {
    kNoCreate,
    kCreate,
  };

  static TestExtensionEnvironment* GetInstance();

  explicit TestExtensionEnvironment(
      Type type = Type::kWithTaskEnvironment,
      ProfileCreationType profile_creation_type = ProfileCreationType::kCreate
  );

  TestExtensionEnvironment(const TestExtensionEnvironment&) = delete;
  TestExtensionEnvironment& operator=(const TestExtensionEnvironment&) = delete;

  ~TestExtensionEnvironment();

  void SetBrowserContext(content::TestBrowserContext* browser_context);

  content::TestBrowserContext* browser_context() const;

  // Returns the TestExtensionSystem created by the TestBrowserContext.
  TestExtensionSystem* GetExtensionSystem();

  // Returns an ExtensionService created (and owned) by the
  // TestExtensionSystem created by the TestBrowserContext.
  ExtensionService* GetExtensionService();

  // Returns ExtensionPrefs created (and owned) by the
  // TestExtensionSystem created by the TestBrowserContext.
  extensions::ExtensionPrefs* GetExtensionPrefs();

  // Creates an Extension and registers it with the ExtensionService.
  // The Extension has a default manifest of {name: "Extension",
  // version: "1.0", manifest_version: 2}, and values in
  // manifest_extra override these defaults.
  const extensions::Extension*
  MakeExtension(const base::Value::Dict& manifest_extra);

  // Use a specific extension ID instead of the default generated in
  // Extension::Create.
  const extensions::Extension*
  MakeExtension(const base::Value::Dict& manifest_extra,
                const std::string& id);

  // Generates a valid packaged app manifest with the given ID. If |install|
  // it gets added to the ExtensionService in |profile|.
  scoped_refptr<const extensions::Extension>
  MakePackagedApp(const std::string& id, bool install);

  // Returns a test web contents that has a tab id.
  std::unique_ptr<content::WebContents> MakeTab() const;

  // Deletes the testing browser context to test browser context teardown.
  void DeleteBrowserContext();

  PrefService* GetPrefService() const { return user_pref_service_.get(); }

 private:
  void Init();

  // If |task_environment_| is needed, then it needs to constructed before
  // |browser_context_| and destroyed after |browser_context_|.
  const std::unique_ptr<content::BrowserTaskEnvironment> task_environment_;

#if BUILDFLAG(IS_WIN)
  ui::ScopedOleInitializer ole_initializer_;
#endif

  // TestBrowserContext may be created or not, depending on the caller's
  // configuration passed to the constructor. This member keeps the ownership
  // if the mode is kCreate.
  std::unique_ptr<content::TestBrowserContext> browser_context_;

  // Unowned pointer of Profile for this test environment. May be the pointer
  // to `browser_context_`, or may be injected by SetProfile().
  raw_ptr<content::TestBrowserContext, DanglingUntriaged> browser_context_ptr_;

  std::unique_ptr<PrefService> local_state_;
  std::unique_ptr<PrefService> user_pref_service_;

  raw_ptr<ExtensionService, DanglingUntriaged> extension_service_ = nullptr;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_TEST_TEST_EXTENSION_ENVIRONMENT_H_
