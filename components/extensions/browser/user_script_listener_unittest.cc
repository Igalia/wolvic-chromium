// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/user_script_listener.h"

#include <memory>
#include <optional>

#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/json/json_file_value_serializer.h"
#include "base/memory/raw_ptr.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/threading/thread.h"
#include "components/extensions/browser/extension_service.h"
#include "components/extensions/browser/test_extension_system.h"
#include "components/extensions/browser/unpacked_installer.h"
#include "components/extensions/test/test_extension_environment.h"
#include "components/extensions/test/test_extension_paths.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/navigation_throttle.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/test/browser_task_environment.h"
#include "content/public/test/mock_navigation_handle.h"
#include "content/public/test/test_browser_context.h"
#include "content/public/test/test_renderer_host.h"
#include "content/public/test/test_utils.h"
#include "content/public/test/web_contents_tester.h"
#include "extensions/browser/api/scripting/scripting_utils.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/test_extension_registry_observer.h"
#include "extensions/common/url_pattern_set.h"
#include "testing/gtest/include/gtest/gtest.h"

using content::NavigationThrottle;

namespace components_extensions {

namespace mojom = extensions::mojom;
namespace scripting = extensions::scripting;

using extensions::Extension;
using extensions::ExtensionId;
using extensions::ExtensionRegistry;
using extensions::ExtensionSet;
using extensions::ExtensionSystem;
using extensions::TestExtensionRegistryObserver;
using extensions::UnloadedExtensionReason;
using extensions::URLPatternSet;

namespace {

const char kMatchingUrl[] = "http://google.com/";
const char kMatchingPrefsUrl[] = "http://prefs.com/";
const char kNotMatchingUrl[] = "http://example.com/";
const ExtensionId kTestExtensionId = "behllobkkfkfnphdnhnkndlbkcpglgmj";

// Yoinked from manifest_unittest.cc.
std::optional<base::Value::Dict> LoadManifestFile(const base::FilePath path,
                                                  std::string* error) {
  EXPECT_TRUE(base::PathExists(path));
  JSONFileValueDeserializer deserializer(path);
  std::unique_ptr<base::Value> manifest =
      deserializer.Deserialize(nullptr, error);
  if (!manifest || !manifest->is_dict()) {
    return std::nullopt;
  }
  return std::move(*manifest).TakeDict();
}

scoped_refptr<Extension> LoadExtension(const std::string& filename,
                                       std::string* error) {
  base::FilePath path;
  base::PathService::Get(DIR_TEST_DATA, &path);
  path = path.
      AppendASCII("extensions").
      AppendASCII("manifest_tests").
      AppendASCII(filename.c_str());
  std::optional<base::Value::Dict> manifest = LoadManifestFile(path, error);
  if (!manifest) {
    return nullptr;
  }
  return Extension::Create(path.DirName(), mojom::ManifestLocation::kUnpacked,
                           *manifest, Extension::NO_FLAGS, error);
}

}  // namespace

class UserScriptListenerTest : public testing::Test {
 public:
  UserScriptListenerTest()
      : task_environment_(content::BrowserTaskEnvironment::IO_MAINLOOP) {}

  ~UserScriptListenerTest() override {}

  UserScriptListenerTest(const UserScriptListenerTest&) = delete;
  UserScriptListenerTest& operator=(const UserScriptListenerTest&) = delete;

  void SetUp() override {
    browser_context_= std::make_unique<content::TestBrowserContext>();
    ASSERT_TRUE(browser_context_);
    env_ = std::make_unique<TestExtensionEnvironment>(browser_context_.get(), nullptr);
    env_->AddBrowserContext(browser_context_.get());

    listener_ = std::make_unique<UserScriptListener>();
    TestExtensionSystem* test_extension_system =
        static_cast<TestExtensionSystem*>(ExtensionSystem::Get(browser_context_.get()));
    service_ = test_extension_system->CreateExtensionService(
        base::CommandLine::ForCurrentProcess(), base::FilePath(), false);

    auto instance = content::SiteInstance::Create(browser_context_.get());
    instance->GetProcess()->Init();
    web_contents_ = content::WebContentsTester::CreateTestWebContents(
        browser_context_.get(), std::move(instance));
  }

  void TearDown() override {
    // The Listener unsubscribes itself from the profile in StartTearDown;
    // failure to unsubscribe will result in the profile_manager's destructor
    // throwing an error since there's still a subscription in the callback
    // list.
    listener_->StartTearDown();
  }

  void MarkNavigationResumed() { was_navigation_resumed_ = true; }

 protected:
  void LoadTestExtension() {
    base::FilePath test_dir;
    ASSERT_TRUE(base::PathService::Get(DIR_TEST_DATA, &test_dir));
    base::FilePath extension_path = test_dir.AppendASCII("extensions")
                                        .AppendASCII("good")
                                        .AppendASCII("Extensions")
                                        .AppendASCII(kTestExtensionId)
                                        .AppendASCII("1.0.0.0");
    TestExtensionRegistryObserver observer(ExtensionRegistry::Get(browser_context_.get()),
                                           kTestExtensionId);
    UnpackedInstaller::Create(service_)->Load(extension_path);
    observer.WaitForExtensionLoaded();
  }

  void UnloadTestExtension() {
    const ExtensionSet& extensions =
        ExtensionRegistry::Get(browser_context_.get())->enabled_extensions();
    ASSERT_FALSE(extensions.empty());
    service_->UnloadExtension((*extensions.begin())->id(),
                              UnloadedExtensionReason::DISABLE);
  }

  std::unique_ptr<NavigationThrottle> CreateListenerNavigationThrottle(
      content::NavigationHandle* handle) {
    std::unique_ptr<NavigationThrottle> throttle =
        listener_->CreateNavigationThrottle(handle);
    throttle->set_resume_callback_for_testing(
        base::BindRepeating(&UserScriptListenerTest::MarkNavigationResumed,
                            base::Unretained(this)));
    return throttle;
  }

  void AddPersistentScriptingURLPatternToPrefs() {
    URLPatternSet persistent_urls;
    persistent_urls.AddPattern(
        URLPattern(URLPattern::SCHEME_HTTP, kMatchingPrefsUrl));
    scripting::SetPersistentScriptURLPatterns(browser_context_.get(), kTestExtensionId,
                                              persistent_urls);
  }

  content::BrowserTaskEnvironment task_environment_;
  content::RenderViewHostTestEnabler rvh_test_enabler_;
  std::unique_ptr<UserScriptListener> listener_;
  std::unique_ptr<content::TestBrowserContext> browser_context_;
  std::unique_ptr<TestExtensionEnvironment> env_;
  raw_ptr<ExtensionService> service_ = nullptr;
  bool was_navigation_resumed_ = false;
  std::unique_ptr<content::WebContents> web_contents_;
};

namespace {

TEST_F(UserScriptListenerTest, DelayAndUpdate) {
  LoadTestExtension();

  content::MockNavigationHandle handle(GURL(kMatchingUrl),
                                       web_contents_->GetPrimaryMainFrame());
  std::unique_ptr<NavigationThrottle> throttle =
      CreateListenerNavigationThrottle(&handle);
  EXPECT_EQ(NavigationThrottle::DEFER, throttle->WillStartRequest());

  listener_->TriggerUserScriptsReadyForTesting(browser_context_.get());
  EXPECT_TRUE(was_navigation_resumed_);
}

// Test that requests matching URL patterns from persistent dynamic content
// scripts registered from previous sessions (stored inside prefs) are
// throttled.
TEST_F(UserScriptListenerTest, DelayForPersistentScriptPatterns) {
  AddPersistentScriptingURLPatternToPrefs();
  LoadTestExtension();

  content::MockNavigationHandle handle(GURL(kMatchingPrefsUrl),
                                       web_contents_->GetPrimaryMainFrame());

  std::unique_ptr<NavigationThrottle> throttle =
      CreateListenerNavigationThrottle(&handle);
  EXPECT_EQ(NavigationThrottle::DEFER, throttle->WillStartRequest());

  listener_->TriggerUserScriptsReadyForTesting(browser_context_.get());
  EXPECT_TRUE(was_navigation_resumed_);
}

TEST_F(UserScriptListenerTest, DelayAndUnload) {
  LoadTestExtension();

  content::MockNavigationHandle handle(GURL(kMatchingUrl),
                                       web_contents_->GetPrimaryMainFrame());
  std::unique_ptr<NavigationThrottle> throttle =
      CreateListenerNavigationThrottle(&handle);
  EXPECT_EQ(NavigationThrottle::DEFER, throttle->WillStartRequest());

  UnloadTestExtension();
  base::RunLoop().RunUntilIdle();

  // This is still not enough to start delayed requests. We have to notify the
  // listener that the user scripts have been updated.
  EXPECT_FALSE(was_navigation_resumed_);

  listener_->TriggerUserScriptsReadyForTesting(browser_context_.get());
  EXPECT_TRUE(was_navigation_resumed_);
}

TEST_F(UserScriptListenerTest, NoDelayNoExtension) {
  content::MockNavigationHandle handle(GURL(kMatchingUrl),
                                       web_contents_->GetPrimaryMainFrame());
  std::unique_ptr<NavigationThrottle> throttle =
      listener_->CreateNavigationThrottle(&handle);
  EXPECT_EQ(nullptr, throttle);
}

TEST_F(UserScriptListenerTest, NoDelayNotMatching) {
  AddPersistentScriptingURLPatternToPrefs();
  LoadTestExtension();

  content::MockNavigationHandle handle(GURL(kNotMatchingUrl),
                                       web_contents_->GetPrimaryMainFrame());
  std::unique_ptr<NavigationThrottle> throttle =
      listener_->CreateNavigationThrottle(&handle);
  EXPECT_EQ(nullptr, throttle);
}

// TODO(mshin): Enable the below test after supporting multiple profile
TEST_F(UserScriptListenerTest, DISABLED_MultiProfile) {
  LoadTestExtension();

  // Fire up a second profile and have it load an extension with a content
  // script.
  auto browser_context2 = std::make_unique<content::TestBrowserContext>();
  ASSERT_TRUE(browser_context2);

  std::string error;
  scoped_refptr<Extension> extension = LoadExtension(
      "content_script_yahoo.json", &error);
  ASSERT_TRUE(extension.get());

  ExtensionRegistry* registry = ExtensionRegistry::Get(browser_context2.get());
  registry->AddEnabled(extension);
  registry->TriggerOnLoaded(extension.get());

  content::MockNavigationHandle handle(GURL(kMatchingUrl),
                                       web_contents_->GetPrimaryMainFrame());
  std::unique_ptr<NavigationThrottle> throttle =
      CreateListenerNavigationThrottle(&handle);
  EXPECT_EQ(NavigationThrottle::DEFER, throttle->WillStartRequest());

  // When the first profile's user scripts are ready, the request should still
  // be blocked waiting for browser_context2.
  listener_->TriggerUserScriptsReadyForTesting(browser_context_.get());
  EXPECT_FALSE(was_navigation_resumed_);

  // After browser_context2 is ready, the request should proceed.
  listener_->TriggerUserScriptsReadyForTesting(browser_context2.get());
  EXPECT_TRUE(was_navigation_resumed_);
}

// Test when the user scripts ready trigger occurs before the throttle's
// WillStartRequest function is called.  This can occur when there are multiple
// throttles.
TEST_F(UserScriptListenerTest, ResumeBeforeStart) {
  LoadTestExtension();
  content::MockNavigationHandle handle(GURL(kMatchingUrl),
                                       web_contents_->GetPrimaryMainFrame());
  std::unique_ptr<NavigationThrottle> throttle =
      listener_->CreateNavigationThrottle(&handle);
  ASSERT_TRUE(throttle);

  listener_->TriggerUserScriptsReadyForTesting(browser_context_.get());

  ASSERT_EQ(content::NavigationThrottle::PROCEED, throttle->WillStartRequest());
}

}  // namespace

}  // namespace components_extensions
