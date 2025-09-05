// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/renderer/wolvic_content_renderer_client.h"

#include "components/cdm/renderer/key_system_support_update.h"
#include "components/autofill/content/renderer/autofill_agent.h"
#include "components/autofill/content/renderer/password_autofill_agent.h"
#include "components/autofill/content/renderer/password_generation_agent.h"
#include "components/visitedlink/renderer/visitedlink_reader.h"
#include "content/public/renderer/render_frame.h"
#include "mojo/public/cpp/bindings/binder_map.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/common/features.h"
#include "wolvic/renderer/browser_exposed_renderer_interfaces.h"
#include "wolvic/renderer/wolvic_render_frame_observer.h"

#include "components/extensions/common/buildflags.h"
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
#include "base/command_line.h"
#include "base/process/current_process.h"
#include "components/extensions/common/chrome_resource_request_blocked_reason.h"
#include "components/extensions/common/initialize_extensions_client.h"
#include "components/extensions/renderer/api/chrome_extensions_renderer_api_provider.h"
#include "components/extensions/renderer/chrome_extensions_renderer_client.h"
#include "content/public/renderer/render_thread.h"
#include "extensions/common/constants.h"
#include "extensions/common/context_data.h"
#include "extensions/common/extension_urls.h"
#include "extensions/common/manifest_handlers/csp_info.h"
#include "extensions/common/manifest_handlers/web_accessible_resources_info.h"
#include "extensions/common/switches.h"
#include "extensions/renderer/api/core_extensions_renderer_api_provider.h"
#include "extensions/renderer/dispatcher.h"
#include "extensions/renderer/guest_view/mime_handler_view/mime_handler_view_container_manager.h"
#include "extensions/renderer/renderer_extension_registry.h"
#include "extensions/renderer/worker_script_context_set.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "third_party/blink/public/common/security/protocol_handler_security_level.h"
#include "third_party/blink/public/mojom/css/preferred_color_scheme.mojom.h"
#include "third_party/blink/public/platform/scheduler/web_renderer_process_type.h"
#include "third_party/blink/public/platform/web_content_security_policy_struct.h"
#include "third_party/blink/public/platform/web_runtime_features.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/platform/web_vector.h"
#include "third_party/blink/public/web/web_security_policy.h"
#include "third_party/blink/public/web/web_settings.h"
#endif

using components_extensions::ChromeExtensionsRendererAPIProvider;
using components_extensions::ChromeExtensionsRendererClient;
using blink::WebLocalFrame;
using blink::WebSecurityPolicy;
using blink::WebString;

namespace wolvic {

namespace {

bool IsStandaloneContentExtensionProcess() {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return base::CommandLine::ForCurrentProcess()->HasSwitch(
    extensions::switches::kExtensionProcess);
#else
  return false;
#endif
}

bool IsExtensionExtendedErrorCode(int extended_error_code) {
  return extended_error_code ==
         static_cast<int>(ChromeResourceRequestBlockedReason::kExtension);
}

}

WolvicContentRendererClient::WolvicContentRendererClient() {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  components_extensions::EnsureExtensionsClientInitialized();
  extensions::ExtensionsRendererClient::Set(
      ChromeExtensionsRendererClient::GetInstance());
#endif
}

WolvicContentRendererClient::~WolvicContentRendererClient() = default;

std::unique_ptr<media::KeySystemSupportObserver>
WolvicContentRendererClient::GetSupportedKeySystems(
    media::GetSupportedKeySystemsCB cb) {
  return cdm::GetSupportedKeySystemsUpdates(/*can_persist_data=*/true,
                                            std::move(cb));
}

void WolvicContentRendererClient::RenderThreadStarted() {
  visited_link_reader_ = std::make_unique<visitedlink::VisitedLinkReader>();
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  content::RenderThread* thread = content::RenderThread::Get();
  const bool is_extension = IsStandaloneContentExtensionProcess();

  thread->SetRendererProcessType(
      is_extension
          ? blink::scheduler::WebRendererProcessType::kExtensionRenderer
          : blink::scheduler::WebRendererProcessType::kRenderer);

  if (is_extension) {
    // The process name was set to "Renderer" in RendererMain(). Update it to
    // "Extension Renderer" to highlight that it's hosting an extension.
    base::CurrentProcess::GetInstance().SetProcessType(
        base::CurrentProcessType::PROCESS_RENDERER_EXTENSION);
  }

  ChromeExtensionsRendererClient* chrome_extensions_renderer_client =
      ChromeExtensionsRendererClient::GetInstance();
  chrome_extensions_renderer_client->AddAPIProvider(
      std::make_unique<extensions::CoreExtensionsRendererAPIProvider>());
  chrome_extensions_renderer_client->AddAPIProvider(
      std::make_unique<ChromeExtensionsRendererAPIProvider>());
  chrome_extensions_renderer_client->RenderThreadStarted();
  WebSecurityPolicy::RegisterURLSchemeAsExtension(
      WebString::FromASCII(extensions::kExtensionScheme));
  WebSecurityPolicy::RegisterURLSchemeAsCodeCacheWithHashing(
      WebString::FromASCII(extensions::kExtensionScheme));
  WebSecurityPolicy::AddSchemeToSecureContextSafelist(
      WebString::FromASCII(extensions::kExtensionScheme));
#endif

}

void WolvicContentRendererClient::ExposeInterfacesToBrowser(mojo::BinderMap* binders) {
  // NOTE: Do not add binders directly within this method. Instead, modify the
  // definition of |ExposeRendererInterfacesToBrowser()| to ensure security
  // review coverage.
  ExposeRendererInterfacesToBrowser(this, binders);
}

void WolvicContentRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
  auto* render_frame_observer = new WolvicRenderFrameObserver(render_frame);

#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // TODO(mshin): Enable the below code after implementing the content setting
  // auto content_settings_delegate =
  //     std::make_unique<ChromeContentSettingsAgentDelegate>(render_frame);
  // content_settings_delegate->SetExtensionDispatcher(
  //     ChromeExtensionsRendererClient::GetInstance()->extension_dispatcher());

  service_manager::BinderRegistry* registry = render_frame_observer->registry();
  ChromeExtensionsRendererClient::GetInstance()->RenderFrameCreated(
      render_frame, registry);
#endif

  blink::AssociatedInterfaceRegistry* associated_interfaces =
      render_frame_observer->associated_interfaces();

  if (!render_frame->IsInFencedFrameTree() ||
      base::FeatureList::IsEnabled(blink::features::kFencedFramesAPIChanges)) {
    auto password_autofill_agent =
        std::make_unique<autofill::PasswordAutofillAgent>(
            render_frame, associated_interfaces,
            autofill::PasswordAutofillAgent::EnableHeavyFormDataScraping(
                false));
    new autofill::AutofillAgent(
        render_frame,
        {
            autofill::AutofillAgent::ExtractAllDatalists(false),
            autofill::AutofillAgent::FocusRequiresScroll(false),
            autofill::AutofillAgent::QueryPasswordSuggestions(false),
            autofill::AutofillAgent::SecureContextRequired(false),
            autofill::AutofillAgent::UserGestureRequired(false),
            autofill::AutofillAgent::UsesKeyboardAccessoryForSuggestions(false),
        },
        std::move(password_autofill_agent),
        std::unique_ptr<autofill::PasswordGenerationAgent>(),
        associated_interfaces);
  }


#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  associated_interfaces
      ->AddInterface<extensions::mojom::MimeHandlerViewContainerManager>(
          base::BindRepeating(
              &extensions::MimeHandlerViewContainerManager::BindReceiver,
              base::Unretained(render_frame)));
#endif
}

void WolvicContentRendererClient::PrepareErrorPage(
    content::RenderFrame* render_frame,
    const blink::WebURLError& error,
    const std::string& http_method,
    content::mojom::AlternativeErrorPageOverrideInfoPtr
        alternative_error_page_info,
    std::string* error_html) {
  if (error_html && error_html->empty()) {
    bool is_blocked_by_extension = false;
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
    is_blocked_by_extension = IsExtensionExtendedErrorCode(error.extended_reason());
#endif

    *error_html =
        "<head><title>Error</title></head><body>" +
        (is_blocked_by_extension ?
            std::string("This page has been blocked by an extension.") :
            std::string("Could not load the requested resource.")) +
        "<br/>Error code: " + base::NumberToString(error.reason()) +
        (!is_blocked_by_extension && error.reason() < 0 ?
            " (" + net::ErrorToString(error.reason()) + ")" : "") +
        "</body>";
  }
}

void WolvicContentRendererClient::PrepareErrorPageForHttpStatusError(
    content::RenderFrame* render_frame,
    const blink::WebURLError& error,
    const std::string& http_method,
    int http_status,
    content::mojom::AlternativeErrorPageOverrideInfoPtr
        alternative_error_page_info,
    std::string* error_html) {
  if (error_html) {
    *error_html =
        "<head><title>Error</title></head><body>Server returned HTTP status " +
        base::NumberToString(http_status) + "</body>";
  }
}

void WolvicContentRendererClient::WebViewCreated(
    blink::WebView* web_view,
    bool was_created_by_renderer,
    const url::Origin* outermost_origin) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  ChromeExtensionsRendererClient::GetInstance()->WebViewCreated(
      web_view, outermost_origin);
#endif
}

v8::Local<v8::Object> WolvicContentRendererClient::GetScriptableObject(
    const blink::WebElement& plugin_element,
    v8::Isolate* isolate) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return ChromeExtensionsRendererClient::GetInstance()->GetScriptableObject(
      plugin_element, isolate);
#else
  return v8::Local<v8::Object>();
#endif
}

bool WolvicContentRendererClient::AllowPopup() {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return ChromeExtensionsRendererClient::GetInstance()->AllowPopup();
#else
  return false;
#endif
}
  
bool WolvicContentRendererClient::ShouldNotifyServiceWorkerOnWebSocketActivity(
    v8::Local<v8::Context> context) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  extensions::ScriptContext* script_context =
      ChromeExtensionsRendererClient::GetInstance()
          ->extension_dispatcher()
          ->GetWorkerScriptContextSet()
          ->GetContextByV8Context(context);
  // Only notify on web socket activity if the service worker is the background
  // service worker for an extension.
  return script_context &&
         ChromeExtensionsRendererClient::GetInstance()
             ->ExtensionAPIEnabledForServiceWorkerScript(
                 script_context->service_worker_scope(), script_context->url());
#else
  return false;
#endif
}

blink::ProtocolHandlerSecurityLevel
WolvicContentRendererClient::GetProtocolHandlerSecurityLevel(
    const url::Origin& origin) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return ChromeExtensionsRendererClient::GetInstance()
      ->GetProtocolHandlerSecurityLevel();
#else
  return blink::ProtocolHandlerSecurityLevel::kStrict;
#endif
}

void WolvicContentRendererClient::WillSendRequest(
    WebLocalFrame* frame,
    ui::PageTransition transition_type,
    const blink::WebURL& url,
    const net::SiteForCookies& site_for_cookies,
    const url::Origin* initiator_origin,
    GURL* new_url) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // Check whether the request should be allowed. If not allowed, we reset the
  // URL to something invalid to prevent the request and cause an error.
  ChromeExtensionsRendererClient::GetInstance()->WillSendRequest(
      frame, transition_type, url, site_for_cookies, initiator_origin, new_url);
  if (!new_url->is_empty())
    return;
#endif
}

uint64_t WolvicContentRendererClient::VisitedLinkHash(std::string_view canonical_url) {
  return visited_link_reader_->ComputeURLFingerprint(canonical_url);
}

bool WolvicContentRendererClient::IsLinkVisited(uint64_t link_hash) {
  return visited_link_reader_->IsVisited(link_hash);
}

bool WolvicContentRendererClient::ShouldReportDetailedMessageForSource(
    const std::u16string& source) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return extensions::IsSourceFromAnExtension(source);
#else
  return false;
#endif
}

void WolvicContentRendererClient::RunScriptsAtDocumentStart(
    content::RenderFrame* render_frame) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  ChromeExtensionsRendererClient::GetInstance()->RunScriptsAtDocumentStart(
      render_frame);
  // |render_frame| might be dead by now.
#endif
}

void WolvicContentRendererClient::RunScriptsAtDocumentEnd(content::RenderFrame* render_frame) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  ChromeExtensionsRendererClient::GetInstance()->RunScriptsAtDocumentEnd(
      render_frame);
  // |render_frame| might be dead by now.
#endif
}

void WolvicContentRendererClient::RunScriptsAtDocumentIdle(
    content::RenderFrame* render_frame) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  ChromeExtensionsRendererClient::GetInstance()->RunScriptsAtDocumentIdle(
      render_frame);
  // |render_frame| might be dead by now.
#endif
}


void WolvicContentRendererClient::
    SetRuntimeFeaturesDefaultsBeforeBlinkInitialization() {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // WebHID and WebUSB on service workers is only available in extensions.
  if (IsStandaloneContentExtensionProcess()) {
    blink::WebRuntimeFeatures::EnableWebUSBOnServiceWorkers(true);
  }
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)
}

bool WolvicContentRendererClient::AllowScriptExtensionForServiceWorker(
    const url::Origin& script_origin) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return script_origin.scheme() == extensions::kExtensionScheme;
#else
  return false;
#endif
}

void WolvicContentRendererClient::
    DidInitializeServiceWorkerContextOnWorkerThread(
        blink::WebServiceWorkerContextProxy* context_proxy,
        const GURL& service_worker_scope,
        const GURL& script_url) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  ChromeExtensionsRendererClient::GetInstance()
      ->extension_dispatcher()
      ->DidInitializeServiceWorkerContextOnWorkerThread(
          context_proxy, service_worker_scope, script_url);
#endif
}

void WolvicContentRendererClient::WillEvaluateServiceWorkerOnWorkerThread(
    blink::WebServiceWorkerContextProxy* context_proxy,
    v8::Local<v8::Context> v8_context,
    int64_t service_worker_version_id,
    const GURL& service_worker_scope,
    const GURL& script_url) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  ChromeExtensionsRendererClient::GetInstance()
      ->extension_dispatcher()
      ->WillEvaluateServiceWorkerOnWorkerThread(
          context_proxy, v8_context, service_worker_version_id,
          service_worker_scope, script_url);
#endif
}

void WolvicContentRendererClient::DidStartServiceWorkerContextOnWorkerThread(
    int64_t service_worker_version_id,
    const GURL& service_worker_scope,
    const GURL& script_url) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  ChromeExtensionsRendererClient::GetInstance()
      ->extension_dispatcher()
      ->DidStartServiceWorkerContextOnWorkerThread(
          service_worker_version_id, service_worker_scope, script_url);
#endif
}

void WolvicContentRendererClient::WillDestroyServiceWorkerContextOnWorkerThread(
    v8::Local<v8::Context> context,
    int64_t service_worker_version_id,
    const GURL& service_worker_scope,
    const GURL& script_url) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  ChromeExtensionsRendererClient::GetInstance()
      ->extension_dispatcher()
      ->WillDestroyServiceWorkerContextOnWorkerThread(
          context, service_worker_version_id, service_worker_scope, script_url);
#endif
}

blink::WebFrame* WolvicContentRendererClient::FindFrame(
    blink::WebLocalFrame* relative_to_frame,
    const std::string& name) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return ChromeExtensionsRendererClient::FindFrame(relative_to_frame, name);
#else
  return nullptr;
#endif  // BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
}

bool WolvicContentRendererClient::IsSafeRedirectTarget(const GURL& from_url,
                                                       const GURL& to_url) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  if (to_url.SchemeIs(extensions::kExtensionScheme)) {
    const extensions::Extension* extension =
        extensions::RendererExtensionRegistry::Get()->GetByID(to_url.host());
    if (!extension)
      return false;
    // TODO(solomonkinard): Use initiator_origin and add tests.
    if (extensions::WebAccessibleResourcesInfo::IsResourceWebAccessible(
            extension, to_url.path(), nullptr)) {
      return true;
    }
    return extension->guid() == from_url.host();
  }
#endif  // BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  return true;
}


void WolvicContentRendererClient::AppendContentSecurityPolicy(
    const blink::WebURL& url,
    blink::WebVector<blink::WebContentSecurityPolicyHeader>* csp) {
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  DCHECK(csp);
  GURL gurl(url);
  const extensions::Extension* extension =
      extensions::RendererExtensionRegistry::Get()->GetExtensionOrAppByURL(
          gurl);
  if (!extension)
    return;

  // Append a minimum CSP to ensure the extension can't relax the default
  // applied CSP through means like Service Worker.
  const std::string* default_csp =
      extensions::CSPInfo::GetMinimumCSPToAppend(*extension, gurl.path());
  if (!default_csp)
    return;

  csp->push_back({blink::WebString::FromUTF8(*default_csp),
                  network::mojom::ContentSecurityPolicyType::kEnforce,
                  network::mojom::ContentSecurityPolicySource::kHTTP});
#endif
}

}  // namespace wolvic
