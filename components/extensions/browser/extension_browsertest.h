// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_BROWSERTEST_H_
#define COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_BROWSERTEST_H_

#include <optional>
#include <string>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/raw_ptr.h"
#include "base/scoped_observation.h"
#include "base/test/scoped_path_override.h"
#include "build/build_config.h"
#include "content/public/test/content_browser_test.h"
#include "components/extensions/browser/chrome_extensions_browser_client.h"
#include "components/extensions/browser/event_router_forwarder.h"
#include "components/extensions/browser/install_verifier.h"
// #include "components/extensions/browser/updater/extension_updater.h"
#include "content/public/test/browser_test_base.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/browsertest_util.h"
#include "extensions/browser/extension_creator.h"
#include "extensions/browser/extension_host.h"
#include "extensions/browser/extension_protocols.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_registry_observer.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/sandboxed_unpacker.h"
#include "extensions/browser/scoped_ignore_content_verifier_for_test.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_id.h"
#include "extensions/common/feature_switch.h"
#include "extensions/common/features/feature_channel.h"
#include "extensions/common/manifest.h"
#include "extensions/common/mojom/manifest.mojom-shared.h"

class PrefService;

namespace content {
class BrowserContext;
class ServiceWorkerContext;
class WebContents;
}  // namespace content

namespace extensions {
class ExtensionCacheFake;
class ExtensionSet;
class ProcessManager;
}

namespace components_extensions {
class ChromeExtensionTestNotificationObserver;
class ExtensionService;

// Base class for extension browser tests. Provides utilities for loading,
// unloading, and installing extensions.
class ExtensionBrowserTest : public content::ContentBrowserTest,
                             public extensions::ExtensionRegistryObserver,
                             public ChromeExtensionsBrowserClient::Delegate {
 public:
  // Different types of extension's lazy background contexts used in some tests.
  enum class ContextType {
    // TODO(crbug.com:/1241220): Get rid of this value when we can use
    // std::optional in the LoadOptions struct.
    // No specific context type.
    kNone,
    // A non-persistent background page/JS based extension.
    kEventPage,
    // A Service Worker based extension.
    kServiceWorker,
    // An extension with a persistent background page.
    kPersistentBackground,
    // Use the value from the manifest. This is used when the test
    // has been parameterized but the particular extension should
    // be loaded without using the parameterized type. Typically,
    // this is used when a test loads another extension that is
    // not parameterized.
    kFromManifest,
  };

  ExtensionBrowserTest(const ExtensionBrowserTest&) = delete;
  ExtensionBrowserTest& operator=(const ExtensionBrowserTest&) = delete;

  // ExtensionRegistryObserver:
  void OnExtensionLoaded(content::BrowserContext* browser_context,
                         const extensions::Extension* extension) override;
  void OnShutdown(extensions::ExtensionRegistry* registry) override;

 protected:
  struct LoadOptions {
    // Allows the extension to run in incognito mode.
    bool allow_in_incognito = false;

    // Allows file access for the extension.
    bool allow_file_access = false;

    // Doesn't fail when the loaded manifest has warnings (should only be used
    // when testing deprecated features).
    bool ignore_manifest_warnings = false;

    // Waits for extension renderers to fully load.
    bool wait_for_renderers = true;

    // An optional install param.
    const char* install_param = nullptr;

    // If this is a Service Worker-based extension, wait for the
    // Service Worker's registration to be stored before returning.
    bool wait_for_registration_stored = false;

    // Loads the extension with location COMPONENT.
    bool load_as_component = false;

    // Changes the "manifest_version" manifest key to 3. Note as of now, this
    // doesn't make any other changes to convert the extension to MV3 other than
    // changing the integer value in the manifest.
    bool load_as_manifest_version_3 = false;

    // Used to force loading the extension with a particular background type.
    // Currently this only support loading an extension as using a service
    // worker.
    ContextType context_type = ContextType::kNone;
  };

  explicit ExtensionBrowserTest(ContextType context_type = ContextType::kNone);
  ~ExtensionBrowserTest() override;

  // Useful accessors.
  ExtensionService* extension_service();

  extensions::ExtensionRegistry* extension_registry();

  const extensions::ExtensionId& last_loaded_extension_id() {
    return last_loaded_extension_id_;
  }

  // Get the BrowserContext to use.
  virtual content::BrowserContext* browser_context();

  virtual content::WebContents* web_contents();

  // Extensions used in tests are typically not from the web store and will have
  // missing content verification hashes. The default implementation disables
  // content verification; this should be overridden by derived tests which care
  // about content verification.
  virtual bool ShouldEnableContentVerification();

  // Extensions used in tests are typically not from the web store and will fail
  // install verification. The default implementation disables install
  // verification; this should be overridden by derived tests which care
  // about install verification.
  virtual bool ShouldEnableInstallVerification();

  // Returns the path of the directory from which to serve resources when they
  // are prefixed with "_test_resources/".
  // The default is chrome/test/data/extensions/.
  virtual base::FilePath GetTestResourcesParentDir();

  static const extensions::Extension* 
  GetExtensionByPath(const extensions::ExtensionSet& extensions,
                     const base::FilePath& path);

  // BrowserTestBase:
  void PreRunTestOnMainThread() override;
  void PostRunTestOnMainThread() override;
  void SetUp() override;
  void SetUpCommandLine(base::CommandLine* command_line) override;
  void SetUpOnMainThread() override;
  void TearDownOnMainThread() override;

  // ChromeExtensionsBrowserClient::Delegate
  EventRouterForwarder* extension_event_router_forwarder() override;
  network::mojom::NetworkContext* GetNetworkContext() override;
  bool IsShuttingDown() override;
  std::string GetApplicationLocale() override;
  std::vector<content::BrowserContext*> GetAllBrowserContexts() override;
  content::BrowserContext*
  GetOriginalBrowserContext(content::BrowserContext* context) override;
  PrefService*
  GetPrefServiceForContext(content::BrowserContext* context) override;

  const extensions::Extension* LoadExtension(const base::FilePath& path);

  const extensions::Extension* LoadExtension(const base::FilePath& path,
                                             const LoadOptions& options);

  // Loads unpacked extension from |path| with manifest |manifest_relative_path|
  // and imitates that it is a component extension.
  // |manifest_relative_path| is relative to |path|.
  const extensions::Extension* LoadExtensionAsComponentWithManifest(
      const base::FilePath& path,
      const base::FilePath::CharType* manifest_relative_path);

  // Loads unpacked extension from |path| and imitates that it is a component
  // extension. Equivalent to
  // LoadExtensionAsComponentWithManifest(path, kManifestFilename).
  const extensions::Extension* LoadExtensionAsComponent(const base::FilePath& path);

  // Pack the extension in |dir_path| into a crx file and return its path.
  // Return an empty FilePath if there were errors.
  base::FilePath PackExtension(
      const base::FilePath& dir_path,
      int extra_run_flags = extensions::ExtensionCreator::kNoRunFlags);

  // Pack the extension in |dir_path| into a crx file at |crx_path|, using the
  // key |pem_path|. If |pem_path| does not exist, create a new key at
  // |pem_out_path|.
  // Return the path to the crx file, or an empty FilePath if there were errors.
  base::FilePath PackExtensionWithOptions(
      const base::FilePath& dir_path,
      const base::FilePath& crx_path,
      const base::FilePath& pem_path,
      const base::FilePath& pem_out_path,
      int extra_run_flags = extensions::ExtensionCreator::kNoRunFlags);

  // |expected_change| indicates how many extensions should be installed (or
  // disabled, if negative).
  // 1 means you expect a new install, 0 means you expect an upgrade, -1 means
  // you expect a failed upgrade.
  const extensions::Extension* InstallExtension(const base::FilePath& path,
                                    std::optional<int> expected_change) {
    return InstallOrUpdateExtension(std::string(), path, INSTALL_UI_TYPE_NONE,
                                    std::move(expected_change));
  }

  // Same as above, but an install source other than
  // mojom::ManifestLocation::kInternal can be specified.
  const extensions::Extension* InstallExtension(const base::FilePath& path,
                                    std::optional<int> expected_change,
                                    extensions::mojom::ManifestLocation install_source) {
    return InstallOrUpdateExtension(std::string(), path, INSTALL_UI_TYPE_NONE,
                                    std::move(expected_change), install_source);
  }

  // Installs an extension and grants it the permissions it requests.
  // TODO(devlin): It seems like this is probably the desired outcome most of
  // the time - otherwise the extension installs in a disabled state.
  const extensions::Extension* InstallExtensionWithPermissionsGranted(
      const base::FilePath& file_path,
      std::optional<int> expected_change) {
    return InstallOrUpdateExtension(
        std::string(), file_path, INSTALL_UI_TYPE_NONE,
        std::move(expected_change), extensions::mojom::ManifestLocation::kInternal,
        browser_context(), extensions::Extension::NO_FLAGS, false, true);
  }

  // Installs extension as if it came from the Chrome Webstore.
  const extensions::Extension* InstallExtensionFromWebstore(
      const base::FilePath& path,
      std::optional<int> expected_change);

  // Same as above but passes an id to CrxInstaller and does not allow a
  // privilege increase.
  const extensions::Extension*
  UpdateExtension(const extensions::ExtensionId& id,
                  const base::FilePath& path,
                  std::optional<int> expected_change) {
    return InstallOrUpdateExtension(id, path, INSTALL_UI_TYPE_NONE,
                                    std::move(expected_change));
  }

  // Same as UpdateExtension but waits for the extension to be idle first.
  const extensions::Extension* UpdateExtensionWaitForIdle(
      const extensions::ExtensionId& id,
      const base::FilePath& path,
      std::optional<int> expected_change);

  const extensions::Extension* InstallExtensionWithUIAutoConfirm(
      const base::FilePath& path,
      std::optional<int> expected_change,
      content::BrowserContext* browser_context) {
    return InstallOrUpdateExtension(
        std::string(), path, INSTALL_UI_TYPE_AUTO_CONFIRM,
        std::move(expected_change), browser_context,
        extensions::Extension::NO_FLAGS);
  }

  const extensions::Extension* InstallExtensionWithSourceAndFlags(
      const base::FilePath& path,
      std::optional<int> expected_change,
      extensions::mojom::ManifestLocation install_source,
      extensions::Extension::InitFromValueFlags creation_flags) {
    return InstallOrUpdateExtension(std::string(), path, INSTALL_UI_TYPE_NONE,
                                    std::move(expected_change), install_source,
                                    browser_context(), creation_flags, false, false);
  }

  // Begins install process but simulates a user cancel.
  const extensions::Extension* StartInstallButCancel(const base::FilePath& path) {
    return InstallOrUpdateExtension(
        std::string(), path, INSTALL_UI_TYPE_CANCEL, 0);
  }

  void ReloadExtension(const extensions::ExtensionId& extension_id);

  void UnloadExtension(const extensions::ExtensionId& extension_id);

  void UninstallExtension(const extensions::ExtensionId& extension_id);

  void DisableExtension(const extensions::ExtensionId& extension_id);

  void EnableExtension(const extensions::ExtensionId& extension_id);

  // Wait for the number of visible page actions to change to |count|.
  bool WaitForPageActionVisibilityChangeTo(int count);

  // Wait for all extension views to load.
  bool WaitForExtensionViewsToLoad();

  // Wait for the extension to be idle.
  bool WaitForExtensionIdle(const extensions::ExtensionId& extension_id);

  // Wait for the extension to not be idle.
  bool WaitForExtensionNotIdle(const extensions::ExtensionId& extension_id);

  // Simulates a page calling window.open on an URL and waits for the
  // navigation.
  // |should_succeed| indicates whether the navigation should succeed, in which
  // case the last committed url should match the passed url and the page should
  // not be an error or interstitial page.
  void OpenWindow(content::WebContents* contents,
                  const GURL& url,
                  bool newtab_process_should_equal_opener,
                  bool should_succeed,
                  content::WebContents** newtab_result);

  // Simulates a page navigating itself to an URL and waits for the
  // navigation. Returns true if the navigation succeeds.
  [[nodiscard]] bool NavigateInRenderer(content::WebContents* contents,
                                        const GURL& url);

  // Looks for an ExtensionHost whose URL has the given path component
  // (including leading slash).  Also verifies that the expected number of hosts
  // are loaded.
  extensions::ExtensionHost*
  FindHostWithPath(extensions::ProcessManager* manager,
                   const std::string& path,
                   int expected_hosts);

  // Waits until `script` calls "chrome.test.sendScriptResult(result)",
  // where `result` is a serializable value, and returns `result`. Fails
  // the test and returns an empty base::Value if `extension_id` isn't
  // installed in the test's profile or doesn't have a background page, or
  // if executing the script fails. The argument `script_user_activation`
  // determines if the script should be executed after a user activation.
  base::Value ExecuteScriptInBackgroundPage(
      const extensions::ExtensionId& extension_id,
      const std::string& script,
      extensions::browsertest_util::ScriptUserActivation script_user_activation =
          extensions::browsertest_util::ScriptUserActivation::kDontActivate);

  // Waits until |script| calls "window.domAutomationController.send(result)",
  // where |result| is a string, and returns |result|. Fails the test and
  // returns an empty base::Value if |extension_id| isn't installed in test's
  // profile or doesn't have a background page, or if executing the script
  // fails. The argument |script_user_activation| determines if the script
  // should be executed after a user activation.
  std::string ExecuteScriptInBackgroundPageDeprecated(
      const extensions::ExtensionId& extension_id,
      const std::string& script,
      extensions::browsertest_util::ScriptUserActivation script_user_activation =
          extensions::browsertest_util::ScriptUserActivation::kDontActivate);

  bool ExecuteScriptInBackgroundPageNoWait(
      const extensions::ExtensionId& extension_id,
      const std::string& script,
      extensions::browsertest_util::ScriptUserActivation script_user_activation =
          extensions::browsertest_util::ScriptUserActivation::kDontActivate);

  // Get the ServiceWorkerContext for the default browser's profile.
  content::ServiceWorkerContext* GetServiceWorkerContext();

  // Get the ServiceWorkerContext for the `browser_context`.
  static content::ServiceWorkerContext* GetServiceWorkerContext(
      content::BrowserContext* browser_context);

  // Set to "chrome/test/data/extensions". Derived classes may override.
  // TODO(michaelpg): Don't override protected data members.
  base::FilePath test_data_dir_;

  std::unique_ptr<ChromeExtensionTestNotificationObserver> observer_;

  const ContextType context_type_;

 private:
  // Modifies extension at `input_path` as dictated by `options`. On success,
  // returns true and populates `out_path`. On failure, false is returned.
  bool ModifyExtensionIfNeeded(const LoadOptions& options,
                               const base::FilePath& input_path,
                               base::FilePath* out_path);

  // Temporary directory for testing.
  base::ScopedTempDir temp_dir_;

  // Specifies the type of UI (if any) to show during installation and what
  // user action to simulate.
  enum InstallUIType {
    INSTALL_UI_TYPE_NONE,
    INSTALL_UI_TYPE_CANCEL,
    INSTALL_UI_TYPE_NORMAL,
    INSTALL_UI_TYPE_AUTO_CONFIRM,
  };

  const extensions::Extension*
  InstallOrUpdateExtension(const extensions::ExtensionId& id,
                           const base::FilePath& path,
                           InstallUIType ui_type,
                           std::optional<int> expected_change);
  const extensions::Extension* InstallOrUpdateExtension(
      const extensions::ExtensionId& id,
      const base::FilePath& path,
      InstallUIType ui_type,
      std::optional<int> expected_change,
      content::BrowserContext* browser_context,
      extensions::Extension::InitFromValueFlags creation_flags);
  const extensions::Extension* InstallOrUpdateExtension(
      const extensions::ExtensionId& id,
      const base::FilePath& path,
      InstallUIType ui_type,
      std::optional<int> expected_change,
      extensions::mojom::ManifestLocation install_source);
  const extensions::Extension* InstallOrUpdateExtension(
      const extensions::ExtensionId& id,
      const base::FilePath& path,
      InstallUIType ui_type,
      std::optional<int> expected_change,
      extensions::mojom::ManifestLocation install_source,
      content::BrowserContext* browser_context,
      extensions::Extension::InitFromValueFlags creation_flags,
      bool wait_for_idle,
      bool grant_permissions);

  // Used for setting the default scoped current channel for extension browser
  // tests to UNKNOWN (trunk), in order to enable channel restricted features.
  // TODO(crbug/1427323): We should remove this and have the current channel
  // respect what is defined on the builder. If a test requires a specific
  // channel for a channel restricted feature, it should be defining its own
  // scoped channel override. As this stands, it means we don't really have
  // non-trunk coverage for most extension browser tests.
  extensions::ScopedCurrentChannel current_channel_;

  // Disable external install UI.
  extensions::FeatureSwitch::ScopedOverride override_prompt_for_external_extensions_;

#if BUILDFLAG(IS_WIN)
  // Use mock shortcut directories to ensure app shortcuts are cleaned up.
  base::ScopedPathOverride user_desktop_override_;
  base::ScopedPathOverride common_desktop_override_;
  base::ScopedPathOverride user_quick_launch_override_;
  base::ScopedPathOverride start_menu_override_;
  base::ScopedPathOverride common_start_menu_override_;
#endif

  // The default browser context to be used.
  raw_ptr<content::BrowserContext, AcrossTasksDanglingUntriaged> browser_context_;

  // Cache cache implementation.
  std::unique_ptr<extensions::ExtensionCacheFake> test_extension_cache_;

  // An override so that chrome-extensions://<extension_id>/_test_resources/foo
  // maps to chrome/test/data/extensions/foo.
  extensions::ExtensionProtocolTestHandler test_protocol_handler_;

  // Conditionally disable content verification.
  std::unique_ptr<extensions::ScopedIgnoreContentVerifierForTest>
      ignore_content_verification_;

  // Conditionally disable install verification.
  std::unique_ptr<ScopedInstallVerifierBypassForTest>
      ignore_install_verification_;

  // Used to disable CRX publisher signature checking.
  extensions::SandboxedUnpacker::ScopedVerifierFormatOverrideForTest
      verifier_format_override_;

  // TODO(mshin): Enable the below code after migrating ExtensionUpdater
  // ExtensionUpdater::ScopedSkipScheduledCheckForTest skip_scheduled_check_;

  // Listens to extension loaded notifications.
  base::ScopedObservation<extensions::ExtensionRegistry,
                          extensions::ExtensionRegistryObserver>
      registry_observation_{this};

  extensions::ExtensionId last_loaded_extension_id_;

  std::unique_ptr<PrefService> local_state_;
  std::unique_ptr<PrefService> user_pref_service_;
  std::unique_ptr<ChromeExtensionsBrowserClient> extensions_browser_client_;
  scoped_refptr<EventRouterForwarder> extension_event_router_forwarder_;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_BROWSERTEST_H_
