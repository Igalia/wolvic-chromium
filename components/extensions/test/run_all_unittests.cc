// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "base/base_paths.h"
#include "base/functional/bind.h"
#include "base/path_service.h"
#include "base/test/launcher/unit_test_launcher.h"
#include "base/test/test_io_thread.h"
#include "build/buildflag.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/content_client.h"
#include "content/public/common/content_paths.h"
#include "content/public/test/content_test_suite_base.h"
#include "content/public/test/unittest_test_suite.h"
#include "components/extensions/browser/chrome_extensions_browser_client.h"
#include "components/extensions/browser/event_router_forwarder.h"
#include "components/extensions/common/initialize_extensions_client.h"
#include "components/extensions/test/test_extension_environment.h"
#include "components/extensions/test/test_extension_paths.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension_paths.h"
#include "extensions/test/test_extensions_client.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/resource/resource_scale_factor.h"
#include "ui/base/ui_base_paths.h"
#include "ui/gl/test/gl_surface_test_support.h"
#include "url/url_util.h"

namespace {

using components_extensions::ChromeExtensionsBrowserClient;
using components_extensions::EventRouterForwarder;
using components_extensions::TestExtensionEnvironment;
using extensions::ExtensionsBrowserClient;

class ExtensionsUnitTestSuiteInitializer
   : public testing::EmptyTestEventListener,
     public ChromeExtensionsBrowserClient::Delegate {
 public:
  ExtensionsUnitTestSuiteInitializer() = default;
  ExtensionsUnitTestSuiteInitializer(const ExtensionsUnitTestSuiteInitializer&) =
      delete;
  ExtensionsUnitTestSuiteInitializer& operator=(
      const ExtensionsUnitTestSuiteInitializer&) = delete;
  ~ExtensionsUnitTestSuiteInitializer() override = default;

  // Implement ChromeExtensionsBrowserClient::Delegate
  EventRouterForwarder* extension_event_router_forwarder() override {
    return extension_event_router_forwarder_.get();  
  }

  network::mojom::NetworkContext* GetNetworkContext() override {
    return TestExtensionEnvironment::GetInstance()->
        browser_context()->GetDefaultStoragePartition()->GetNetworkContext();
  }

  bool IsShuttingDown() override {
    return TestExtensionEnvironment::GetInstance()->IsShuttingDown();
  }

  std::string GetApplicationLocale() override { return "en-US"; }

  std::vector<content::BrowserContext*> GetAllBrowserContexts() override {
    return TestExtensionEnvironment::GetInstance()->GetAllBrowserContexts();
  }

  content::BrowserContext*
  GetOriginalBrowserContext(content::BrowserContext* context) override {
    return context;
  }

  PrefService*
  GetPrefServiceForContext(content::BrowserContext* context) override {
    return TestExtensionEnvironment::GetInstance()->GetPrefService();
  }

  void OnTestStart(const testing::TestInfo& test_info) override {
    extension_event_router_forwarder_ =
        base::MakeRefCounted<EventRouterForwarder>();

    components_extensions::EnsureExtensionsClientInitialized();

    extensions_browser_client_ =
        std::make_unique<ChromeExtensionsBrowserClient>(this);
    ExtensionsBrowserClient::Set(extensions_browser_client_.get());
  }

  void OnTestEnd(const testing::TestInfo& test_info) override {
    // TestingBrowserProcess::TearDownAndDeleteInstance();
    extensions_browser_client_.reset();
    ExtensionsBrowserClient::Set(nullptr);
  }

 private:
  std::unique_ptr<ChromeExtensionsBrowserClient> extensions_browser_client_;
  scoped_refptr<EventRouterForwarder> extension_event_router_forwarder_;
};

// Content client that exists only to register chrome-extension:// scheme with
// the url module.
// TODO(jamescook): Should this be merged with ShellContentClient? Should this
// be a persistent object available to tests?
class ExtensionsContentClient : public content::ContentClient {
 public:
  ExtensionsContentClient() = default;
  ExtensionsContentClient(const ExtensionsContentClient&) = delete;
  ExtensionsContentClient& operator=(const ExtensionsContentClient&) = delete;
  ~ExtensionsContentClient() override = default;

  // content::ContentClient overrides:
  void AddAdditionalSchemes(Schemes* schemes) override {
    schemes->standard_schemes.push_back(extensions::kExtensionScheme);
    schemes->savable_schemes.push_back(extensions::kExtensionScheme);
  }
};

// The test suite for extensions_unittests.
class ExtensionsTestSuite : public content::ContentTestSuiteBase {
 public:
  ExtensionsTestSuite(int argc, char** argv);
  ExtensionsTestSuite(const ExtensionsTestSuite&) = delete;
  ExtensionsTestSuite& operator=(const ExtensionsTestSuite&) = delete;
  ~ExtensionsTestSuite() override;

 private:
  // base::TestSuite:
  void Initialize() override;
  void Shutdown() override;
};

ExtensionsTestSuite::ExtensionsTestSuite(int argc, char** argv)
    : content::ContentTestSuiteBase(argc, argv) {}

ExtensionsTestSuite::~ExtensionsTestSuite() = default;

void ExtensionsTestSuite::Initialize() {
  testing::TestEventListeners& listeners =
      testing::UnitTest::GetInstance()->listeners();
  listeners.Append(new ExtensionsUnitTestSuiteInitializer);

  content::ContentTestSuiteBase::Initialize();
  gl::GLSurfaceTestSupport::InitializeOneOff();

  // Register the chrome-extension:// scheme via this circuitous path.
  {
    ExtensionsContentClient content_client;
    RegisterContentSchemes(&content_client);
  }
  RegisterInProcessThreads();

  components_extensions::RegisterPathProvider();
  content::RegisterPathProvider();
  extensions::RegisterPathProvider();
  ui::RegisterPathProvider();

  base::FilePath ui_test_pak_path;
  ASSERT_TRUE(base::PathService::Get(ui::UI_TEST_PAK, &ui_test_pak_path));
  ui::ResourceBundle::InitSharedInstanceWithPakPath(ui_test_pak_path);

  base::FilePath pak_path;
#if BUILDFLAG(IS_ANDROID)
  base::PathService::Get(ui::DIR_RESOURCE_PAKS_ANDROID, &pak_path);
#else
  base::PathService::Get(base::DIR_ASSETS, &pak_path);
#endif

  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      pak_path.AppendASCII("extensions_tests_resources.pak"),
      ui::kScaleFactorNone);
}

void ExtensionsTestSuite::Shutdown() {
  ui::ResourceBundle::CleanupSharedInstance();
  content::ContentTestSuiteBase::Shutdown();
}

}  // namespace

int main(int argc, char** argv) {
  content::UnitTestTestSuite test_suite(
      new ExtensionsTestSuite(argc, argv),
      base::BindRepeating(
          content::UnitTestTestSuite::CreateTestContentClients));
  return base::LaunchUnitTests(argc, argv,
                               base::BindOnce(&content::UnitTestTestSuite::Run,
                                              base::Unretained(&test_suite)));
}
