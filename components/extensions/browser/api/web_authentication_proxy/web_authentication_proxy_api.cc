// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/web_authentication_proxy/web_authentication_proxy_api.h"

#include "base/functional/bind.h"
#include "base/logging.h"
#include "components/extensions/common/api/web_authentication_proxy.h"
#include "content/public/browser/browser_context.h"

namespace extensions {

BrowserContextKeyedAPIFactory<WebAuthenticationProxyAPI>*
WebAuthenticationProxyAPI::GetFactoryInstance() {
  static base::NoDestructor<
      BrowserContextKeyedAPIFactory<WebAuthenticationProxyAPI>>
      instance;
  return instance.get();
}

WebAuthenticationProxyAPI::WebAuthenticationProxyAPI(
    content::BrowserContext* context)
    : context_(context) {
  EventRouter::Get(context_)->RegisterObserver(
      this,
      api::web_authentication_proxy::OnRemoteSessionStateChange::kEventName);
}

WebAuthenticationProxyAPI::~WebAuthenticationProxyAPI() = default;

void WebAuthenticationProxyAPI::Shutdown() {
  EventRouter::Get(context_)->UnregisterObserver(this);
}

void WebAuthenticationProxyAPI::OnListenerAdded(
    const EventListenerInfo& details) {
  DCHECK_EQ(
      details.event_name,
      api::web_authentication_proxy::OnRemoteSessionStateChange::kEventName);
  // This may be called multiple times for the same extension, but we only need
  // to instantiate a notifier once.
  // TODO(mshin): Enable the below code after support Remote session
  // session_state_change_notifiers_.try_emplace(
  //     details.extension_id, EventRouter::Get(context_), details.extension_id);
}

void WebAuthenticationProxyAPI::OnListenerRemoved(
    const EventListenerInfo& details) {
  DCHECK_EQ(
      details.event_name,
      api::web_authentication_proxy::OnRemoteSessionStateChange::kEventName);
  if (EventRouter::Get(context_)->ExtensionHasEventListener(
          details.extension_id, api::web_authentication_proxy::
                                    OnRemoteSessionStateChange::kEventName)) {
    // This wasn't necessarily the last remaining listener for this extension.
    return;
  }
  // TODO(mshin): Enable the below code after support Remote session
  // auto it = session_state_change_notifiers_.find(details.extension_id);
  // DCHECK(it != session_state_change_notifiers_.end());
  // session_state_change_notifiers_.erase(it);
}

WebAuthenticationProxyAttachFunction::WebAuthenticationProxyAttachFunction() =
    default;
WebAuthenticationProxyAttachFunction::~WebAuthenticationProxyAttachFunction() =
    default;

ExtensionFunction::ResponseAction WebAuthenticationProxyAttachFunction::Run() {
  DCHECK(extension());

  // TODO(mshin): Support Web Authentication Proxy
  // const bool success =
  //     WebAuthenticationProxyRegistrarFactory::GetForBrowserContext(
  //         browser_context())
  //         ->SetRequestProxy(Profile::FromBrowserContext(browser_context()),
  //                           extension());
  const bool success = false;
  return RespondNow(success ? NoArguments()
                            : Error("Another extension is already attached"));
}

WebAuthenticationProxyDetachFunction::WebAuthenticationProxyDetachFunction() =
    default;
WebAuthenticationProxyDetachFunction::~WebAuthenticationProxyDetachFunction() =
    default;

ExtensionFunction::ResponseAction WebAuthenticationProxyDetachFunction::Run() {
  DCHECK(extension());

  return RespondNow(Error("Not Implement"));
}

WebAuthenticationProxyCompleteCreateRequestFunction::
    WebAuthenticationProxyCompleteCreateRequestFunction() = default;
WebAuthenticationProxyCompleteCreateRequestFunction::
    ~WebAuthenticationProxyCompleteCreateRequestFunction() = default;

void WebAuthenticationProxyCompleteCreateRequestFunction::DoRespond(
    std::optional<std::string> error) {
  Respond(error ? Error(std::move(*error)) : NoArguments());
}

ExtensionFunction::ResponseAction
WebAuthenticationProxyCompleteCreateRequestFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

WebAuthenticationProxyCompleteGetRequestFunction::
    WebAuthenticationProxyCompleteGetRequestFunction() = default;
WebAuthenticationProxyCompleteGetRequestFunction::
    ~WebAuthenticationProxyCompleteGetRequestFunction() = default;

void WebAuthenticationProxyCompleteGetRequestFunction::DoRespond(
    std::optional<std::string> error) {
  Respond(error ? Error(std::move(*error)) : NoArguments());
}

ExtensionFunction::ResponseAction
WebAuthenticationProxyCompleteGetRequestFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

WebAuthenticationProxyCompleteIsUvpaaRequestFunction::
    WebAuthenticationProxyCompleteIsUvpaaRequestFunction() = default;
WebAuthenticationProxyCompleteIsUvpaaRequestFunction::
    ~WebAuthenticationProxyCompleteIsUvpaaRequestFunction() = default;

ExtensionFunction::ResponseAction
WebAuthenticationProxyCompleteIsUvpaaRequestFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
