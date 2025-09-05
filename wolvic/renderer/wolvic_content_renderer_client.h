// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef WOLVIC_RENDERER_WOLVIC_CONTENT_RENDERER_CLIENT_H_
#define WOLVIC_RENDERER_WOLVIC_CONTENT_RENDERER_CLIENT_H_

#include "content/public/renderer/content_renderer_client.h"

namespace visitedlink {
class VisitedLinkReader;
}

namespace wolvic {

class WolvicContentRendererClient : public content::ContentRendererClient {
 public:
  WolvicContentRendererClient();

  WolvicContentRendererClient(const WolvicContentRendererClient&) = delete;
  WolvicContentRendererClient& operator=(const WolvicContentRendererClient&) =
      delete;

  ~WolvicContentRendererClient() override;

  // ContentRendererClient implementation.
  std::unique_ptr<media::KeySystemSupportObserver> GetSupportedKeySystems(
      media::GetSupportedKeySystemsCB cb) override;
  void RenderThreadStarted() override;
  void ExposeInterfacesToBrowser(mojo::BinderMap* binders) override;
  void RenderFrameCreated(content::RenderFrame* render_frame) override;
  void PrepareErrorPage(content::RenderFrame* render_frame,
                        const blink::WebURLError& error,
                        const std::string& http_method,
                        content::mojom::AlternativeErrorPageOverrideInfoPtr
                            alternative_error_page_info,
                        std::string* error_html) override;
  void PrepareErrorPageForHttpStatusError(
      content::RenderFrame* render_frame,
      const blink::WebURLError& error,
      const std::string& http_method,
      int http_status,
      content::mojom::AlternativeErrorPageOverrideInfoPtr
          alternative_error_page_info,
      std::string* error_html) override;
  void WebViewCreated(blink::WebView* web_view,
                      bool was_created_by_renderer,
                      const url::Origin* outermost_origin) override;
  v8::Local<v8::Object> GetScriptableObject(
      const blink::WebElement& plugin_element,
      v8::Isolate* isolate) override;
  bool AllowPopup() override;
  bool ShouldNotifyServiceWorkerOnWebSocketActivity(
      v8::Local<v8::Context> context) override;
  blink::ProtocolHandlerSecurityLevel GetProtocolHandlerSecurityLevel(
      const url::Origin& origin) override;
  void WillSendRequest(blink::WebLocalFrame* frame,
                       ui::PageTransition transition_type,
                       const blink::WebURL& url,
                       const net::SiteForCookies& site_for_cookies,
                       const url::Origin* initiator_origin,
                       GURL* new_url) override;
  uint64_t VisitedLinkHash(std::string_view canonical_url) override;
  bool IsLinkVisited(uint64_t link_hash) override;
  bool ShouldReportDetailedMessageForSource(
      const std::u16string& source) override;
  void RunScriptsAtDocumentStart(content::RenderFrame* render_frame) override;
  void RunScriptsAtDocumentEnd(content::RenderFrame* render_frame) override;
  void RunScriptsAtDocumentIdle(content::RenderFrame* render_frame) override;
  void SetRuntimeFeaturesDefaultsBeforeBlinkInitialization() override;
  bool AllowScriptExtensionForServiceWorker(
      const url::Origin& script_origin) override;
  void DidInitializeServiceWorkerContextOnWorkerThread(
      blink::WebServiceWorkerContextProxy* context_proxy,
      const GURL& service_worker_scope,
      const GURL& script_url) override;
  void WillEvaluateServiceWorkerOnWorkerThread(
      blink::WebServiceWorkerContextProxy* context_proxy,
      v8::Local<v8::Context> v8_context,
      int64_t service_worker_version_id,
      const GURL& service_worker_scope,
      const GURL& script_url) override;
  void DidStartServiceWorkerContextOnWorkerThread(
      int64_t service_worker_version_id,
      const GURL& service_worker_scope,
      const GURL& script_url) override;
  void WillDestroyServiceWorkerContextOnWorkerThread(
      v8::Local<v8::Context> context,
      int64_t service_worker_version_id,
      const GURL& service_worker_scope,
      const GURL& script_url) override;
  blink::WebFrame* FindFrame(blink::WebLocalFrame* relative_to_frame,
                             const std::string& name) override;
  bool IsSafeRedirectTarget(const GURL& from_url, const GURL& to_url) override;
  void AppendContentSecurityPolicy(
      const blink::WebURL& url,
      blink::WebVector<blink::WebContentSecurityPolicyHeader>* csp) override;

  visitedlink::VisitedLinkReader* visited_link_reader() {
    return visited_link_reader_.get();
  }

private:
  std::unique_ptr<visitedlink::VisitedLinkReader> visited_link_reader_;
};

}  // namespace wolvic

#endif  // WOLVIC_RENDERER_WOLVIC_CONTENT_RENDERER_CLIENT_H_