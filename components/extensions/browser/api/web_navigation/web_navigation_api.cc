// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Implements the Chrome Extensions WebNavigation API.

#include "components/extensions/browser/api/web_navigation/web_navigation_api.h"

#include <memory>

#include "components/extensions/browser/api/web_navigation/web_navigation_api_constants.h"
#include "components/extensions/browser/api/web_navigation/frame_navigation_state.h"
#include "components/extensions/common/api/web_navigation.h"
#include "content/public/browser/navigation_details.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_api_frame_id_map.h"
#include "extensions/browser/view_type_utils.h"
#include "extensions/common/mojom/view_type.mojom.h"
#include "net/base/net_errors.h"

namespace GetFrame = extensions::api::web_navigation::GetFrame;
namespace GetAllFrames = extensions::api::web_navigation::GetAllFrames;

namespace extensions {

namespace web_navigation = api::web_navigation;

// WebNavigationTabObserver ------------------------------------------

WebNavigationTabObserver::WebNavigationTabObserver(
    content::WebContents* web_contents)
    : WebContentsObserver(web_contents),
      content::WebContentsUserData<WebNavigationTabObserver>(*web_contents) {}

WebNavigationTabObserver::~WebNavigationTabObserver() {}

// static
WebNavigationTabObserver* WebNavigationTabObserver::Get(
    content::WebContents* web_contents) {
  return FromWebContents(web_contents);
}

void WebNavigationTabObserver::RenderFrameDeleted(
    content::RenderFrameHost* render_frame_host) {
  auto* navigation_state =
      FrameNavigationState::GetForCurrentDocument(render_frame_host);
  if (navigation_state && navigation_state->CanSendEvents() &&
      !navigation_state->GetDocumentLoadCompleted()) {
    // TODO(mshin): Enable the below code after migrating web_navigation_api_helpers
    // web_navigation_api_helpers::DispatchOnErrorOccurred(
    //     web_contents(), render_frame_host, navigation_state->GetUrl(),
    //     net::ERR_ABORTED);
    navigation_state->SetErrorOccurredInFrame();
  }
}

void WebNavigationTabObserver::RenderFrameHostChanged(
    content::RenderFrameHost* old_host,
    content::RenderFrameHost* new_host) {
  if (old_host)
    RenderFrameHostPendingDeletion(old_host);
}

void WebNavigationTabObserver::DidStartNavigation(
    content::NavigationHandle* navigation_handle) {
  if (navigation_handle->IsSameDocument() ||
      !FrameNavigationState::IsValidUrl(navigation_handle->GetURL())) {
    return;
  }

  // TODO(mshin): Enable the below code after migrating web_navigation_api_helpers
  // pending_on_before_navigate_event_ =
  //     web_navigation_api_helpers::CreateOnBeforeNavigateEvent(
  //         navigation_handle);

  // Only dispatch the onBeforeNavigate event if the associated WebContents
  // is already added to the tab strip. Otherwise the event should be delayed
  // and sent after the addition, to preserve the ordering of events.
  //
  // TODO(nasko|devlin): This check is necessary because chrome::Navigate()
  // begins the navigation before adding the tab to the TabStripModel, and it
  // is used an indication of that. It would be best if instead it was known
  // when the tab was created and immediately sent the created event instead of
  // waiting for the later TabStripModel kInserted change, but this appears to
  // work for now.
  // TODO(mshin): Enable the below code after migrating ExtensionTabUtil
  // if (ExtensionTabUtil::GetTabById(ExtensionTabUtil::GetTabId(web_contents()),
  //                                  web_contents()->GetBrowserContext(), false,
  //                                  nullptr)) {
  //   DispatchCachedOnBeforeNavigate();
  // }
}

void WebNavigationTabObserver::DidFinishNavigation(
    content::NavigationHandle* navigation_handle) {
  // If there has been a DidStartNavigation call before the tab was ready to
  // dispatch events, ensure that it is sent before processing the
  // DidFinishNavigation.
  // Note: This is exercised by WebNavigationApiTest.TargetBlankIncognito.
  DispatchCachedOnBeforeNavigate();

  if (navigation_handle->HasCommitted() && !navigation_handle->IsErrorPage()) {
    HandleCommit(navigation_handle);
    return;
  }

  HandleError(navigation_handle);
}

void WebNavigationTabObserver::DOMContentLoaded(
    content::RenderFrameHost* render_frame_host) {
  auto* navigation_state =
      FrameNavigationState::GetForCurrentDocument(render_frame_host);
  if (!navigation_state || !navigation_state->CanSendEvents())
    return;

  navigation_state->SetParsingFinished();
  // TODO(mshin): Enable the below code after migrating web_navigation_api_helpers
  // web_navigation_api_helpers::DispatchOnDOMContentLoaded(
  //     web_contents(), render_frame_host, navigation_state->GetUrl());

  if (!navigation_state->GetDocumentLoadCompleted())
    return;

  // The load might already have finished by the time we finished parsing. For
  // compatibility reasons, we artifically delay the load completed signal until
  // after parsing was completed.
  // TODO(mshin): Enable the below code after migrating web_navigation_api_helpers
  // web_navigation_api_helpers::DispatchOnCompleted(
  //     web_contents(), render_frame_host, navigation_state->GetUrl());
}

void WebNavigationTabObserver::DidFinishLoad(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url) {
  auto* navigation_state =
      FrameNavigationState::GetForCurrentDocument(render_frame_host);
  // When showing replacement content, we might get load signals for frames
  // that weren't regularly loaded.
  if (!navigation_state)
    return;

  navigation_state->SetDocumentLoadCompleted();
  if (!navigation_state->CanSendEvents())
    return;

  // A new navigation might have started before the old one completed.
  // Ignore the old navigation completion in that case.
  if (navigation_state->GetUrl() != validated_url)
    return;

  // The load might already have finished by the time we finished parsing. For
  // compatibility reasons, we artifically delay the load completed signal until
  // after parsing was completed.
  if (!navigation_state->GetParsingFinished())
    return;
  // TODO(mshin): Enable the below code after migrating web_navigation_api_helpers
  // web_navigation_api_helpers::DispatchOnCompleted(
  //     web_contents(), render_frame_host, navigation_state->GetUrl());
}

void WebNavigationTabObserver::DidFailLoad(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url,
    int error_code) {
  auto* navigation_state =
      FrameNavigationState::GetForCurrentDocument(render_frame_host);
  // When showing replacement content, we might get load signals for frames
  // that weren't regularly loaded.
  if (!navigation_state)
    return;

  // TODO(mshin): Enable the below code after migrating web_navigation_api_helpers
  // if (navigation_state->CanSendEvents()) {
  //   web_navigation_api_helpers::DispatchOnErrorOccurred(
  //       web_contents(), render_frame_host, navigation_state->GetUrl(),
  //       error_code);
  // }
  navigation_state->SetErrorOccurredInFrame();
}

void WebNavigationTabObserver::DidOpenRequestedURL(
    content::WebContents* new_contents,
    content::RenderFrameHost* source_render_frame_host,
    const GURL& url,
    const content::Referrer& referrer,
    WindowOpenDisposition disposition,
    ui::PageTransition transition,
    bool started_from_context_menu,
    bool renderer_initiated) {
  auto* navigation_state =
      FrameNavigationState::GetForCurrentDocument(source_render_frame_host);
  if (!navigation_state || !navigation_state->CanSendEvents())
    return;

  // We only send the onCreatedNavigationTarget if we end up creating a new
  // window.
  if (disposition != WindowOpenDisposition::SINGLETON_TAB &&
      disposition != WindowOpenDisposition::NEW_FOREGROUND_TAB &&
      disposition != WindowOpenDisposition::NEW_BACKGROUND_TAB &&
      disposition != WindowOpenDisposition::NEW_POPUP &&
      disposition != WindowOpenDisposition::NEW_WINDOW &&
      disposition != WindowOpenDisposition::OFF_THE_RECORD)
    return;

  WebNavigationAPI* api = WebNavigationAPI::GetFactoryInstance()->Get(
      web_contents()->GetBrowserContext());
  if (!api)
    return;  // Possible in unit tests.
  
  // TODO(mshin): Implement WebNavigationEventRouter
}

void WebNavigationTabObserver::DispatchCachedOnBeforeNavigate() {
  if (!pending_on_before_navigate_event_)
    return;

  // EventRouter can be null in unit tests.
  EventRouter* event_router =
      EventRouter::Get(web_contents()->GetBrowserContext());
  if (event_router)
    event_router->BroadcastEvent(std::move(pending_on_before_navigate_event_));
}

void WebNavigationTabObserver::HandleCommit(
    content::NavigationHandle* navigation_handle) {
  // bool is_reference_fragment_navigation =
  //     navigation_handle->IsSameDocument() &&
  //     IsReferenceFragmentNavigation(navigation_handle->GetRenderFrameHost(),
  //                                   navigation_handle->GetURL());

  FrameNavigationState::GetOrCreateForCurrentDocument(
      navigation_handle->GetRenderFrameHost())
      ->StartTrackingDocumentLoad(
          navigation_handle->GetURL(), navigation_handle->IsSameDocument(),
          navigation_handle->IsServedFromBackForwardCache(),
          /*is_error_page=*/false);

  // TODO(mshin): Enable the below code after migrating web_navigation_api_helpers
  // events::HistogramValue histogram_value = events::UNKNOWN;
  // std::string event_name;
  // if (is_reference_fragment_navigation) {
  //   histogram_value = events::WEB_NAVIGATION_ON_REFERENCE_FRAGMENT_UPDATED;
  //   event_name = web_navigation::OnReferenceFragmentUpdated::kEventName;
  // } else if (navigation_handle->IsSameDocument()) {
  //   histogram_value = events::WEB_NAVIGATION_ON_HISTORY_STATE_UPDATED;
  //   event_name = web_navigation::OnHistoryStateUpdated::kEventName;
  // } else {
  //   histogram_value = events::WEB_NAVIGATION_ON_COMMITTED;
  //   event_name = web_navigation::OnCommitted::kEventName;
  // }
  // web_navigation_api_helpers::DispatchOnCommitted(histogram_value, event_name,
  //                                                 navigation_handle);

  // if (navigation_handle->IsServedFromBackForwardCache()) {
  //   web_navigation_api_helpers::DispatchOnCompleted(
  //       navigation_handle->GetWebContents(),
  //       navigation_handle->GetRenderFrameHost(), navigation_handle->GetURL());
  // }
}

void WebNavigationTabObserver::HandleError(
    content::NavigationHandle* navigation_handle) {
  if (navigation_handle->HasCommitted()) {
    FrameNavigationState::GetOrCreateForCurrentDocument(
        navigation_handle->GetRenderFrameHost())
        ->StartTrackingDocumentLoad(navigation_handle->GetURL(),
                                    navigation_handle->IsSameDocument(),
                                    /*is_from_back_forward_cache=*/false,
                                    /*is_error_page=*/true);
  }

  // TODO(mshin): Enable the below code after migrating web_navigation_api_helpers
  // web_navigation_api_helpers::DispatchOnErrorOccurred(navigation_handle);
}

bool WebNavigationTabObserver::IsReferenceFragmentNavigation(
    content::RenderFrameHost* render_frame_host,
    const GURL& url) {
  auto* navigation_state =
      FrameNavigationState::GetForCurrentDocument(render_frame_host);

  GURL existing_url = navigation_state ? navigation_state->GetUrl() : GURL();
  if (existing_url == url)
    return false;

  return existing_url.EqualsIgnoringRef(url);
}

void WebNavigationTabObserver::RenderFrameHostPendingDeletion(
    content::RenderFrameHost* pending_delete_render_frame_host) {
  // The |pending_delete_render_frame_host| and its children are now pending
  // deletion. Stop tracking them.

  pending_delete_render_frame_host->ForEachRenderFrameHost(
      [this](content::RenderFrameHost* render_frame_host) {
        auto* navigation_state =
            FrameNavigationState::GetForCurrentDocument(render_frame_host);
        if (navigation_state) {
          RenderFrameDeleted(render_frame_host);
          FrameNavigationState::DeleteForCurrentDocument(render_frame_host);
        }
      });
}

ExtensionFunction::ResponseAction WebNavigationGetFrameFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction WebNavigationGetAllFramesFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

WebNavigationAPI::WebNavigationAPI(content::BrowserContext* context)
    : browser_context_(context) {
  EventRouter* event_router = EventRouter::Get(browser_context_);
  event_router->RegisterObserver(this,
                                 web_navigation::OnBeforeNavigate::kEventName);
  event_router->RegisterObserver(this, web_navigation::OnCommitted::kEventName);
  event_router->RegisterObserver(this, web_navigation::OnCompleted::kEventName);
  event_router->RegisterObserver(
      this, web_navigation::OnCreatedNavigationTarget::kEventName);
  event_router->RegisterObserver(
      this, web_navigation::OnDOMContentLoaded::kEventName);
  event_router->RegisterObserver(
      this, web_navigation::OnHistoryStateUpdated::kEventName);
  event_router->RegisterObserver(this,
                                 web_navigation::OnErrorOccurred::kEventName);
  event_router->RegisterObserver(
      this, web_navigation::OnReferenceFragmentUpdated::kEventName);
  event_router->RegisterObserver(this,
                                 web_navigation::OnTabReplaced::kEventName);
}

WebNavigationAPI::~WebNavigationAPI() {}

void WebNavigationAPI::Shutdown() {
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

static base::LazyInstance<BrowserContextKeyedAPIFactory<WebNavigationAPI>>::
    DestructorAtExit g_web_navigation_api_factory = LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<WebNavigationAPI>*
WebNavigationAPI::GetFactoryInstance() {
  return g_web_navigation_api_factory.Pointer();
}

void WebNavigationAPI::OnListenerAdded(const EventListenerInfo& details) {
  // TODO(mshin): Implement WebNavigationEventRouter
  // web_navigation_event_router_ = std::make_unique<WebNavigationEventRouter>(
  //     browser_context_);
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(WebNavigationTabObserver);

}  // namespace extensions
