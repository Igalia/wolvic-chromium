// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/extension_service_test_base.h"

#include <utility>

#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/memory/ref_counted.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/task/single_thread_task_runner.h"
#include "base/test/task_environment.h"
#include "build/build_config.h"
#include "components/extensions/browser/chrome_extension_web_contents_observer.h"
#include "components/extensions/browser/component_loader.h"
#include "components/extensions/browser/crx_installer.h"
#include "components/extensions/browser/extension_service.h"
#include "components/extensions/browser/extension_system_factory.h"
#include "components/extensions/browser/load_error_reporter.h"
#include "components/extensions/browser/extension_service.h"
#include "components/extensions/browser/shared_module_service.h"
#include "components/extensions/browser/test_extension_system.h"
#include "components/extensions/test/test_extension_environment.h"
#include "components/extensions/test/test_extension_paths.h"
#include "components/crx_file/crx_verifier.h"
#include "components/policy/core/common/policy_service_impl.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/supervised_user/core/common/buildflags.h"
#include "components/sync_preferences/pref_service_mock_factory.h"
#include "components/sync_preferences/pref_service_syncable.h"
#include "components/sync_preferences/testing_pref_service_syncable.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/content_client.h"
#include "content/public/test/browser_task_environment.h"
#include "content/public/test/test_browser_context.h"
#include "extensions/browser/api/audio/audio_api.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/browser/extension_prefs_observer.h"
#include "extensions/browser/extension_pref_value_map_factory.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/permissions_manager.h"
#include "extensions/browser/pref_names.h"
#include "extensions/common/extensions_client.h"

namespace components_extensions {

using content::TestBrowserContext;
using extensions::AudioAPI;
using extensions::ExtensionsClient;
using extensions::ExtensionPrefs;
using extensions::ExtensionRegistry;
using extensions::ExtensionSystem;
using extensions::PermissionsManager;

namespace {

static constexpr char kPreferencesFilename[] = "Preferences";

// Create a testing browser context according to |params|.

std::tuple<std::unique_ptr<TestBrowserContext>,
          std::unique_ptr<TestExtensionEnvironment>>
BuildTestingBrowserContext(
    const ExtensionServiceTestBase::ExtensionServiceInitParams& params,
    base::ScopedTempDir& temp_dir,
    policy::PolicyService* policy_service) {
  if (!temp_dir.CreateUniqueTempDir()) {
    return {nullptr, nullptr};
  }

  base::FilePath profile_dir =
      temp_dir.GetPath().Append(FILE_PATH_LITERAL("TestingExtensionsPath"));
  if (base::File::Error error = base::File::FILE_OK;
      !base::CreateDirectoryAndGetError(profile_dir, &error)) {
    LOG(ERROR) << "Failed to create profile directory: " << error;
    return {nullptr, nullptr};
  }

  base::FilePath extensions_install_dir =
      profile_dir.AppendASCII(extensions::kInstallDirectoryName);
  if (!base::DeletePathRecursively(extensions_install_dir)) {
    LOG(ERROR) << "Failed to clean extensions directory";
    return {nullptr, nullptr};
  }
  if (params.extensions_dir.empty()) {
    if (base::File::Error error = base::File::FILE_OK;
        !base::CreateDirectoryAndGetError(extensions_install_dir, &error)) {
      LOG(ERROR) << "Failed to create extensions directory: " << error;
      return {nullptr, nullptr};
    }
  } else {
    if (!base::CopyDirectory(params.extensions_dir, extensions_install_dir,
                             true)) {
      LOG(ERROR) << "Failed to copy extensions directory";
      return {nullptr, nullptr};
    }
  }

  // Only perform cleanup and copying of unpacked extensions if the path exists
  // for the test since this is less common than for packed extensions.
  if (base::PathExists(params.unpacked_extensions_dir)) {
    base::FilePath unpacked_extensions_install_dir =
        profile_dir.AppendASCII(extensions::kUnpackedInstallDirectoryName);
    if (!base::DeletePathRecursively(unpacked_extensions_install_dir)) {
      LOG(ERROR) << "Failed to clean unpacked extensions directory";
      return {nullptr, nullptr};
    }
    if (params.unpacked_extensions_dir.empty()) {
      if (base::File::Error error = base::File::FILE_OK;
          !base::CreateDirectoryAndGetError(unpacked_extensions_install_dir,
                                            &error)) {
        LOG(ERROR) << "Failed to create unpacked extensions directory: "
                   << error;
        return {nullptr, nullptr};
      }
    } else {
      if (!base::CopyDirectory(params.unpacked_extensions_dir,
                               unpacked_extensions_install_dir, true)) {
        LOG(ERROR) << "Failed to copy unpacked extensions directory";
        return {nullptr, nullptr};
      }
    }
  }

// TODO(mshin): Support the multiple profiles
//   if (params.profile_is_supervised) {
// #if BUILDFLAG(ENABLE_SUPERVISED_USERS)
//     profile_builder.SetIsSupervisedProfile();
// #endif
//   }

//   if (params.profile_is_guest) {
//     profile_builder.SetGuestSession();
//   }

//   if (params.enable_bookmark_model) {
//     profile_builder.AddTestingFactory(
//         BookmarkModelFactory::GetInstance(),
//         BookmarkModelFactory::GetDefaultFactory());
//     profile_builder.AddTestingFactory(
//         ManagedBookmarkServiceFactory::GetInstance(),
//         ManagedBookmarkServiceFactory::GetDefaultFactory());
//   }

//   profile_builder.AddTestingFactory(
//       ChromeSigninClientFactory::GetInstance(),
//       base::BindRepeating(&signin::BuildTestSigninClient));
//   profile_builder.AddTestingFactories(
//       IdentityTestEnvironmentProfileAdaptor::
//           GetIdentityTestEnvironmentFactories());
//   // TODO(crbug.com/1222596): SyncService (and thus TrustedVaultService)
//   // instantiation can be scoped down to a few derived fixtures.
//   profile_builder.AddTestingFactory(
//       TrustedVaultServiceFactory::GetInstance(),
//       TrustedVaultServiceFactory::GetDefaultFactory());
//   profile_builder.AddTestingFactory(SyncServiceFactory::GetInstance(),
//                                     SyncServiceFactory::GetDefaultFactory());
//   profile_builder.AddTestingFactory(
//       ExtensionGarbageCollectorFactory::GetInstance(),
//       base::BindRepeating(&ExtensionGarbageCollectorFactory::BuildInstanceFor));

//  profile_builder.SetPath(profile_dir);
//  return profile_builder.Build();

  // If pref_file is empty, TestingPrefServiceSyncable is automatically created
  // in TestExtensionEnvironment.
  std::unique_ptr<sync_preferences::PrefServiceSyncable> prefs;
  if (params.prefs_content.has_value()) {
    base::FilePath prefs_path =
        profile_dir.Append(kPreferencesFilename);
    if (!base::WriteFile(prefs_path, params.prefs_content.value())) {
      LOG(ERROR) << "Failed to write a prefs file";
      return {nullptr, nullptr};
    }

    // Create a PrefService that only contains user defined preference values
    // and policies.
    sync_preferences::PrefServiceMockFactory factory;
    factory.SetUserPrefsFile(
        prefs_path, base::SingleThreadTaskRunner::GetCurrentDefault().get());
    scoped_refptr<user_prefs::PrefRegistrySyncable> registry(
        new user_prefs::PrefRegistrySyncable);

    prefs = factory.CreateSyncable(registry.get());

    AudioAPI::RegisterUserPrefs(registry.get());
    ExtensionPrefs::RegisterProfilePrefs(registry.get());
    PermissionsManager::RegisterProfilePrefs(registry.get());
  }

  auto browser_context = std::make_unique<TestBrowserContext>(profile_dir);
  auto env = std::make_unique<TestExtensionEnvironment>(browser_context.get(), std::move(prefs));

  return {std::move(browser_context), std::move(env)};
}

}  // namespace

class TestContentBrowserClient : public content::ContentBrowserClient {
 public:
  TestContentBrowserClient() = default;
  ~TestContentBrowserClient() override = default;

  void OnWebContentsCreated(content::WebContents* web_contents) override {
    ChromeExtensionWebContentsObserver::CreateForWebContents(web_contents);
  }
};

ExtensionServiceTestBase::ExtensionServiceInitParams::
    ExtensionServiceInitParams() = default;

ExtensionServiceTestBase::ExtensionServiceInitParams::
    ExtensionServiceInitParams(const ExtensionServiceInitParams& other) =
        default;

ExtensionServiceTestBase::ExtensionServiceInitParams::
    ~ExtensionServiceInitParams() = default;

bool ExtensionServiceTestBase::ExtensionServiceInitParams::
    SetPrefsContentFromFile(const base::FilePath& filepath) {
  std::string content;
  if (!base::ReadFileToString(filepath, &content)) {
    return false;
  }
  prefs_content.emplace(std::move(content));
  return true;
}

bool ExtensionServiceTestBase::ExtensionServiceInitParams::
    ConfigureByTestDataDirectory(const base::FilePath& filepath) {
  if (!SetPrefsContentFromFile(filepath.Append(kPreferencesFilename))) {
    return false;
  }
  extensions_dir = filepath.AppendASCII(extensions::kInstallDirectoryName);
  unpacked_extensions_dir = filepath.AppendASCII(extensions::kUnpackedInstallDirectoryName);
  return true;
}

ExtensionServiceTestBase::ExtensionServiceTestBase()
    : ExtensionServiceTestBase(
          std::make_unique<content::BrowserTaskEnvironment>(
              base::test::TaskEnvironment::MainThreadType::IO)) {}

ExtensionServiceTestBase::ExtensionServiceTestBase(
    std::unique_ptr<content::BrowserTaskEnvironment> task_environment)
    : task_environment_(std::move(task_environment)),
      service_(nullptr),
      registry_(nullptr),
      verifier_format_override_(crx_file::VerifierFormat::CRX3) {
  base::FilePath test_data_dir;
  base::PathService::Get(components_extensions::DIR_TEST_DATA, &test_data_dir);
  if (!base::PathExists(test_data_dir)) {  // We don't want to create this.
    ADD_FAILURE();
    return;
  }

  data_dir_ = test_data_dir.AppendASCII("extensions");

  policy_service_ = std::make_unique<policy::PolicyServiceImpl>(
      std::vector<
          raw_ptr<policy::ConfigurationPolicyProvider, VectorExperimental>>{
          &policy_provider_});

  overriden_client_ = std::make_unique<TestContentBrowserClient>();
}

ExtensionServiceTestBase::~ExtensionServiceTestBase() {
  // Why? Because |browser_context_| has to be destroyed before |at_exit_manager_|, but
  // is declared above it in the class definition since it's protected.
  // TODO(1269752): Since we're getting rid of at_exit_manager_, perhaps
  // we don't need this call?
  browser_context_.reset();
}

void ExtensionServiceTestBase::InitializeExtensionService(
    const ExtensionServiceTestBase::ExtensionServiceInitParams& params) {
  std::tie(browser_context_, env_) =
      BuildTestingBrowserContext(params, temp_dir_, policy_service_.get());

  extensions_install_dir_ =
      browser_context_->GetPath().AppendASCII(extensions::kInstallDirectoryName);
  unpacked_install_dir_ =
      browser_context_->GetPath().AppendASCII(extensions::kUnpackedInstallDirectoryName);

  CreateExtensionService(params);
  registry_ = ExtensionRegistry::Get(browser_context());
}

void ExtensionServiceTestBase::InitializeEmptyExtensionService() {
  ExtensionServiceInitParams params;
  params.prefs_content = "";
  InitializeExtensionService(params);
}

void ExtensionServiceTestBase::InitializeGoodInstalledExtensionService() {
  ExtensionServiceInitParams params;
  ASSERT_TRUE(
      params.ConfigureByTestDataDirectory(data_dir().AppendASCII("good")));
  InitializeExtensionService(params);
}

void ExtensionServiceTestBase::InitializeExtensionServiceWithUpdater() {
  ExtensionServiceInitParams params;
  params.autoupdate_enabled = true;
  InitializeExtensionService(params);
  // TODO(mshin): Enable the below code after migrating ExtensionUpdater
  // service_->updater()->Start();
}

void ExtensionServiceTestBase::
    InitializeExtensionServiceWithExtensionsDisabled() {
  ExtensionServiceInitParams params;
  params.extensions_enabled = false;
  InitializeExtensionService(params);
}

size_t ExtensionServiceTestBase::GetPrefKeyCount() {
  const base::Value::Dict& dict =
      env_->GetPrefService()->GetDict(extensions::pref_names::kExtensions);
  return dict.size();
}

void ExtensionServiceTestBase::ValidatePrefKeyCount(size_t count) {
  EXPECT_EQ(count, GetPrefKeyCount());
}

testing::AssertionResult ExtensionServiceTestBase::ValidateBooleanPref(
    const std::string& extension_id,
    const std::string& pref_path,
    bool expected_val) {
  std::string msg =
      base::StringPrintf("while checking: %s %s == %s", extension_id.c_str(),
                         pref_path.c_str(), expected_val ? "true" : "false");

  PrefService* prefs = env_->GetPrefService();
  const base::Value::Dict& dict =
      prefs->GetDict(extensions::pref_names::kExtensions);
  const base::Value::Dict* pref = dict.FindDict(extension_id);

  if (!pref) {
    return testing::AssertionFailure()
           << "extension pref does not exist " << msg;
  }

  std::optional<bool> val = pref->FindBoolByDottedPath(pref_path);
  if (!val.has_value()) {
    return testing::AssertionFailure()
           << pref_path << " pref not found " << msg;
  }

  return expected_val == val.value() ? testing::AssertionSuccess()
                                     : testing::AssertionFailure()
                                           << "base::Value is incorrect "
                                           << msg;
}

void ExtensionServiceTestBase::ValidateIntegerPref(
    const std::string& extension_id,
    const std::string& pref_path,
    int expected_val) {
  std::string msg = base::StringPrintf(
      "while checking: %s %s == %s", extension_id.c_str(), pref_path.c_str(),
      base::NumberToString(expected_val).c_str());

  PrefService* prefs = env_->GetPrefService();
  const base::Value::Dict& dict = prefs->GetDict(extensions::pref_names::kExtensions);
  const base::Value::Dict* pref = dict.FindDict(extension_id);
  ASSERT_TRUE(pref) << msg;
  EXPECT_EQ(expected_val, pref->FindIntByDottedPath(pref_path)) << msg;
}

void ExtensionServiceTestBase::ValidateStringPref(
    const std::string& extension_id,
    const std::string& pref_path,
    const std::string& expected_val) {
  std::string msg = base::StringPrintf("while checking: %s.manifest.%s == %s",
                                       extension_id.c_str(), pref_path.c_str(),
                                       expected_val.c_str());

  const base::Value::Dict& dict =
      env_->GetPrefService()->GetDict(extensions::pref_names::kExtensions);
  std::string manifest_path = extension_id + ".manifest";
  const base::Value::Dict* pref = dict.FindDictByDottedPath(manifest_path);
  ASSERT_TRUE(pref) << msg;
  const std::string* val = pref->FindStringByDottedPath(pref_path);
  ASSERT_TRUE(val) << msg;
  EXPECT_EQ(expected_val, *val) << msg;
}

void ExtensionServiceTestBase::SetUp() {
  original_client_ = content::SetBrowserClientForTesting(overriden_client_.get());

  LoadErrorReporter::GetInstance()->ClearErrors();

  // Update the webstore update url. Some tests leave it set to a non-default
  // webstore_update_url_. This can make extension_urls::IsWebstoreUpdateUrl
  // return a false negative.
  ExtensionsClient::Get()->InitializeWebStoreUrls(
      base::CommandLine::ForCurrentProcess());
}

void ExtensionServiceTestBase::TearDown() {
  if (browser_context_) {
    content::StoragePartitionConfig default_storage_partition_config =
        content::StoragePartitionConfig::CreateDefault(browser_context());
    auto* partition = browser_context_->GetStoragePartition(
        default_storage_partition_config, /*can_create=*/false);
    if (partition) {
      partition->WaitForDeletionTasksForTesting();
    }
  }
  policy_provider_.Shutdown();

  // TODO(mshin): Consider to use DependencyManager instead of the below code
  extensions::ProcessManager::Get(browser_context_.get())->Shutdown();
  env_ = nullptr;
  browser_context_ = nullptr;

  content::SetBrowserClientForTesting(original_client_.get());
  original_client_ = nullptr;
}

void ExtensionServiceTestBase::SetUpTestSuite() {
  // Safe to call multiple times.
  LoadErrorReporter::Init(false);  // no noisy errors.
}

// These are declared in the .cc so that all inheritors don't need to know
// that TestingProfile derives Profile derives BrowserContext.
content::BrowserContext* ExtensionServiceTestBase::browser_context() {
  return browser_context_.get();
}

sync_preferences::TestingPrefServiceSyncable*
ExtensionServiceTestBase::testing_pref_service() {
  return env_->GetTestingPrefService();
}

void ExtensionServiceTestBase::CreateExtensionService(
    const ExtensionServiceInitParams& params) {
  TestExtensionSystem* system =
      static_cast<TestExtensionSystem*>(ExtensionSystem::Get(browser_context()));
  if (!params.is_first_run) {
    ExtensionPrefs::Get(browser_context())->SetAlertSystemFirstRun();
  }

  service_ = system->CreateExtensionService(
      base::CommandLine::ForCurrentProcess(), extensions_install_dir_,
      unpacked_install_dir_, params.autoupdate_enabled,
      params.extensions_enabled);

  service_->component_loader()->set_ignore_allowlist_for_testing(true);

  // When we start up, we want to make sure there is no external provider,
  // since the ExtensionService on Windows will use the Registry as a default
  // provider and if there is something already registered there then it will
  // interfere with the tests. Those tests that need an external provider
  // will register one specifically.
  service_->ClearProvidersForTesting();

  service_->RegisterInstallGate(ExtensionPrefs::DELAY_REASON_WAIT_FOR_IMPORTS,
                                service_->shared_module_service());
}

}  // namespace components_extensions
