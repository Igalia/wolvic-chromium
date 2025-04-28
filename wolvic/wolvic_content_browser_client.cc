// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/wolvic_content_browser_client.h"
#include <memory>

#include "base/path_service.h"
#include "components/cdm/browser/media_drm_storage_impl.h"
#include "components/embedder_support/user_agent_utils.h"
#include "components/password_manager/content/browser/content_password_manager_driver_factory.h"
#include "components/prefs/pref_service.h"
#include "components/site_isolation/preloaded_isolated_origins.h"
#include "content/public/browser/browser_url_handler.h"
#include "content/browser/loader/file_url_loader_factory.h"
#include "content/public/browser/permission_controller.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/window_container_type.mojom-shared.h"
#include "content/shell/browser/shell.h"
#include "content/shell/browser/shell_devtools_manager_delegate.h"
#include "media/mojo/mojom/media_drm_storage.mojom.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "wolvic/browser/dialogs/http_auth_manager.h"
#include "wolvic/browser/service_tab_launcher.h"
#include "wolvic/browser/session_settings.h"
#include "wolvic/browser/wolvic_browser_interface_binders.h"
#include "wolvic/wolvic_browser_context.h"
#include "wolvic/wolvic_content_main_delegate.h"
#include "wolvic/wolvic_main_parts.h"
#include "third_party/blink/public/common/permissions/permission_utils.h"

#include "components/extensions/common/buildflags.h"
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
#include "content/public/browser/file_url_loader.h"
#include "content/browser/loader/file_url_loader_factory.h"
#include "content/public/browser/site_isolation_policy.h"
#include "content/public/browser/web_ui_url_loader_factory.h"
#include "content/public/common/url_constants.h"
#include "components/extensions/browser/chrome_extension_web_contents_observer.h"
#include "components/extensions/common/extension_constants.h"
#include "extensions/browser/api/web_request/web_request_api.h"
#include "extensions/browser/api/web_request/web_request_proxying_webtransport.h"
#include "extensions/browser/extension_navigation_throttle.h"
#include "extensions/browser/extension_protocols.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_util.h"
#include "extensions/browser/guest_view/web_view/web_view_guest.h"
#include "extensions/browser/guest_view/web_view/web_view_permission_helper.h"
#include "extensions/browser/guest_view/web_view/web_view_renderer_state.h"
#include "extensions/browser/process_map.h"
#include "extensions/browser/script_injection_tracker.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_set.h"
#include "extensions/common/manifest_handlers/background_info.h"
#include "extensions/common/permissions/permissions_data.h"
#include "extensions/common/switches.h"
#include "extensions/browser/browser_frame_context_data.h"
#include "services/network/public/cpp/self_deleting_url_loader_factory.h"
#include "third_party/blink/public/mojom/webpreferences/web_preferences.mojom.h"

using blink::web_pref::WebPreferences;
using content::AllowServiceWorkerResult;
using content::BrowserThread;
using content::BrowserURLHandler;
using content::ContentBrowserClient;
using content::RenderFrameHost;
using content::SiteInstance;
using components_extensions::ChromeContentBrowserClientExtensionsPart;
using components_extensions::ChromeExtensionWebContentsObserver;
using extensions::APIPermission;
using extensions::Extension;
using extensions::Manifest;
using extensions::mojom::APIPermissionID;
#endif

namespace wolvic {

namespace {

WolvicContentBrowserClient* g_instance = nullptr;

void CreateOriginId(cdm::MediaDrmStorageImpl::OriginIdObtainedCB callback) {
  std::move(callback).Run(true, base::UnguessableToken::Create());
}

void AllowEmptyOriginIdCB(base::OnceCallback<void(bool)> callback) {
  // Since CreateOriginId() always returns a non-empty origin ID, we don't need
  // to allow empty origin ID.
  std::move(callback).Run(false);
}

void CreateMediaDrmStorage(
    content::RenderFrameHost* render_frame_host,
    mojo::PendingReceiver<::media::mojom::MediaDrmStorage> receiver) {
  CHECK(render_frame_host);

  if (render_frame_host->GetLastCommittedOrigin().opaque()) {
    LOG(ERROR) << __func__ << ": Unique origin.";
    return;
  }

  auto* wolvic_browser_context = static_cast<WolvicBrowserContext*>(
      render_frame_host->GetBrowserContext());
  CHECK(wolvic_browser_context) << "WolvicBrowserContext not available.";

  PrefService* pref_service = wolvic_browser_context->GetPrefService();
  CHECK(pref_service);

  // The object will be deleted on connection error, or when the frame navigates
  // away.
  new cdm::MediaDrmStorageImpl(
      *render_frame_host, pref_service, base::BindRepeating(&CreateOriginId),
      base::BindRepeating(&AllowEmptyOriginIdCB), std::move(receiver));
}

#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
// void MaybeAddThrottle(
//     std::unique_ptr<content::NavigationThrottle> maybe_throttle,
//     std::vector<std::unique_ptr<content::NavigationThrottle>>* throttles) {
//   if (maybe_throttle)
//     throttles->push_back(std::move(maybe_throttle));
// }


// The SpecialAccessFileURLLoaderFactory provided to the extension background
// pages.  Checks with the ChildProcessSecurityPolicy to validate the file
// access.
class SpecialAccessFileURLLoaderFactory
    : public network::SelfDeletingURLLoaderFactory {
 public:
  // Returns mojo::PendingRemote to a newly constructed
  // SpecialAccessFileURLLoaderFactory.  The factory is self-owned - it will
  // delete itself once there are no more receivers (including the receiver
  // associated with the returned mojo::PendingRemote and the receivers bound by
  // the Clone method).
  static mojo::PendingRemote<network::mojom::URLLoaderFactory> Create(
      int child_id) {
    mojo::PendingRemote<network::mojom::URLLoaderFactory> pending_remote;

    // The SpecialAccessFileURLLoaderFactory will delete itself when there are
    // no more receivers - see the
    // network::SelfDeletingURLLoaderFactory::OnDisconnect method.
    new SpecialAccessFileURLLoaderFactory(
        child_id, pending_remote.InitWithNewPipeAndPassReceiver());

    return pending_remote;
  }

  SpecialAccessFileURLLoaderFactory(const SpecialAccessFileURLLoaderFactory&) =
      delete;
  SpecialAccessFileURLLoaderFactory& operator=(
      const SpecialAccessFileURLLoaderFactory&) = delete;

 private:
  explicit SpecialAccessFileURLLoaderFactory(
      int child_id,
      mojo::PendingReceiver<network::mojom::URLLoaderFactory> factory_receiver)
      : network::SelfDeletingURLLoaderFactory(std::move(factory_receiver)),
        child_id_(child_id) {}

  // network::mojom::URLLoaderFactory:
  void CreateLoaderAndStart(
      mojo::PendingReceiver<network::mojom::URLLoader> loader,
      int32_t request_id,
      uint32_t options,
      const network::ResourceRequest& request,
      mojo::PendingRemote<network::mojom::URLLoaderClient> client,
      const net::MutableNetworkTrafficAnnotationTag& traffic_annotation)
      override {
    if (!content::ChildProcessSecurityPolicy::GetInstance()->CanRequestURL(
            child_id_, request.url)) {
      mojo::Remote<network::mojom::URLLoaderClient>(std::move(client))
          ->OnComplete(
              network::URLLoaderCompletionStatus(net::ERR_ACCESS_DENIED));
      return;
    }
    content::CreateFileURLLoaderBypassingSecurityChecks(
        request, std::move(loader), std::move(client),
        /*observer=*/nullptr,
        /* allow_directory_listing */ true);
  }

  int child_id_;
};

// Returns true if there is is an extension matching `url` in
// `render_process_id` with `permission`.
//
// GetExtensionOrAppByURL requires a full URL in order to match with a hosted
// app, even though normal extensions just use the host.
bool URLHasExtensionPermission(extensions::ProcessMap* process_map,
                               extensions::ExtensionRegistry* registry,
                               const GURL& url,
                               int render_process_id,
                               APIPermissionID permission) {
  // Includes web URLs that are part of an extension's web extent.
  const Extension* extension =
      registry->enabled_extensions().GetExtensionOrAppByURL(url);
  return extension &&
         extension->permissions_data()->HasAPIPermission(permission) &&
         process_map->Contains(extension->id(), render_process_id);
}

void InitializeFileURLLoaderFactoryForExtension(
    int render_process_id,
    content::BrowserContext* browser_context,
    const extensions::Extension* extension,
    ContentBrowserClient::NonNetworkURLLoaderFactoryMap* factories) {
  // Extensions with the necessary permissions get access to file:// URLs that
  // gets approval from ChildProcessSecurityPolicy. Keep this logic in sync with
  // ExtensionWebContentsObserver::RenderFrameCreated.
  Manifest::Type type = extension->GetType();
  if ((type == Manifest::TYPE_EXTENSION ||
      type == Manifest::TYPE_LEGACY_PACKAGED_APP) &&
      extensions::util::AllowFileAccess(extension->id(), browser_context)) {
    factories->emplace(
        url::kFileScheme,
        SpecialAccessFileURLLoaderFactory::Create(render_process_id));
  }
}

void AddChromeSchemeFactories(
    int render_process_id,
    content::RenderFrameHost* frame_host,
    content::WebContents* web_contents,
    const extensions::Extension* extension,
    ContentBrowserClient::NonNetworkURLLoaderFactoryMap* factories) {
  ChromeExtensionWebContentsObserver* web_observer =
      ChromeExtensionWebContentsObserver::FromWebContents(
          web_contents);
  // There is nothing to do if no ChromeExtensionWebContentsObserver is attached
  // to the |web_contents| or no enabled extension exists.
  if (!web_observer || !extension)
    return;

  std::vector<std::string> allowed_webui_hosts;
  // Support for chrome:// scheme if appropriate.
  if ((extension->is_extension() || extension->is_platform_app()) &&
      Manifest::IsComponentLocation(extension->location())) {
    // Components of chrome that are implemented as extensions or platform apps
    // are allowed to use chrome://resources/ and chrome://theme/ URLs.
    allowed_webui_hosts.emplace_back(content::kChromeUIResourcesHost);
    allowed_webui_hosts.emplace_back(components_extensions::kChromeUIThemeHost);
    // For testing purposes chrome://webui-test/ is also allowed.
    allowed_webui_hosts.emplace_back(components_extensions::kChromeUIWebUITestHost);
  }
  if (extension->is_extension() || extension->is_legacy_packaged_app() ||
      (extension->is_platform_app() &&
      Manifest::IsComponentLocation(extension->location()))) {
    // Extensions, legacy packaged apps, and component platform apps are allowed
    // to use chrome://favicon/, chrome://extension-icon/ and chrome://app-icon
    // URLs. Hosted apps are not allowed because they are served via web servers
    // (and are generally never given access to Chrome APIs).
    allowed_webui_hosts.emplace_back(components_extensions::kChromeUIExtensionIconHost);
    allowed_webui_hosts.emplace_back(components_extensions::kChromeUIFaviconHost);
    allowed_webui_hosts.emplace_back(components_extensions::kChromeUIAppIconHost);
  }
  if (!allowed_webui_hosts.empty()) {
    factories->emplace(content::kChromeUIScheme,
                      content::CreateWebUIURLLoaderFactory(
                          frame_host, content::kChromeUIScheme,
                          std::move(allowed_webui_hosts)));
  }
}
#endif

}  // namespace

WolvicContentBrowserClient::WolvicContentBrowserClient()
    : browser_main_parts_(nullptr) {
  DCHECK(!g_instance);
  g_instance = this;
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  extensions_part_ = std::make_unique<ChromeContentBrowserClientExtensionsPart>();
#endif
}

WolvicContentBrowserClient::~WolvicContentBrowserClient() {
  g_instance = nullptr;
}

// static
WolvicContentBrowserClient* WolvicContentBrowserClient::Get() {
  return g_instance;
}

content::BrowserContext* WolvicContentBrowserClient::browser_context() {
  return browser_main_parts_->browser_context();
}

content::BrowserContext*
WolvicContentBrowserClient::off_the_record_browser_context() {
  return browser_main_parts_->off_the_record_browser_context();
}

std::unique_ptr<content::BrowserMainParts>
WolvicContentBrowserClient::CreateBrowserMainParts(
    bool /* is_integration_test */) {
  CHECK(!browser_main_parts_);
  browser_main_parts_ = new WolvicMainParts();
  return std::unique_ptr<content::BrowserMainParts>(browser_main_parts_);
}

std::unique_ptr<content::DevToolsManagerDelegate>
WolvicContentBrowserClient::CreateDevToolsManagerDelegate() {
  return std::make_unique<content::ShellDevToolsManagerDelegate>(
      browser_context());
}

std::unique_ptr<content::LoginDelegate>
WolvicContentBrowserClient::CreateLoginDelegate(
    const net::AuthChallengeInfo& auth_info,
    content::WebContents* web_contents,
    content::BrowserContext* browser_context,
    const content::GlobalRequestID& request_id,
    bool is_request_for_primary_main_frame,
    const GURL& url,
    scoped_refptr<net::HttpResponseHeaders> response_headers,
    bool first_auth_attempt,
    LoginAuthRequiredCallback auth_required_callback) {
  return std::make_unique<HttpAuthManager>(auth_info, web_contents,
                                           first_auth_attempt,
                                           std::move(auth_required_callback));
}

#if BUILDFLAG(ENABLE_VR)
content::XrIntegrationClient*
WolvicContentBrowserClient::GetXrIntegrationClient() {
  if (!xr_integration_client_)
    xr_integration_client_ = std::make_unique<WolvicXrIntegrationClient>(
        base::PassKey<WolvicContentBrowserClient>());
  return xr_integration_client_.get();
}
#endif

std::string WolvicContentBrowserClient::GetUserAgent() {
  auto* settings = SessionSettings::Get();
  if (auto user_agent_override = settings->GetUserAgentOverride())
    return *user_agent_override;

  return settings->GetDefaultUserAgent(settings->GetUserAgentMode());
}

blink::UserAgentMetadata WolvicContentBrowserClient::GetUserAgentMetadata() {
  typedef SessionSettings::UserAgentMode UserAgentMode;

  auto metadata = embedder_support::GetUserAgentMetadata();
  auto user_agent_mode = SessionSettings::Get()->GetUserAgentMode();
  switch (user_agent_mode) {
    case UserAgentMode::kMobile:
    case UserAgentMode::kMobileVR:
      metadata.mobile = true;
      break;
    case UserAgentMode::kDesktop:
      metadata.mobile = false;
      break;
  }
  return metadata;
}

void WolvicContentBrowserClient::ConfigureNetworkContextParams(
    content::BrowserContext* context,
    bool in_memory,
    const base::FilePath& relative_partition_path,
    network::mojom::NetworkContextParams* network_context_params,
    cert_verifier::mojom::CertVerifierCreationParams*
        cert_verifier_creation_params) {
  base::FilePath user_data_path;
  base::PathService::Get(content::SHELL_DIR_USER_DATA, &user_data_path);
  network_context_params->file_paths = network::mojom::NetworkContextFilePaths::New();
  network_context_params->file_paths->http_cache_directory =
      user_data_path.Append(FILE_PATH_LITERAL("Cache"));

  // TODO: Set the desktop user agent by the default, and revisit this to set
  // the setting value if the payment request solves the UA issue.

  // These values will be used when the network requst has the empty http
  // header. All network requests created by renderer(web page) already have
  // the http header, so the value will be used only for the network requests
  // created by the native code like the payment request.
  auto* settings = SessionSettings::Get();
  network_context_params->user_agent =
      settings->GetDefaultUserAgent(SessionSettings::UserAgentMode::kDesktop);
  network_context_params->accept_language = "en-us,en";
}

void WolvicContentBrowserClient::BindMediaServiceReceiver(
    content::RenderFrameHost* render_frame_host,
    mojo::GenericPendingReceiver receiver) {
  if (auto r = receiver.As<media::mojom::MediaDrmStorage>()) {
    CreateMediaDrmStorage(render_frame_host, std::move(r));
  }
}

void WolvicContentBrowserClient::OpenURL(
    content::SiteInstance* site_instance,
    const content::OpenURLParams& params,
    base::OnceCallback<void(content::WebContents*)> callback) {
  content::BrowserContext* browser_context = site_instance->GetBrowserContext();
  // TODO (jfernandez): Explose an alternate approach based on the
  // TabModelJniBridge::HandlePopupNavigation
  ServiceTabLauncher::GetInstance()->LaunchTab(browser_context, params,
                                               std::move(callback));
}

void WolvicContentBrowserClient::
    RegisterAssociatedInterfaceBindersForRenderFrameHost(
    content::RenderFrameHost& render_frame_host,
    blink::AssociatedInterfaceRegistry& associated_registry) {
  associated_registry.AddInterface<
      autofill::mojom::PasswordManagerDriver>(base::BindRepeating(
      [](content::RenderFrameHost* render_frame_host,
         mojo::PendingAssociatedReceiver<autofill::mojom::PasswordManagerDriver>
             receiver) {
        password_manager::ContentPasswordManagerDriverFactory::
            BindPasswordManagerDriver(std::move(receiver), render_frame_host);
      },
      &render_frame_host));
}

void WolvicContentBrowserClient::RegisterBrowserInterfaceBindersForFrame(
    content::RenderFrameHost* render_frame_host,
    mojo::BinderMapWithContext<content::RenderFrameHost*>* map) {
  wolvic::internal::PopulateWolvicFrameBinders(map, render_frame_host);
}

void WolvicContentBrowserClient::SiteInstanceGotProcessAndSite(
    SiteInstance* site_instance) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  extensions_part_->SiteInstanceGotProcessAndSite(site_instance);
#endif
}

void WolvicContentBrowserClient::AppendExtraCommandLineSwitches(
    base::CommandLine* command_line,
    int child_process_id) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  const base::CommandLine& browser_command_line =
      *base::CommandLine::ForCurrentProcess();
  std::string process_type =
  command_line->GetSwitchValueASCII(switches::kProcessType);
  if (process_type == switches::kRendererProcess) {
    content::RenderProcessHost* process =
        content::RenderProcessHost::FromID(child_process_id);
    if (process) {
      extensions_part_->AppendExtraRendererCommandLineSwitches(command_line, *process);
    }
    // Please keep this in alphabetical order.
    static const char* const kSwitchNames[] = {
      extensions::switches::kAllowHTTPBackgroundPage,
      extensions::switches::kAllowLegacyExtensionManifests,
      extensions::switches::kDisableExtensionsHttpThrottling,
      extensions::switches::kEnableExperimentalExtensionApis,
      extensions::switches::kExtensionsOnChromeURLs,
      extensions::switches::kSetExtensionThrottleTestParams,  // For tests only.
      extensions::switches::kAllowlistedExtensionID,
    };
    command_line->CopySwitchesFrom(browser_command_line, kSwitchNames);
  } else if (process_type == switches::kUtilityProcess) {
    static const char* const kSwitchNames[] = {
        extensions::switches::kAllowHTTPBackgroundPage,
        extensions::switches::kEnableExperimentalExtensionApis,
        extensions::switches::kExtensionsOnChromeURLs,
        extensions::switches::kAllowlistedExtensionID,
    };

    command_line->CopySwitchesFrom(browser_command_line, kSwitchNames);
  }
#endif
}

content::AllowServiceWorkerResult
WolvicContentBrowserClient::AllowServiceWorker(
    const GURL& scope,
    const net::SiteForCookies& site_for_cookies,
    const std::optional<url::Origin>& top_frame_origin,
    const GURL& script_url,
    content::BrowserContext* context) {
  DCHECK(context);
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  GURL first_party_url = top_frame_origin ? top_frame_origin->GetURL() : GURL();

#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // Check if this is an extension-related service worker, and, if so, if it's
  // allowed (this can return false if, e.g., the extension is disabled).
  // If it's not allowed, return immediately. We deliberately do *not* report
  // to the PageSpecificContentSettings, since the service worker is blocked
  // because of the extension, rather than because of the user's content
  // settings.
  if (!ChromeContentBrowserClientExtensionsPart::AllowServiceWorker(
          scope, first_party_url, script_url, context)) {
    return content::AllowServiceWorkerResult::No();
  }
#endif
  return AllowServiceWorkerResult::Yes();
}

bool WolvicContentBrowserClient::MayDeleteServiceWorkerRegistration(
    const GURL& scope,
    content::BrowserContext* browser_context) {
  DCHECK(browser_context);
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  if (!ChromeContentBrowserClientExtensionsPart::
          MayDeleteServiceWorkerRegistration(scope, browser_context)) {
    return false;
  }
#endif

  return true;
}

bool WolvicContentBrowserClient::ShouldTryToUpdateServiceWorkerRegistration(
    const GURL& scope,
    content::BrowserContext* browser_context) {
  DCHECK(browser_context);
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  if (!ChromeContentBrowserClientExtensionsPart::
          ShouldTryToUpdateServiceWorkerRegistration(scope, browser_context)) {
    return false;
  }
#endif

  return true;
}

bool WolvicContentBrowserClient::DoesSchemeAllowCrossOriginSharedWorker(
    const std::string& scheme) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // Extensions are allowed to start cross-origin shared workers.
  if (scheme == extensions::kExtensionScheme)
    return true;
#endif

  return false;
}


bool WolvicContentBrowserClient::CanCreateWindow(
    RenderFrameHost* opener,
    const GURL& opener_url,
    const GURL& opener_top_level_frame_url,
    const url::Origin& source_origin,
    content::mojom::WindowContainerType container_type,
    const GURL& target_url,
    const content::Referrer& referrer,
    const std::string& frame_name,
    WindowOpenDisposition disposition,
    const blink::mojom::WindowFeatures& features,
    bool user_gesture,
    bool opener_suppressed,
    bool* no_javascript_access) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(opener);

  content::WebContents* web_contents =
      content::WebContents::FromRenderFrameHost(opener);
  content::BrowserContext* browser_context = web_contents->GetBrowserContext();
  DCHECK(browser_context);
  *no_javascript_access = false;

  // If the opener is trying to create a background window but doesn't have
  // the appropriate permission, fail the attempt.
  if (container_type == content::mojom::WindowContainerType::BACKGROUND) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
    auto* process_map = extensions::ProcessMap::Get(browser_context);
    auto* registry = extensions::ExtensionRegistry::Get(browser_context);
    if (!URLHasExtensionPermission(process_map, registry, opener_url,
                                   opener->GetProcess()->GetID(),
                                   APIPermissionID::kBackground)) {
      return false;
    }

    // Note: this use of GetExtensionOrAppByURL is safe but imperfect.  It may
    // return a recently installed Extension even if this CanCreateWindow call
    // was made by an old copy of the page in a normal web process.  That's ok,
    // because the permission check above would have caused an early return
    // already. We must use the full URL to find hosted apps, though, and not
    // just the origin.
    const Extension* extension =
        registry->enabled_extensions().GetExtensionOrAppByURL(opener_url);
    if (extension && !extensions::BackgroundInfo::AllowJSAccess(extension))
      *no_javascript_access = true;
#endif

    return true;
  }

#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  if (target_url.SchemeIs(extensions::kExtensionScheme)) {
    // Intentionally duplicating |registry| code from above because we want to
    // reduce calls to retrieve them as this function is a SYNC IPC handler.
    auto* registry = extensions::ExtensionRegistry::Get(browser_context);
    const Extension* extension =
        registry->enabled_extensions().GetExtensionOrAppByURL(target_url);
    if (extension && extension->is_platform_app()) {
      // window.open() may not be used to load v2 apps in a regular tab.
      return false;
    }
  }
#endif
  return true;
}

void WolvicContentBrowserClient::OverrideWebkitPrefs(
    content::WebContents* web_contents,
    WebPreferences* web_prefs) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  web_prefs->animation_policy =
      blink::mojom::ImageAnimationPolicy::kImageAnimationPolicyAllowed;

  extensions_part_->OverrideWebkitPrefs(web_contents, web_prefs);
#endif
}

bool WolvicContentBrowserClient::OverrideWebPreferencesAfterNavigation(
    content::WebContents* web_contents,
    WebPreferences* web_prefs) {
  bool prefs_changed = false;
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  prefs_changed |=
      extensions_part_->OverrideWebPreferencesAfterNavigation(
            web_contents, web_prefs);
#endif
  return prefs_changed;
}

void WolvicContentBrowserClient::BrowserURLHandlerCreated(
  BrowserURLHandler* handler) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  extensions_part_->BrowserURLHandlerCreated(handler);
#endif
}

void WolvicContentBrowserClient::GetAdditionalAllowedSchemesForFileSystem(
    std::vector<std::string>* additional_allowed_schemes) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  extensions_part_->GetAdditionalAllowedSchemesForFileSystem(
      additional_allowed_schemes);
#endif
}

content::StoragePartitionConfig
WolvicContentBrowserClient::GetStoragePartitionConfigForSite(
    content::BrowserContext* browser_context,
    const GURL& site) {
  // Default to the browser-wide storage partition and override based on |site|
  // below.
  content::StoragePartitionConfig default_storage_partition_config =
      content::StoragePartitionConfig::CreateDefault(browser_context);

  // A non-default storage partition is used in the following situations:
  // - To enforce process isolation between a more-trusted content (Chrome Apps,
  // Extensions, and Isolated Web Apps) and regular web content.
  // - For the <webview> tag, which Chrome Apps, Isolated Web Apps and WebUI use
  // to create temporary storage buckets for loading various kinds of web
  // content.
  //
  // In general, those use cases aren't considered part of the user's normal
  // browsing activity.
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  if (site.SchemeIs(extensions::kExtensionScheme)) {
    // The host in an extension site URL is the extension_id.
    CHECK(site.has_host());
    return extensions::util::GetStoragePartitionConfigForExtensionId(
        site.host(), browser_context);
  }
#endif

  return default_storage_partition_config;
}


GURL WolvicContentBrowserClient::GetEffectiveURL(
    content::BrowserContext* browser_context,
    const GURL& url) {
#if BUILDFLAG(ENABLE_EXTENSIONS)
  return ChromeContentBrowserClientExtensionsPart::GetEffectiveURL(browser_context,
                                                                   url);
#else
  return url;
#endif
}

bool WolvicContentBrowserClient::
    ShouldCompareEffectiveURLsForSiteInstanceSelection(
        content::BrowserContext* browser_context,
        content::SiteInstance* candidate_site_instance,
        bool is_outermost_main_frame,
        const GURL& candidate_url,
        const GURL& destination_url) {
  DCHECK(browser_context);
  DCHECK(candidate_site_instance);
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return ChromeContentBrowserClientExtensionsPart::
      ShouldCompareEffectiveURLsForSiteInstanceSelection(
          browser_context, candidate_site_instance, is_outermost_main_frame,
          candidate_url, destination_url);
#else
  return true;
#endif
}


bool WolvicContentBrowserClient::ShouldUseProcessPerSite(
    content::BrowserContext* browser_context,
    const GURL& site_url) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  if (ChromeContentBrowserClientExtensionsPart::
        ShouldUseProcessPerSite(browser_context, site_url))
    return true;
#endif
  return false;
}


bool WolvicContentBrowserClient::ShouldUseSpareRenderProcessHost(
    content::BrowserContext* browser_context,
    const GURL& site_url) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return ChromeContentBrowserClientExtensionsPart::
      ShouldUseSpareRenderProcessHost(browser_context, site_url);
#else
  return true;
#endif
}

bool WolvicContentBrowserClient::DoesSiteRequireDedicatedProcess(
    content::BrowserContext* browser_context,
    const GURL& effective_site_url) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  if (ChromeContentBrowserClientExtensionsPart::
        DoesSiteRequireDedicatedProcess(
          browser_context, effective_site_url)) {
    return true;
  }
#endif
  return false;
}


bool WolvicContentBrowserClient::ShouldTreatURLSchemeAsFirstPartyWhenTopLevel(
    base::StringPiece scheme,
    bool is_embedded_origin_secure) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return scheme == extensions::kExtensionScheme;
#else
  return false;
#endif
}

// TODO(crbug.com/1087559): This is based on SubframeTask::GetTitle()
// implementation. Find a general solution to avoid code duplication.
std::string WolvicContentBrowserClient::GetSiteDisplayNameForCdmProcess(
    content::BrowserContext* browser_context,
    const GURL& site_url) {
  // By default, use the |site_url| spec as the display name.
  std::string name = site_url.spec();

#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // If |site_url| wraps a chrome extension ID, we can display the extension
  // name instead, which is more human-readable.
  if (site_url.SchemeIs(extensions::kExtensionScheme)) {
    const extensions::Extension* extension =
        extensions::ExtensionRegistry::Get(browser_context)
            ->enabled_extensions()
            .GetExtensionOrAppByURL(site_url);
    if (extension)
      name = extension->name();
  }
#endif  // BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)

  return name;
}


void WolvicContentBrowserClient::OverrideURLLoaderFactoryParams(
    content::BrowserContext* browser_context,
    const url::Origin& origin,
    bool is_for_isolated_world,
    network::mojom::URLLoaderFactoryParams* factory_params) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  ChromeContentBrowserClientExtensionsPart::
      OverrideURLLoaderFactoryParams(
          browser_context, origin, is_for_isolated_world, factory_params);
#endif
}

void WolvicContentBrowserClient::GetAdditionalViewSourceSchemes(
    std::vector<std::string>* additional_schemes) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  additional_schemes->push_back(extensions::kExtensionScheme);
#endif
}


network::mojom::IPAddressSpace
WolvicContentBrowserClient::DetermineAddressSpaceFromURL(const GURL& url) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  if (url.SchemeIs(extensions::kExtensionScheme))
    return network::mojom::IPAddressSpace::kLocal;
#endif

  return network::mojom::IPAddressSpace::kUnknown;
}

bool WolvicContentBrowserClient::CanCommitURL(
    content::RenderProcessHost* process_host,
    const GURL& url) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return ChromeContentBrowserClientExtensionsPart::CanCommitURL(
      process_host,url);
#else
  return true;
#endif
}

bool WolvicContentBrowserClient::IsSuitableHost(
    content::RenderProcessHost* process_host,
    const GURL& site_url) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return ChromeContentBrowserClientExtensionsPart::IsSuitableHost(
    process_host->GetBrowserContext(), process_host, site_url);
#else
  return true;
#endif
}

bool WolvicContentBrowserClient::ShouldEmbeddedFramesTryToReuseExistingProcess(
    content::RenderFrameHost* outermost_main_frame) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return ChromeContentBrowserClientExtensionsPart::
      ShouldEmbeddedFramesTryToReuseExistingProcess(outermost_main_frame);
#else
  return true;
#endif
}


bool WolvicContentBrowserClient::ShouldSwapBrowsingInstancesForNavigation(
    SiteInstance* site_instance,
    const GURL& current_effective_url,
    const GURL& destination_effective_url) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return ChromeContentBrowserClientExtensionsPart::
      ShouldSwapBrowsingInstancesForNavigation(
          site_instance, current_effective_url, destination_effective_url);
#else
  return false;
#endif
}


std::vector<url::Origin>
WolvicContentBrowserClient::GetOriginsRequiringDedicatedProcess() {
  std::vector<url::Origin> isolated_origin_list;
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  auto origins_from_extensions = ChromeContentBrowserClientExtensionsPart::
      GetOriginsRequiringDedicatedProcess();
  std::move(std::begin(origins_from_extensions),
            std::end(origins_from_extensions),
            std::back_inserter(isolated_origin_list));
#endif

  // Include additional origins preloaded with specific browser configurations,
  // if any.  For example, this is used on Google Chrome for Android to preload
  // a list of important sites to isolate.
  auto built_in_origins =
      site_isolation::GetBrowserSpecificBuiltInIsolatedOrigins();
  std::move(std::begin(built_in_origins), std::end(built_in_origins),
            std::back_inserter(isolated_origin_list));

  return isolated_origin_list;
}


std::vector<std::unique_ptr<content::NavigationThrottle>>
WolvicContentBrowserClient::CreateThrottlesForNavigation(
    content::NavigationHandle* handle) {
  std::vector<std::unique_ptr<content::NavigationThrottle>> throttles;
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  throttles.push_back(
    std::make_unique<extensions::ExtensionNavigationThrottle>(handle));

  // TODO(mshin): Enable the below code after migrating UserScriptListener
  // MaybeAddThrottle(extensions::ExtensionsBrowserClient::Get()
  //                   ->GetUserScriptListener()
  //                   ->CreateNavigationThrottle(handle),
  //               &throttles);
#endif
  return throttles;
}

mojo::PendingRemote<network::mojom::URLLoaderFactory>
WolvicContentBrowserClient::CreateNonNetworkNavigationURLLoaderFactory(
    const std::string& scheme,
    int frame_tree_node_id) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  content::WebContents* web_contents =
      content::WebContents::FromFrameTreeNodeId(frame_tree_node_id);
  content::BrowserContext* browser_context = web_contents->GetBrowserContext();

  if (scheme == extensions::kExtensionScheme) {
    return extensions::CreateExtensionNavigationURLLoaderFactory(
        browser_context,
        !!extensions::WebViewGuest::FromWebContents(web_contents));
  }
#endif  // BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return {};
}

void WolvicContentBrowserClient::
RegisterNonNetworkServiceWorkerUpdateURLLoaderFactories(
    content::BrowserContext* browser_context,
    NonNetworkURLLoaderFactoryMap* factories) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  factories->emplace(
      extensions::kExtensionScheme,
      extensions::CreateExtensionServiceWorkerScriptURLLoaderFactory(
          browser_context));
#endif  // BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
}

void WolvicContentBrowserClient::
RegisterNonNetworkSubresourceURLLoaderFactories(
    int render_process_id,
    int render_frame_id,
    const std::optional<url::Origin>& request_initiator_origin,
    NonNetworkURLLoaderFactoryMap* factories) {
#if BUILDFLAG(ENABLE_EXTENSIONS)
  content::RenderFrameHost* frame_host =
      RenderFrameHost::FromID(render_process_id, render_frame_id);
  auto* web_contents = content::WebContents::FromRenderFrameHost(frame_host);
  content::BrowserContext* browser_context =
      content::RenderProcessHost::FromID(render_process_id)
          ->GetBrowserContext();

  factories->emplace(extensions::kExtensionScheme,
                extensions::CreateExtensionURLLoaderFactory(
                    render_process_id, render_frame_id));

  const extensions::Extension* extension = nullptr;
  if (request_initiator_origin != std::nullopt) {
  extensions::ExtensionRegistry* registry =
    extensions::ExtensionRegistry::Get(browser_context);
  DCHECK(registry);
  extension = registry->enabled_extensions().GetExtensionOrAppByURL(
    request_initiator_origin->GetURL());
  }

  // For service worker contexts, we only allow file access. The remainder of
  // this code is used to allow extensions to access chrome:-scheme
  // resources, which we are moving away from.
  // TODO(crbug.com/1280411) Factories should not be created for unloaded
  // extensions.
  if (extension) {
    InitializeFileURLLoaderFactoryForExtension(
      render_process_id, browser_context, extension, factories);
  }

  // This logic should match
  // ChromeExtensionWebContentsObserver::RenderFrameCreated.
  if (web_contents) {
    AddChromeSchemeFactories(render_process_id, frame_host, web_contents,
                          extension, factories);
  }
#endif  //  BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
}

void WolvicContentBrowserClient::WillCreateURLLoaderFactory(
    content::BrowserContext* browser_context,
    content::RenderFrameHost* frame,
    int render_process_id,
    URLLoaderFactoryType type,
    const url::Origin& request_initiator,
    std::optional<int64_t> navigation_id,
    ukm::SourceIdObj ukm_source_id,
    network::URLLoaderFactoryBuilder& factory_builder,
    mojo::PendingRemote<network::mojom::TrustedURLLoaderHeaderClient>*
        header_client,
    bool* bypass_redirect_checks,
    bool* disable_secure_dns,
    network::mojom::URLLoaderFactoryOverridePtr* factory_override,
    scoped_refptr<base::SequencedTaskRunner> navigation_response_task_runner) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  auto* web_request_api =
      extensions::BrowserContextKeyedAPIFactory<extensions::WebRequestAPI>::Get(
          browser_context);

  // NOTE: Some unit test environments do not initialize
  // BrowserContextKeyedAPI factories for e.g. WebRequest.
  if (web_request_api) {
    bool use_proxy_for_web_request =
        web_request_api->MaybeProxyURLLoaderFactory(
            browser_context, frame, render_process_id, type,
            std::move(navigation_id), ukm_source_id, factory_builder,
            header_client, navigation_response_task_runner, request_initiator);
    if (bypass_redirect_checks)
      *bypass_redirect_checks = use_proxy_for_web_request;
  }
#endif
}

bool WolvicContentBrowserClient::WillInterceptWebSocket(
    content::RenderFrameHost* frame) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  if (!frame) {
    return false;
  }
  const auto* web_request_api =
      extensions::BrowserContextKeyedAPIFactory<extensions::WebRequestAPI>::Get(
          frame->GetBrowserContext());

  // NOTE: Some unit test environments do not initialize
  // BrowserContextKeyedAPI factories for e.g. WebRequest.
  if (!web_request_api)
    return false;

  return (web_request_api->MayHaveProxies() ||
          web_request_api->MayHaveWebsocketProxiesForExtensionTelemetry() ||
          web_request_api->IsAvailableToWebViewEmbedderFrame(frame));
#else
  return false;
#endif
}

void WolvicContentBrowserClient::CreateWebSocket(
    content::RenderFrameHost* frame,
    WebSocketFactory factory,
    const GURL& url,
    const net::SiteForCookies& site_for_cookies,
    const std::optional<std::string>& user_agent,
    mojo::PendingRemote<network::mojom::WebSocketHandshakeClient>
        handshake_client) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // TODO(crbug.com/1243518): Request w/o a frame also should be proxied.
  if (!frame) {
    return;
  }
  auto* web_request_api =
      extensions::BrowserContextKeyedAPIFactory<extensions::WebRequestAPI>::Get(
          frame->GetBrowserContext());

  DCHECK(web_request_api);
  web_request_api->ProxyWebSocket(frame, std::move(factory), url,
                                  site_for_cookies, user_agent,
                                  std::move(handshake_client));
#endif
}

bool WolvicContentBrowserClient::WillCreateRestrictedCookieManager(
    network::mojom::RestrictedCookieManagerRole role,
    content::BrowserContext* browser_context,
    const url::Origin& origin,
    const net::IsolationInfo& isolation_info,
    bool is_service_worker,
    int process_id,
    int routing_id,
    mojo::PendingReceiver<network::mojom::RestrictedCookieManager>* receiver) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // TODO(mshin): Enable the below code after migrating ChromeExtensionCookies
  // if (origin.scheme() == extensions::kExtensionScheme) {
  //   DCHECK_EQ(network::mojom::RestrictedCookieManagerRole::SCRIPT, role);
  //   extensions::ChromeExtensionCookies::Get(browser_context)
  //       ->CreateRestrictedCookieManager(origin, isolation_info,
  //                                       std::move(*receiver));
  //   return true;
  // }
#endif
  return false;
}


bool WolvicContentBrowserClient::ShouldForceDownloadResource(
    content::BrowserContext* browser_context,
    const GURL& url,
    const std::string& mime_type) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // Special-case user scripts to get downloaded instead of viewed.
  if (extensions::UserScript::IsURLUserScript(url, mime_type)) {
    return true;
  }
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)
  return false;
}

bool WolvicContentBrowserClient::IsSecurityLevelAcceptableForWebAuthn(
    content::RenderFrameHost* rfh,
    const url::Origin& caller_origin) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  if (caller_origin.scheme() == extensions::kExtensionScheme) {
    return true;
  }
#endif
  if (net::IsLocalhost(caller_origin.GetURL())) {
    return true;
  }
  // TODO : Apply Security level
  return true;
}

bool WolvicContentBrowserClient::HandleExternalProtocol(
    const GURL& url,
    content::WebContents::Getter web_contents_getter,
    int frame_tree_node_id,
    content::NavigationUIData* navigation_data,
    bool is_primary_main_frame,
    bool is_in_fenced_frame_tree,
    network::mojom::WebSandboxFlags sandbox_flags,
    ui::PageTransition page_transition,
    bool has_user_gesture,
    const std::optional<url::Origin>& initiating_origin,
    content::RenderFrameHost* initiator_document,
    mojo::PendingRemote<network::mojom::URLLoaderFactory>* out_factory) {
// TODO(mshin) : Support ExtensionNavigationUIData
#if 0//BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // External protocols are disabled for guests. An exception is made for the
  // "mailto" protocol, so that pages that utilize it work properly in a
  // WebView.
  ChromeNavigationUIData* chrome_data =
      static_cast<ChromeNavigationUIData*>(navigation_data);
  if ((chrome_data &&
       chrome_data->GetExtensionNavigationUIData()->is_web_view()) &&
      !url.SchemeIs(url::kMailToScheme)) {
    return false;
  }
#endif  // BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return true;
}

bool WolvicContentBrowserClient::IsBuiltinComponent(
    content::BrowserContext* browser_context,
    const url::Origin& origin) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return ChromeContentBrowserClientExtensionsPart::IsBuiltinComponent(
      browser_context, origin);
#else
  return false;
#endif
}

bool WolvicContentBrowserClient::IsClipboardPasteAllowed(
    content::RenderFrameHost* render_frame_host) {
  DCHECK(render_frame_host);

  // Paste requires either (1) user activation, ...
  if (content::WebContents::FromRenderFrameHost(render_frame_host)
          ->HasRecentInteraction()) {
    return true;
  }

  // (2) granted web permission, ...
  content::BrowserContext* browser_context =
      render_frame_host->GetBrowserContext();
  content::PermissionController* permission_controller =
      browser_context->GetPermissionController();
  blink::mojom::PermissionStatus status =
      permission_controller->GetPermissionStatusForCurrentDocument(
          blink::PermissionType::CLIPBOARD_READ_WRITE, render_frame_host);
  if (status == blink::mojom::PermissionStatus::GRANTED)
    return true;

#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // (3) origination directly from a Chrome extension, ...
  const GURL& url =
      render_frame_host->GetMainFrame()->GetLastCommittedOrigin().GetURL();
  auto* registry = extensions::ExtensionRegistry::Get(browser_context);
  if (url.SchemeIs(extensions::kExtensionScheme)) {
    return URLHasExtensionPermission(extensions::ProcessMap::Get(browser_context),
                                     registry, url,
                                     render_frame_host->GetProcess()->GetID(),
                                     APIPermissionID::kClipboardRead);
  }

  // or (4) origination from a process that at least might be running a
  // content script from an extension with the clipboardRead permission.
  // Note that we currently don't allow clipboard operations based just on user
  // script injections.
  extensions::ExtensionIdSet extension_ids = extensions::
      ScriptInjectionTracker::GetExtensionsThatRanContentScriptsInProcess(
          *render_frame_host->GetProcess());
  for (const auto& extension_id : extension_ids) {
    const Extension* extension =
        registry->enabled_extensions().GetByID(extension_id);
    if (extension && extension->permissions_data()->HasAPIPermission(
                         APIPermissionID::kClipboardRead)) {
      return true;
    }
  }
#endif  // BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return false;
}

bool WolvicContentBrowserClient::
    ShouldInheritCrossOriginEmbedderPolicyImplicitly(const GURL& url) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return url.SchemeIs(extensions::kExtensionScheme);
#else
  return false;
#endif
}

bool WolvicContentBrowserClient::
    ShouldServiceWorkerInheritPolicyContainerFromCreator(const GURL& url) {
  if (url.SchemeIsLocal()) {
    return true;
  }
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return url.SchemeIs(extensions::kExtensionScheme);
#else
  return false;
#endif
}

bool WolvicContentBrowserClient::ShouldSendOutermostOriginToRenderer(
    const url::Origin& outermost_origin) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // We only want to send the outermost origin if it is an extension scheme.
  // We do not send the outermost origin to every renderer to avoid leaking
  // additional information into the renderer about the embedder. For
  // extensions though this is required for the way content injection API
  // works. We do not want one extension injecting content into the context
  // of another extension.
  return outermost_origin.scheme() == extensions::kExtensionScheme;
#else
  return false;
#endif
}

bool WolvicContentBrowserClient::ShouldUseFirstPartyStorageKey(
    const url::Origin& origin) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return origin.scheme() == extensions::kExtensionScheme;
#else
  return false;
#endif  // BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
}

}  // namespace wolvic
