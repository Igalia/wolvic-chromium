// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WOLVIC_WOLVIC_CONTENT_BROWSER_CLIENT_H_
#define WOLVIC_WOLVIC_CONTENT_BROWSER_CLIENT_H_

#include "base/memory/raw_ptr.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/web_contents.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "storage/browser/file_system/file_system_context.h"
#include "wolvic/browser/dialogs/user_dialog_manager_bridge.h"
#include "wolvic/browser/vr/wolvic_xr_integration_client.h"

#include "components/extensions/common/buildflags.h"
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
#include "components/extensions/browser/chrome_content_browser_client_extensions_part.h"
#endif

namespace content {
class BrowserContext;
class WebContents;
}
namespace wolvic {

class WolvicMainParts;

class WolvicContentBrowserClient : public content::ContentBrowserClient {
 public:
  explicit WolvicContentBrowserClient();

  WolvicContentBrowserClient(const WolvicContentBrowserClient&) = delete;
  WolvicContentBrowserClient& operator=(const WolvicContentBrowserClient&) =
      delete;

  ~WolvicContentBrowserClient() override;

  // Returns the single instance.
  static WolvicContentBrowserClient* Get();

  content::BrowserContext* browser_context();
  content::BrowserContext* off_the_record_browser_context();

  // ContentBrowserClient overrides.
  std::string GetUserAgent() override;
  blink::UserAgentMetadata GetUserAgentMetadata() override;
  void ConfigureNetworkContextParams(
      content::BrowserContext* context,
      bool in_memory,
      const base::FilePath& relative_partition_path,
      network::mojom::NetworkContextParams* network_context_params,
      cert_verifier::mojom::CertVerifierCreationParams*
          cert_verifier_creation_params) override;
  std::unique_ptr<content::BrowserMainParts> CreateBrowserMainParts(
      bool is_integration_test) override;
  std::unique_ptr<content::DevToolsManagerDelegate>
  CreateDevToolsManagerDelegate() override;
  std::unique_ptr<content::LoginDelegate> CreateLoginDelegate(
      const net::AuthChallengeInfo& auth_info,
      content::WebContents* web_contents,
      content::BrowserContext* browser_context,
      const content::GlobalRequestID& request_id,
      bool is_request_for_primary_main_frame,
      const GURL& url,
      scoped_refptr<net::HttpResponseHeaders> response_headers,
      bool first_auth_attempt,
      LoginAuthRequiredCallback auth_required_callback) override;
#if BUILDFLAG(ENABLE_VR)
  content::XrIntegrationClient* GetXrIntegrationClient() override;
#endif
  void BindMediaServiceReceiver(content::RenderFrameHost* render_frame_host,
                                mojo::GenericPendingReceiver receiver) override;
  void OpenURL(
      content::SiteInstance* site_instance,
      const content::OpenURLParams& params,
      base::OnceCallback<void(content::WebContents*)> callback) override;
  void RegisterAssociatedInterfaceBindersForRenderFrameHost(
      content::RenderFrameHost& render_frame_host,
      blink::AssociatedInterfaceRegistry& associated_registry) override;
  void RegisterBrowserInterfaceBindersForFrame(
      content::RenderFrameHost* render_frame_host,
      mojo::BinderMapWithContext<content::RenderFrameHost*>* map) override;
  void SiteInstanceGotProcessAndSite(
    content::SiteInstance* site_instance) override;
  void AppendExtraCommandLineSwitches(base::CommandLine* command_line,
                                      int child_process_id) override;
  content::AllowServiceWorkerResult AllowServiceWorker(
      const GURL& scope,
      const net::SiteForCookies& site_for_cookies,
      const std::optional<url::Origin>& top_frame_origin,
      const GURL& script_url,
      content::BrowserContext* context) override;
  bool MayDeleteServiceWorkerRegistration(
      const GURL& scope,
      content::BrowserContext* browser_context) override;
  bool ShouldTryToUpdateServiceWorkerRegistration(
      const GURL& scope,
      content::BrowserContext* browser_context) override;
  bool DoesSchemeAllowCrossOriginSharedWorker(
      const std::string& scheme) override;
  bool CanCreateWindow(content::RenderFrameHost* opener,
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
                       bool* no_javascript_access) override;
  void OverrideWebkitPrefs(content::WebContents* web_contents,
                           blink::web_pref::WebPreferences* prefs) override;
  bool OverrideWebPreferencesAfterNavigation(
      content::WebContents* web_contents,
      blink::web_pref::WebPreferences* prefs) override;
  void BrowserURLHandlerCreated(content::BrowserURLHandler* handler) override;
  void GetAdditionalAllowedSchemesForFileSystem(
    std::vector<std::string>* additional_schemes) override;

  content::StoragePartitionConfig GetStoragePartitionConfigForSite(
      content::BrowserContext* browser_context,
      const GURL& site) override;
  GURL GetEffectiveURL(content::BrowserContext* browser_context,
                       const GURL& url) override;
  bool ShouldCompareEffectiveURLsForSiteInstanceSelection(
      content::BrowserContext* browser_context,
      content::SiteInstance* candidate_site_instance,
      bool is_outermost_main_frame,
      const GURL& candidate_url,
      const GURL& destination_url) override;
  bool ShouldUseProcessPerSite(content::BrowserContext* browser_context,
                               const GURL& site_url) override;
  bool ShouldUseSpareRenderProcessHost(content::BrowserContext* browser_context,
                                       const GURL& site_url) override;
  bool DoesSiteRequireDedicatedProcess(content::BrowserContext* browser_context,
                                       const GURL& effective_site_url) override;
  bool ShouldTreatURLSchemeAsFirstPartyWhenTopLevel(
      base::StringPiece scheme,
      bool is_embedded_origin_secure) override;
  std::string GetSiteDisplayNameForCdmProcess(
      content::BrowserContext* browser_context,
      const GURL& site_url) override;
  void OverrideURLLoaderFactoryParams(
      content::BrowserContext* browser_context,
      const url::Origin& origin,
      bool is_for_isolated_world,
      network::mojom::URLLoaderFactoryParams* factory_params) override;
  void GetAdditionalViewSourceSchemes(
      std::vector<std::string>* additional_schemes) override;
  network::mojom::IPAddressSpace DetermineAddressSpaceFromURL(
      const GURL& url) override;
  bool CanCommitURL(content::RenderProcessHost* process_host,
                    const GURL& url) override;
  bool IsSuitableHost(content::RenderProcessHost* process_host,
                      const GURL& site_url) override;
  bool ShouldEmbeddedFramesTryToReuseExistingProcess(
      content::RenderFrameHost* outermost_main_frame) override;
  bool ShouldSwapBrowsingInstancesForNavigation(
      content::SiteInstance* site_instance,
      const GURL& current_effective_url,
      const GURL& destination_effective_url) override;
  std::vector<url::Origin> GetOriginsRequiringDedicatedProcess() override;
  std::vector<std::unique_ptr<content::NavigationThrottle>>
  CreateThrottlesForNavigation(content::NavigationHandle* handle) override;
  mojo::PendingRemote<network::mojom::URLLoaderFactory>
  CreateNonNetworkNavigationURLLoaderFactory(const std::string& scheme,
                                             int frame_tree_node_id) override;
  void RegisterNonNetworkServiceWorkerUpdateURLLoaderFactories(
      content::BrowserContext* browser_context,
      NonNetworkURLLoaderFactoryMap* factories) override;
  void RegisterNonNetworkSubresourceURLLoaderFactories(
      int render_process_id,
      int render_frame_id,
      const std::optional<url::Origin>& request_initiator_origin,
      NonNetworkURLLoaderFactoryMap* factories) override;
  void WillCreateURLLoaderFactory(
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
      scoped_refptr<base::SequencedTaskRunner> navigation_response_task_runner)
      override;
  bool WillInterceptWebSocket(content::RenderFrameHost* frame) override;
  void CreateWebSocket(
      content::RenderFrameHost* frame,
      WebSocketFactory factory,
      const GURL& url,
      const net::SiteForCookies& site_for_cookies,
      const std::optional<std::string>& user_agent,
      mojo::PendingRemote<network::mojom::WebSocketHandshakeClient>
          handshake_client) override;
  void MaybeInterceptWebTransport(
      int process_id,
      int frame_routing_id,
      const GURL& url,
      const url::Origin& initiator_origin,
      mojo::PendingRemote<network::mojom::WebTransportHandshakeClient>
          handshake_client,
      WillCreateWebTransportCallback callback);
  bool WillCreateRestrictedCookieManager(
      network::mojom::RestrictedCookieManagerRole role,
      content::BrowserContext* browser_context,
      const url::Origin& origin,
      const net::IsolationInfo& isolation_info,
      bool is_service_worker,
      int process_id,
      int routing_id,
      mojo::PendingReceiver<network::mojom::RestrictedCookieManager>* receiver)
      override;
  bool ShouldForceDownloadResource(content::BrowserContext* browser_context,
                                   const GURL& url,
                                   const std::string& mime_type) override;
  bool IsSecurityLevelAcceptableForWebAuthn(
      content::RenderFrameHost* rfh,
      const url::Origin& caller_origin) override;
  bool HandleExternalProtocol(
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
      mojo::PendingRemote<network::mojom::URLLoaderFactory>* out_factory)
      override;
  bool IsBuiltinComponent(content::BrowserContext* browser_context,
                          const url::Origin& origin) override;
  bool IsClipboardPasteAllowed(
      content::RenderFrameHost* render_frame_host) override;
  bool ShouldInheritCrossOriginEmbedderPolicyImplicitly(
      const GURL& url) override;
  bool ShouldServiceWorkerInheritPolicyContainerFromCreator(
      const GURL& url) override;
  bool ShouldSendOutermostOriginToRenderer(
      const url::Origin& outermost_origin) override;
  bool ShouldUseFirstPartyStorageKey(const url::Origin& origin) override;

 private:
  raw_ptr<WolvicMainParts> browser_main_parts_;
#if BUILDFLAG(ENABLE_VR)
  std::unique_ptr<WolvicXrIntegrationClient> xr_integration_client_;
#endif
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  std::unique_ptr<components_extensions::ChromeContentBrowserClientExtensionsPart> extensions_part_;
#endif
};

}  // namespace wolvic

#endif  // WOLVIC_WOLVIC_CONTENT_BROWSER_CLIENT_H_
