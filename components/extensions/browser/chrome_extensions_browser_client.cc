// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/chrome_extensions_browser_client.h"

#include <memory>
#include <optional>
#include <utility>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/memory/scoped_refptr.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "base/values.h"
#include "base/version.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "components/extensions/browser/api/chrome_extensions_api_client.h"
#include "components/extensions/browser/chrome_component_extension_resource_manager.h"
#include "components/extensions/browser/chrome_content_browser_client_extensions_part.h"
#include "components/extensions/browser/chrome_extensions_browser_api_provider.h"
#include "components/extensions/browser/chrome_extension_host_delegate.h"
#include "components/extensions/browser/chrome_extension_web_contents_observer.h"
#include "components/extensions/browser/chrome_url_request_util.h"
#include "components/extensions/browser/error_console/error_console.h"
#include "components/extensions/browser/event_router_forwarder.h"
#include "components/extensions/browser/extension_service.h"
#include "components/extensions/browser/extension_system_factory.h"
#include "components/extensions/browser/extension_util.h"
#include "components/extensions/browser/user_script_listener.h"
#include "components/extensions/common/extension_constants.h"
#include "components/content_settings/core/common/pref_names.h"
#include "components/embedder_support/user_agent_utils.h"
#include "components/privacy_sandbox/privacy_sandbox_prefs.h"
#include "components/proxy_config/proxy_config_pref_names.h"
#include "components/safe_browsing/core/common/features.h"
#include "components/sessions/content/session_tab_helper.h"
#include "components/update_client/update_client.h"
#include "components/version_info/version_info.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/site_instance.h"
#include "content/public/browser/storage_partition_config.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/url_constants.h"
#include "extensions/browser/api/content_settings/content_settings_service.h"
#include "extensions/browser/api/core_extensions_browser_api_provider.h"
#include "extensions/browser/extension_error.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/extension_util.h"
#include "extensions/browser/extensions_browser_interface_binders.h"
#include "extensions/browser/pref_names.h"
#include "extensions/browser/updater/null_extension_cache.h"
#include "extensions/browser/updater/scoped_extension_updater_keep_alive.h"
#include "extensions/browser/url_request_util.h"
#include "extensions/common/extension_id.h"
#include "extensions/common/extension_urls.h"
#include "extensions/common/features/feature_channel.h"
#include "extensions/common/permissions/permission_set.h"
#include "net/http/http_response_headers.h"
#include "ipc/ipc_message.h"
#include "url/gurl.h"

using extensions::ComponentExtensionResourceManager;
using extensions::CoreExtensionsBrowserAPIProvider;
using extensions::EarlyExtensionPrefsObserver;
using extensions::Extension;
using extensions::ExtensionsBrowserClient;
using extensions::ExtensionCache;
using extensions::ExtensionError;
using extensions::ExtensionHostDelegate;
using extensions::ExtensionId;
using extensions::ExtensionPrefs;
using extensions::ExtensionSet;
using extensions::ExtensionSystem;
using extensions::ExtensionSystemProvider;
using extensions::ExtensionWebContentsObserver;
using extensions::NullExtensionCache;
using extensions::KioskDelegate;
using extensions::PermissionSet;
using extensions::ProcessManagerDelegate;
using extensions::ProcessMap;
using extensions::RuntimeAPIDelegate;
using extensions::ScopedExtensionUpdaterKeepAlive;
using extensions::URLPatternSet;

namespace components_extensions {

namespace {

// TODO(mshin): Enable the below code after migrating Updater
// const char kCrxUrlPath[] = "/service/update2/crx";
// const char kJsonUrlPath[] = "/service/update2/json";

// If true, the extensions client will behave as though there is always a
// new chrome update.
bool g_did_chrome_update_for_testing = false;

bool ExtensionsDisabled(const base::CommandLine& command_line) {
  return command_line.HasSwitch(::switches::kDisableExtensions) ||
         command_line.HasSwitch(::switches::kDisableExtensionsExcept);
}

// TODO(mshin): Enable the below code after migrating Updater & KeepyAlive
// class UpdaterKeepAlive : public ScopedExtensionUpdaterKeepAlive {
//  public:
//   UpdaterKeepAlive(Profile* profile, ProfileKeepAliveOrigin origin)
//       : profile_keep_alive_(profile, origin) {}
//   ~UpdaterKeepAlive() override = default;

//  private:
//   ScopedProfileKeepAlive profile_keep_alive_;
// };

bool ShouldLogExtensionAction(content::BrowserContext* browser_context,
                              const ExtensionId& extension_id) {
  // We only send these IPCs if activity logging is enabled, but due to race
  // conditions (e.g. logging gets disabled but the renderer sends the message
  // before it gets updated), we still need this check here.
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  // TODO(mshin): Enable the below code after migrating ActivityLog & Profile
  // return browser_context &&
  //        delegate_->profile_manager()->IsValidProfile(
  //            browser_context) &&
  //        ActivityLog::GetInstance(browser_context) &&
  //        ActivityLog::GetInstance(browser_context)->ShouldLog(extension_id);
  return false;
}

// TODO(mshin): Enable the below code after migrating ActivityLog
// Logs an action to the extension activity log for the specified profile.
// void AddActionToExtensionActivityLog(content::BrowserContext* browser_context,
//                                      scoped_refptr<Action> action) {
//   DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
//   // If the action included a URL, check whether it is for an incognito profile.
//   // The check is performed here so that it can safely be done from the UI
//   // thread.
//   if (action->page_url().is_valid() || !action->page_title().empty()) {
//     action->set_page_incognito(browser_context->IsOffTheRecord());
//   }
//   ActivityLog::GetInstance(browser_context)->LogAction(action);
// }

bool RegisterTransformers() {
  // TODO(mshin): Enable the below code after migrating PrefMapping
  // PrefMapping* pref_mapping = PrefMapping::GetInstance();
  // pref_mapping->RegisterPrefTransformer(
  //     prefs::kCookieControlsMode,
  //     std::make_unique<CookieControlsModeTransformer>());
  // pref_mapping->RegisterPrefTransformer(
  //     proxy_config::prefs::kProxy, std::make_unique<ProxyPrefTransformer>());
  // pref_mapping->RegisterPrefTransformer(
  //     prefetch::prefs::kNetworkPredictionOptions,
  //     std::make_unique<NetworkPredictionTransformer>());
  // pref_mapping->RegisterPrefTransformer(
  //     prefs::kProtectedContentDefault,
  //     std::make_unique<ProtectedContentEnabledTransformer>());
  // pref_mapping->RegisterPrefTransformer(
  //     prefs::kPrivacySandboxM1TopicsEnabled,
  //     std::make_unique<PrivacySandboxTransformer>());
  // pref_mapping->RegisterPrefTransformer(
  //     prefs::kPrivacySandboxM1FledgeEnabled,
  //     std::make_unique<PrivacySandboxTransformer>());
  // pref_mapping->RegisterPrefTransformer(
  //     prefs::kPrivacySandboxM1AdMeasurementEnabled,
  //     std::make_unique<PrivacySandboxTransformer>());
  // pref_mapping->RegisterPrefTransformer(
  //     prefs::kPrivacySandboxRelatedWebsiteSetsEnabled,
  //     std::make_unique<PrivacySandboxTransformer>());

  return true;
}

}  // namespace

ChromeExtensionsBrowserClient::ChromeExtensionsBrowserClient(Delegate* delegate)
  : delegate_(delegate) {
  AddAPIProvider(std::make_unique<CoreExtensionsBrowserAPIProvider>());
  AddAPIProvider(std::make_unique<ChromeExtensionsBrowserAPIProvider>());

  // This ensures transformers are only registered once. This is required
  // because testing will create the singleton ChromeExtensionsBrowserClient
  // instance multiple times within the same process.
  static bool registered = RegisterTransformers();
  CHECK(registered);

  process_manager_delegate_ = std::make_unique<ChromeProcessManagerDelegate>();
  api_client_ = std::make_unique<ChromeExtensionsAPIClient>();
  extensions::SetCurrentChannel(version_info::Channel::STABLE);
  resource_manager_ =
      std::make_unique<ChromeComponentExtensionResourceManager>();
}

ChromeExtensionsBrowserClient::~ChromeExtensionsBrowserClient() {
  delegate_ = nullptr;
}

void ChromeExtensionsBrowserClient::StartTearDown() {
  GetUserScriptListener()->StartTearDown();
}

bool ChromeExtensionsBrowserClient::IsShuttingDown() {
  return delegate_->IsShuttingDown();
}

std::vector<content::BrowserContext*>
ChromeExtensionsBrowserClient::GetAllBrowserContexts() {
  return delegate_->GetAllBrowserContexts();
}

bool ChromeExtensionsBrowserClient::AreExtensionsDisabled(
    const base::CommandLine& command_line,
    content::BrowserContext* context) {
  // TODO(mshin): Enable the below code after migrating preference
  // Profile* profile = static_cast<Profile*>(context);
  // return ExtensionsDisabled(command_line) ||
  //        profile->GetPrefs()->GetBoolean(prefs::kDisableExtensions);
  return ExtensionsDisabled(command_line);
}

bool ChromeExtensionsBrowserClient::IsValidContext(void* context) {
  DCHECK(context);
  // TODO(mshin): Support Profile for Embedder
  return true;
}

bool ChromeExtensionsBrowserClient::IsSameContext(
    content::BrowserContext* first,
    content::BrowserContext* second) {
  // TODO(mshin): Support Profile for Embedder
  // Profile* first_profile = Profile::FromBrowserContext(first);
  // Profile* second_profile = Profile::FromBrowserContext(second);
  // return first_profile->IsSameOrParent(second_profile);
  return first == second;
}

bool ChromeExtensionsBrowserClient::HasOffTheRecordContext(
    content::BrowserContext* context) {
  // TODO(mshin): Support Profile for Embedder
  // return static_cast<Profile*>(context)->HasPrimaryOTRProfile();
  return context->IsOffTheRecord();
}

content::BrowserContext* ChromeExtensionsBrowserClient::GetOffTheRecordContext(
    content::BrowserContext* context) {
  // TODO(mshin): Support Profile for Embedder
  // return static_cast<Profile*>(context)->GetPrimaryOTRProfile(
  //     /*create_if_needed=*/true);
  return context->IsOffTheRecord() ? context : context->GetOTRBrowserContext();
}

content::BrowserContext* ChromeExtensionsBrowserClient::GetOriginalContext(
    content::BrowserContext* context) {
  DCHECK(context);

  // TODO(mshin): Support Profile for Embedder
  // return static_cast<Profile*>(context)->GetOriginalProfile();
  return delegate_->GetOriginalBrowserContext(context);
}

content::BrowserContext*
ChromeExtensionsBrowserClient::GetContextRedirectedToOriginal(
    content::BrowserContext* context,
    bool force_guest_profile) {
  // TODO(mshin): Support Profile for Embedder
  // ProfileSelections::Builder builder;
  // builder.WithRegular(ProfileSelection::kRedirectedToOriginal);
  // if (force_guest_profile) {
  //   builder.WithGuest(ProfileSelection::kRedirectedToOriginal);
  // }

  // const ProfileSelections selections = builder.Build();
  // return selections.ApplyProfileSelection(Profile::FromBrowserContext(context));
  return delegate_->GetOriginalBrowserContext(context);
}

content::BrowserContext* ChromeExtensionsBrowserClient::GetContextOwnInstance(
    content::BrowserContext* context,
    bool force_guest_profile) {
  // TODO(mshin): Support Profile for Embedder
  // ProfileSelections::Builder builder;
  // builder.WithRegular(ProfileSelection::kOwnInstance);
  // if (force_guest_profile) {
  //   builder.WithGuest(ProfileSelection::kOwnInstance);
  // }

  // const ProfileSelections selections = builder.Build();
  // return selections.ApplyProfileSelection(Profile::FromBrowserContext(context));
  return context;
}

content::BrowserContext*
ChromeExtensionsBrowserClient::GetContextForOriginalOnly(
    content::BrowserContext* context,
    bool force_guest_profile) {
  // TODO(mshin): Support Profile for Embedder
  // ProfileSelections::Builder builder;
  // if (force_guest_profile) {
  //   builder.WithGuest(ProfileSelection::kOriginalOnly);
  // }

  // ProfileSelections selections = builder.Build();
  // return selections.ApplyProfileSelection(Profile::FromBrowserContext(context));
  return context;
}

bool ChromeExtensionsBrowserClient::AreExtensionsDisabledForContext(
    content::BrowserContext* context) {
  return false;
}

bool ChromeExtensionsBrowserClient::IsGuestSession(
    content::BrowserContext* context) const {
  // TODO(mshin): Support Profile for Embedder
  // return static_cast<Profile*>(context)->IsGuestSession();
  return false;
}

bool ChromeExtensionsBrowserClient::IsExtensionIncognitoEnabled(
    const ExtensionId& extension_id,
    content::BrowserContext* context) const {
  return IsGuestSession(context) ||
         extensions::util::IsIncognitoEnabled(extension_id, context);
}

bool ChromeExtensionsBrowserClient::CanExtensionCrossIncognito(
    const Extension* extension,
    content::BrowserContext* context) const {
  return IsGuestSession(context) || extensions::util::CanCrossIncognito(extension, context);
}

base::FilePath ChromeExtensionsBrowserClient::GetBundleResourcePath(
    const network::ResourceRequest& request,
    const base::FilePath& extension_resources_path,
    int* resource_id) const {
  return chrome_url_request_util::GetBundleResourcePath(
      request, extension_resources_path, resource_id);
}

void ChromeExtensionsBrowserClient::LoadResourceFromResourceBundle(
    const network::ResourceRequest& request,
    mojo::PendingReceiver<network::mojom::URLLoader> loader,
    const base::FilePath& resource_relative_path,
    int resource_id,
    scoped_refptr<net::HttpResponseHeaders> headers,
    mojo::PendingRemote<network::mojom::URLLoaderClient> client) {
  chrome_url_request_util::LoadResourceFromResourceBundle(
      request, std::move(loader), resource_relative_path, resource_id,
      std::move(headers), std::move(client));
}

bool ChromeExtensionsBrowserClient::AllowCrossRendererResourceLoad(
    const network::ResourceRequest& request,
    network::mojom::RequestDestination destination,
    ui::PageTransition page_transition,
    int child_id,
    bool is_incognito,
    const Extension* extension,
    const ExtensionSet& extensions,
    const ProcessMap& process_map) {
  bool allowed = false;
  if (chrome_url_request_util::AllowCrossRendererResourceLoad(
          request, destination, page_transition, child_id, is_incognito,
          extension, extensions, process_map, &allowed)) {
    return allowed;
  }

  // Couldn't determine if resource is allowed. Block the load.
  return false;
}

PrefService* ChromeExtensionsBrowserClient::GetPrefServiceForContext(
    content::BrowserContext* context) {
  return delegate_->GetPrefServiceForContext(context);
}

void ChromeExtensionsBrowserClient::GetEarlyExtensionPrefsObservers(
    content::BrowserContext* context,
    std::vector<EarlyExtensionPrefsObserver*>* observers) const {
  // TODO(mshin): Enable the below code after migrating ContentSettingsService  
  // observers->push_back(ContentSettingsService::Get(context));
}

ProcessManagerDelegate*
ChromeExtensionsBrowserClient::GetProcessManagerDelegate() const {
  return process_manager_delegate_.get();
}

mojo::PendingRemote<network::mojom::URLLoaderFactory>
ChromeExtensionsBrowserClient::GetControlledFrameEmbedderURLLoader(
    int frame_tree_node_id,
    content::BrowserContext* browser_context) {
  // Not support Web APP
  // return web_app::IsolatedWebAppURLLoaderFactory::Create(frame_tree_node_id,
  //                                                        browser_context);
  return {};
}

std::unique_ptr<ExtensionHostDelegate>
ChromeExtensionsBrowserClient::CreateExtensionHostDelegate() {
  return std::unique_ptr<ExtensionHostDelegate>(
      new ChromeExtensionHostDelegate);
}

bool ChromeExtensionsBrowserClient::DidVersionUpdate(
    content::BrowserContext* context) {
  // Unit tests may not provide prefs; assume everything is up to date.
  ExtensionPrefs* extension_prefs = ExtensionPrefs::Get(context);
  if (!extension_prefs) {
    return false;
  }

  if (g_did_chrome_update_for_testing) {
    return true;
  }

  // If we're inside a browser test, then assume prefs are all up to date.
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          ::switches::kTestType)) {
    return false;
  }

  PrefService* pref_service = extension_prefs->pref_service();
  base::Version last_version;
  if (pref_service->HasPrefPath(extensions::pref_names::kLastChromeVersion)) {
    std::string last_version_str =
        pref_service->GetString(extensions::pref_names::kLastChromeVersion);
    last_version = base::Version(last_version_str);
  }

  std::string current_version_str(version_info::GetVersionNumber());
  const base::Version& current_version = version_info::GetVersion();
  pref_service->SetString(extensions::pref_names::kLastChromeVersion, current_version_str);

  // If there was no version string in prefs, assume we're out of date.
  if (!last_version.IsValid()) {
    return true;
  }
  // If the current version string is invalid, assume we didn't update.
  if (!current_version.IsValid()) {
    return false;
  }

  return last_version < current_version;
}

void ChromeExtensionsBrowserClient::PermitExternalProtocolHandler() {
  // TODO(mshin): Enable the below code after supporting external protocol handler
  // ExternalProtocolHandler::PermitLaunchUrl();
}

bool ChromeExtensionsBrowserClient::IsInDemoMode() {
  return false;
}

bool ChromeExtensionsBrowserClient::IsScreensaverInDemoMode(
    const std::string& app_id) {
  return false;
}

bool ChromeExtensionsBrowserClient::IsRunningInForcedAppMode() {
  // TODO(mshin): Enable the below code after supporting kiosk mode
  // return chrome::IsRunningInForcedAppMode();
  return false;
}

bool ChromeExtensionsBrowserClient::IsAppModeForcedForApp(
    const ExtensionId& extension_id) {
  // TODO(mshin): Enable the below code after supporting kiosk mode
  // return chrome::IsRunningInForcedAppModeForApp(extension_id);
  return false;
}

bool ChromeExtensionsBrowserClient::IsLoggedInAsPublicAccount() {
  return false;
}

ExtensionSystemProvider*
ChromeExtensionsBrowserClient::GetExtensionSystemFactory() {
  return ExtensionSystemFactory::GetInstance();
}

void ChromeExtensionsBrowserClient::RegisterBrowserInterfaceBindersForFrame(
    mojo::BinderMapWithContext<content::RenderFrameHost*>* binder_map,
    content::RenderFrameHost* render_frame_host,
    const Extension* extension) const {
  PopulateExtensionFrameBinders(binder_map, render_frame_host, extension);
}

std::unique_ptr<RuntimeAPIDelegate>
ChromeExtensionsBrowserClient::CreateRuntimeAPIDelegate(
    content::BrowserContext* context) const {
  // TODO(mshin): Enable the below code after migrating runtime API
  // return std::unique_ptr<RuntimeAPIDelegate>(
  //     new ChromeRuntimeAPIDelegate(context));
  return nullptr;
}

const ComponentExtensionResourceManager*
ChromeExtensionsBrowserClient::GetComponentExtensionResourceManager() {
  return resource_manager_.get();
}

void ChromeExtensionsBrowserClient::BroadcastEventToRenderers(
    extensions::events::HistogramValue histogram_value,
    const std::string& event_name,
    base::Value::List args,
    bool dispatch_to_off_the_record_profiles) {
  delegate_->extension_event_router_forwarder()
      ->BroadcastEventToRenderers(histogram_value, event_name, std::move(args),
                                  GURL(), dispatch_to_off_the_record_profiles);
}

ExtensionCache* ChromeExtensionsBrowserClient::GetExtensionCache() {
  if (!extension_cache_.get()) {
    extension_cache_ = std::make_unique<NullExtensionCache>();
  }
  return extension_cache_.get();
}

bool ChromeExtensionsBrowserClient::IsBackgroundUpdateAllowed() {
  return !base::CommandLine::ForCurrentProcess()->HasSwitch(
      ::switches::kDisableBackgroundNetworking);
}

bool ChromeExtensionsBrowserClient::IsMinBrowserVersionSupported(
    const std::string& min_version) {
  const base::Version& browser_version = version_info::GetVersion();
  base::Version browser_min_version(min_version);
  return !browser_version.IsValid() || !browser_min_version.IsValid() ||
         browser_min_version.CompareTo(browser_version) <= 0;
}

ExtensionWebContentsObserver*
ChromeExtensionsBrowserClient::GetExtensionWebContentsObserver(
    content::WebContents* web_contents) {
  return ChromeExtensionWebContentsObserver::FromWebContents(web_contents);
}

void ChromeExtensionsBrowserClient::ReportError(
    content::BrowserContext* context,
    std::unique_ptr<ExtensionError> error) {
  ErrorConsole::Get(context)->ReportError(std::move(error));
}

void ChromeExtensionsBrowserClient::CleanUpWebView(
    content::BrowserContext* browser_context,
    int embedder_process_id,
    int view_instance_id) {
  // Not support WebView
}

void ChromeExtensionsBrowserClient::ClearBackForwardCache() {
  // TODO(mshin): Enable the below code after migrating ExtensionTabUtil
  // ExtensionTabUtil::ClearBackForwardCache();
}

void ChromeExtensionsBrowserClient::AttachExtensionTaskManagerTag(
    content::WebContents* web_contents,
    extensions::mojom::ViewType view_type) {
  switch (view_type) {
    case extensions::mojom::ViewType::kAppWindow:
    case extensions::mojom::ViewType::kComponent:
    case extensions::mojom::ViewType::kExtensionBackgroundPage:
    case extensions::mojom::ViewType::kExtensionPopup:
    case extensions::mojom::ViewType::kOffscreenDocument:
    case extensions::mojom::ViewType::kExtensionSidePanel:
      // TODO(mshin): Enable the below code after supporting Task Manager
      // These are the only types that are tracked by the ExtensionTag.
      // task_manager::WebContentsTags::CreateForExtension(web_contents,
      //                                                   view_type);
      return;

    case extensions::mojom::ViewType::kBackgroundContents:
    case extensions::mojom::ViewType::kExtensionGuest:
    case extensions::mojom::ViewType::kTabContents:
      // Those types are tracked by other tags:
      // BACKGROUND_CONTENTS --> task_manager::BackgroundContentsTag.
      // GUEST --> ChromeGuestViewManagerDelegate.
      // PANEL --> task_manager::PanelTag.
      // TAB_CONTENTS --> task_manager::TabContentsTag.
      // These tags are created and attached to the web_contents in other
      // locations, and they must be ignored here.
      return;

    case extensions::mojom::ViewType::kInvalid:
      NOTREACHED();
      return;
  }
}

scoped_refptr<update_client::UpdateClient>
ChromeExtensionsBrowserClient::CreateUpdateClient(
    content::BrowserContext* context) {
  // TODO(mshin): Enable the below code after migrating Updater
  // std::optional<GURL> override_url;
  // GURL update_url = extension_urls::GetWebstoreUpdateUrl();
  // if (update_url != extension_urls::GetDefaultWebstoreUpdateUrl()) {
  //   if (update_url.path() == kCrxUrlPath) {
  //     override_url = update_url.GetWithEmptyPath().Resolve(kJsonUrlPath);
  //   } else {
  //     override_url = update_url;
  //   }
  // }
  // return update_client::UpdateClientFactory(
  //     ChromeUpdateClientConfig::Create(context, override_url));
  return nullptr;
}

std::unique_ptr<ScopedExtensionUpdaterKeepAlive>
ChromeExtensionsBrowserClient::CreateUpdaterKeepAlive(
    content::BrowserContext* context) {
  // TODO(mshin): Enable the below code after migrating Updater & KeepyAlive
  // return std::make_unique<UpdaterKeepAlive>(
  //     context, ProfileKeepAliveOrigin::kExtensionUpdater);
  return nullptr;
}

bool ChromeExtensionsBrowserClient::IsActivityLoggingEnabled(
    content::BrowserContext* context) {
  // TODO(mshin): Enable the below code after migrating ActivityLog
  // ActivityLog* activity_log = ActivityLog::GetInstance(context);
  // return activity_log && activity_log->is_active();
  return false;
}

void ChromeExtensionsBrowserClient::GetTabAndWindowIdForWebContents(
    content::WebContents* web_contents,
    int* tab_id,
    int* window_id) {
  sessions::SessionTabHelper* session_tab_helper =
      sessions::SessionTabHelper::FromWebContents(web_contents);
  if (session_tab_helper) {
    *tab_id = session_tab_helper->session_id().id();
    *window_id = session_tab_helper->window_id().id();
  } else {
    *tab_id = -1;
    *window_id = -1;
  }
}

KioskDelegate* ChromeExtensionsBrowserClient::GetKioskDelegate() {
  if (!kiosk_delegate_) {
    // TODO(mshin): Enable the below code after migrating ChromeKioskDelegate
    // kiosk_delegate_ = std::make_unique<ChromeKioskDelegate>();
  }
  return kiosk_delegate_.get();
}

bool ChromeExtensionsBrowserClient::IsLockScreenContext(
    content::BrowserContext* context) {
  return false;
}

std::string ChromeExtensionsBrowserClient::GetApplicationLocale() {
  return delegate_->GetApplicationLocale();
}

bool ChromeExtensionsBrowserClient::IsExtensionEnabled(
    const ExtensionId& extension_id,
    content::BrowserContext* context) const {
  return ExtensionSystem::Get(context)->extension_service()->IsExtensionEnabled(
      extension_id);
}

bool ChromeExtensionsBrowserClient::IsWebUIAllowedToMakeNetworkRequests(
    const url::Origin& origin) {
  // TODO(mshin): Enable the below code after support WebUI
  // return ChromeWebUIControllerFactory::IsWebUIAllowedToMakeNetworkRequests(
  //     origin);
  return false;
}

network::mojom::NetworkContext*
ChromeExtensionsBrowserClient::GetSystemNetworkContext() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  return delegate_->GetNetworkContext();
}


UserScriptListener* ChromeExtensionsBrowserClient::GetUserScriptListener() {
  if (!user_script_listener_) {
    user_script_listener_ = std::make_unique<UserScriptListener>();
  }

  return user_script_listener_.get();
}

void ChromeExtensionsBrowserClient::SignalContentScriptsLoaded(
    content::BrowserContext* context) {
  GetUserScriptListener()->OnScriptsLoaded(context);
}

std::string ChromeExtensionsBrowserClient::GetUserAgent() const {
  return embedder_support::GetUserAgent();
}

bool ChromeExtensionsBrowserClient::ShouldSchemeBypassNavigationChecks(
    const std::string& scheme) const {
  if (scheme == kChromeSearchScheme) {
    return true;
  }

  return ExtensionsBrowserClient::ShouldSchemeBypassNavigationChecks(scheme);
}

base::FilePath ChromeExtensionsBrowserClient::GetSaveFilePath(
    content::BrowserContext* context) {
  // TODO(mshin): Enable the below code after migrating download API
  // DownloadPrefs* download_prefs = DownloadPrefs::FromBrowserContext(context);
  // return download_prefs->SaveFilePath();
  return base::FilePath();
}

void ChromeExtensionsBrowserClient::SetLastSaveFilePath(
    content::BrowserContext* context,
    const base::FilePath& path) {
  // TODO(mshin): Enable the below code after migrating download API
  // DownloadPrefs* download_prefs = DownloadPrefs::FromBrowserContext(context);
  // download_prefs->SetSaveFilePath(path);
}

bool ChromeExtensionsBrowserClient::HasIsolatedStorage(
    const ExtensionId& extension_id,
    content::BrowserContext* context) {
  return components_extensions::util::HasIsolatedStorage(
      extension_id, context);
}

bool ChromeExtensionsBrowserClient::IsScreenshotRestricted(
    content::WebContents* web_contents) const {
  return false;
}

bool ChromeExtensionsBrowserClient::IsValidTabId(
    content::BrowserContext* context,
    int tab_id) const {
  // TODO(mshin): Enable the below code after migrating ExtensionTabUtil
  // return ExtensionTabUtil::GetTabById(
  //     tab_id, context, true /* include_incognito */, nullptr /* contents */);
  return false;
}

void ChromeExtensionsBrowserClient::NotifyExtensionApiTabExecuteScript(
    content::BrowserContext* context,
    const ExtensionId& extension_id,
    const std::string& code) const {
  // TODO(mshin): Enable the below code after support safe browsing
  // auto* telemetry_service =
  //     safe_browsing::ExtensionTelemetryServiceFactory::GetForProfile(
  //         Profile::FromBrowserContext(context));
  // if (!telemetry_service || !telemetry_service->enabled() ||
  //     !base::FeatureList::IsEnabled(
  //         safe_browsing::kExtensionTelemetryTabsExecuteScriptSignal)) {
  //   return;
  // }

  // auto signal = std::make_unique<safe_browsing::TabsExecuteScriptSignal>(
  //     extension_id, code);
  // telemetry_service->AddSignal(std::move(signal));
}

bool ChromeExtensionsBrowserClient::IsExtensionTelemetryServiceEnabled(
    content::BrowserContext* context) const {
  // TODO(mshin): Enable the below code after support safe browsing
  // auto* telemetry_service =
  //     safe_browsing::ExtensionTelemetryServiceFactory::GetForProfile(
  //         Profile::FromBrowserContext(context));
  // return telemetry_service && telemetry_service->enabled();
  return false; 
}

void ChromeExtensionsBrowserClient::NotifyExtensionApiDeclarativeNetRequest(
    content::BrowserContext* context,
    const ExtensionId& extension_id,
    const std::vector<extensions::api::declarative_net_request::Rule>& rules) const {
  // TODO(mshin): Enable the below code after support safe browsing
  // auto* telemetry_service =
  //     safe_browsing::ExtensionTelemetryServiceFactory::GetForProfile(
  //         Profile::FromBrowserContext(context));
  // if (!telemetry_service || !telemetry_service->enabled()) {
  //   return;
  // }

  // // The telemetry service will consume and release the signal object inside the
  // // `AddSignal()` call.
  // auto signal = std::make_unique<safe_browsing::DeclarativeNetRequestSignal>(
  //     extension_id, rules);
  // telemetry_service->AddSignal(std::move(signal));
}

void ChromeExtensionsBrowserClient::NotifyExtensionRemoteHostContacted(
    content::BrowserContext* context,
    const ExtensionId& extension_id,
    const GURL& url) const {
  // Collect only if new interception feature is disabled to avoid duplicates.
  if (base::FeatureList::IsEnabled(
          safe_browsing::
              kExtensionTelemetryInterceptRemoteHostsContactedInRenderer)) {
    return;
  }

  // TODO(mshin): Enable the below code after support safe browsing
  // safe_browsing::RemoteHostInfo::ProtocolType protocol =
  //     safe_browsing::RemoteHostInfo::UNSPECIFIED;
  // if (base::FeatureList::IsEnabled(
  //         safe_browsing::kExtensionTelemetryReportContactedHosts) &&
  //     url.SchemeIsHTTPOrHTTPS()) {
  //   protocol = safe_browsing::RemoteHostInfo::HTTP_HTTPS;
  // } else if (base::FeatureList::IsEnabled(
  //                safe_browsing::
  //                    kExtensionTelemetryReportHostsContactedViaWebSocket) &&
  //            url.SchemeIsWSOrWSS()) {
  //   protocol = safe_browsing::RemoteHostInfo::WEBSOCKET;
  // } else {
  //   return;
  // }
  // auto* telemetry_service =
  //     safe_browsing::ExtensionTelemetryServiceFactory::GetForProfile(
  //         Profile::FromBrowserContext(context));
  // if (!telemetry_service || !telemetry_service->enabled()) {
  //   return;
  // }
  // auto remote_host_signal =
  //     std::make_unique<safe_browsing::RemoteHostContactedSignal>(extension_id,
  //                                                                url, protocol);
  // telemetry_service->AddSignal(std::move(remote_host_signal));
}

// static
void ChromeExtensionsBrowserClient::set_did_chrome_update_for_testing(
    bool did_update) {
  g_did_chrome_update_for_testing = did_update;
}

bool ChromeExtensionsBrowserClient::IsUsbDeviceAllowedByPolicy(
    content::BrowserContext* context,
    const ExtensionId& extension_id,
    int vendor_id,
    int product_id) const {
  // Not support USB API
  return false;
}

void ChromeExtensionsBrowserClient::GetFavicon(
    content::BrowserContext* browser_context,
    const Extension* extension,
    const GURL& url,
    base::CancelableTaskTracker* tracker,
    base::OnceCallback<void(scoped_refptr<base::RefCountedMemory> bitmap_data)>
        callback) const {
  // TODO(mshin): Enable the below code after migrating fivicon_util.cc
  // favicon_util::GetFaviconForExtensionRequest(browser_context, extension, url,
  //                                             tracker, std::move(callback));
}

std::vector<content::BrowserContext*>
ChromeExtensionsBrowserClient::GetRelatedContextsForExtension(
    content::BrowserContext* browser_context,
    const Extension& extension) const {
  return components_extensions::util::GetAllRelatedProfiles(
      browser_context, extension);
}

void ChromeExtensionsBrowserClient::AddAdditionalAllowedHosts(
    const PermissionSet& desired_permissions,
    PermissionSet* granted_permissions) const {
  auto get_new_host_patterns = [](const URLPatternSet& desired_patterns,
                                  const URLPatternSet& granted_patterns) {
    URLPatternSet new_patterns = granted_patterns.Clone();
    for (const URLPattern& pattern : desired_patterns) {
      // The chrome://favicon permission is special. It is requested by
      // extensions to access stored favicons, but is not a traditional
      // host permission. Since it cannot be reasonably runtime-granted
      // while the user is on the site (i.e., the user never visits
      // chrome://favicon/), we auto-grant it and treat it like an API
      // permission.
      bool is_chrome_favicon = pattern.scheme() == content::kChromeUIScheme &&
                               pattern.host() == kChromeUIFaviconHost;
      if (is_chrome_favicon) {
        new_patterns.AddPattern(pattern);
      }
    }
    return new_patterns;
  };

  URLPatternSet new_explicit_hosts =
      get_new_host_patterns(desired_permissions.explicit_hosts(),
                            granted_permissions->explicit_hosts());
  URLPatternSet new_scriptable_hosts =
      get_new_host_patterns(desired_permissions.scriptable_hosts(),
                            granted_permissions->scriptable_hosts());
  granted_permissions->SetExplicitHosts(std::move(new_explicit_hosts));
  granted_permissions->SetScriptableHosts(std::move(new_scriptable_hosts));
}

void ChromeExtensionsBrowserClient::AddAPIActionToActivityLog(
    content::BrowserContext* browser_context,
    const ExtensionId& extension_id,
    const std::string& call_name,
    base::Value::List args,
    const std::string& extra) {
  // TODO(mshin): Enable the below code after migrating ActivityLog
  // AddAPIActionOrEventToActivityLog(browser_context, extension_id,
  //                                  Action::ACTION_API_CALL, call_name,
  //                                  std::move(args), extra);
}

void ChromeExtensionsBrowserClient::AddEventToActivityLog(
    content::BrowserContext* browser_context,
    const ExtensionId& extension_id,
    const std::string& call_name,
    base::Value::List args,
    const std::string& extra) {
  // TODO(mshin): Enable the below code after migrating ActivityLog
  // AddAPIActionOrEventToActivityLog(browser_context, extension_id,
  //                                  Action::ACTION_API_EVENT, call_name,
  //                                  std::move(args), extra);
}

void ChromeExtensionsBrowserClient::AddDOMActionToActivityLog(
    content::BrowserContext* browser_context,
    const ExtensionId& extension_id,
    const std::string& call_name,
    base::Value::List args,
    const GURL& url,
    const std::u16string& url_title,
    int call_type) {
  if (!ShouldLogExtensionAction(browser_context, extension_id)) {
    return;
  }

  // TODO(mshin): Enable the below code after migrating ActivityLog
  // auto action = base::MakeRefCounted<Action>(
  //     extension_id, base::Time::Now(), Action::ACTION_DOM_ACCESS, call_name);
  // action->set_args(std::move(args));
  // action->set_page_url(url);
  // action->set_page_title(base::UTF16ToUTF8(url_title));
  // action->mutable_other().Set(activity_log_constants::kActionDomVerb,
  //                             call_type);
  // AddActionToExtensionActivityLog(browser_context, action);
}

// TODO(mshin): Enable the below code after migrating ActivityLog
// void ChromeExtensionsBrowserClient::AddAPIActionOrEventToActivityLog(
//     content::BrowserContext* browser_context,
//     const ExtensionId& extension_id,
//     extensions::Action::ActionType action_type,
//     const std::string& call_name,
//     base::Value::List args,
//     const std::string& extra) {
//   if (!ShouldLogExtensionAction(browser_context, extension_id)) {
//     return;
//   }

//   TODO(mshin): Enable the below code after migrating ActivityLog
//   auto action = base::MakeRefCounted<Action>(extension_id, base::Time::Now(),
//                                              action_type, call_name);
//   action->set_args(std::move(args));
//   if (!extra.empty()) {
//     action->mutable_other().Set(activity_log_constants::kActionExtra, extra);
//   }
//   AddActionToExtensionActivityLog(browser_context, action);
// }

void ChromeExtensionsBrowserClient::GetWebViewStoragePartitionConfig(
    content::BrowserContext* browser_context,
    content::SiteInstance* owner_site_instance,
    const std::string& partition_name,
    bool in_memory,
    base::OnceCallback<void(std::optional<content::StoragePartitionConfig>)>
        callback) {
  ExtensionsBrowserClient::GetWebViewStoragePartitionConfig(
      browser_context, owner_site_instance, partition_name, in_memory,
      std::move(callback));
}

void ChromeExtensionsBrowserClient::CreatePasswordReuseDetectionManager(
    content::WebContents* web_contents) const {
  // TODO(mshin): Enable the below code after supporting safe browsing
  // ChromePasswordReuseDetectionManagerClient::CreateForWebContents(web_contents);
}

media_device_salt::MediaDeviceSaltService*
ChromeExtensionsBrowserClient::GetMediaDeviceSaltService(
    content::BrowserContext* context) {
  // Not support Media Device Salt
  return nullptr;
}

}  // namespace components_extensions
