// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Implements the Chrome Extensions Cookies API.

#include "components/extensions/browser/api/cookies/cookies_api.h"

#include <memory>
#include <utility>
#include <vector>

#include "base/feature_list.h"
#include "base/functional/bind.h"
#include "base/json/json_writer.h"
#include "base/lazy_instance.h"
#include "base/time/time.h"
#include "components/safe_browsing/core/common/features.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/storage_partition.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/common/error_utils.h"
#include "extensions/common/extension.h"
#include "extensions/common/permissions/permissions_data.h"
#include "net/cookies/canonical_cookie.h"
#include "net/cookies/cookie_constants.h"
#include "services/network/public/mojom/network_service.mojom.h"

using content::BrowserThread;

namespace extensions {

CookiesEventRouter::CookieChangeListener::CookieChangeListener(
    CookiesEventRouter* router,
    bool otr)
    : router_(router), otr_(otr) {}
CookiesEventRouter::CookieChangeListener::~CookieChangeListener() = default;

void CookiesEventRouter::CookieChangeListener::OnCookieChange(
    const net::CookieChangeInfo& change) {
  router_->OnCookieChange(otr_, change);
}

CookiesEventRouter::CookiesEventRouter(content::BrowserContext* context)
    : browser_context_(context) {
  MaybeStartListening();
  // BrowserList::AddObserver(this);
}

CookiesEventRouter::~CookiesEventRouter() {
  // BrowserList::RemoveObserver(this);
}

void CookiesEventRouter::OnCookieChange(bool otr,
                                        const net::CookieChangeInfo& change) {
}

// TODO(mshin): Implement BrowserListObserver
// void CookiesEventRouter::OnBrowserAdded(Browser* browser) {
//   // The new browser may be associated with a profile that is the OTR spinoff
//   // of |profile_|, in which case we need to start listening to cookie changes
//   // there. If this is any other kind of new browser, MaybeStartListening() will
//   // be a no op.
//   MaybeStartListening();
// }

void CookiesEventRouter::MaybeStartListening() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK(browser_context_);

  content::BrowserContext* otr_browser_context =
      browser_context_->GetOTRBrowserContext();

  if (!receiver_.is_bound())
    BindToCookieManager(&receiver_, browser_context_);
  if (!otr_receiver_.is_bound() && otr_browser_context)
    BindToCookieManager(&otr_receiver_, otr_browser_context);
}

void CookiesEventRouter::BindToCookieManager(
    mojo::Receiver<network::mojom::CookieChangeListener>* receiver,
    content::BrowserContext* context) {
  network::mojom::CookieManager* cookie_manager =
      context->GetDefaultStoragePartition()
          ->GetCookieManagerForBrowserProcess();
  if (!cookie_manager)
    return;

  cookie_manager->AddGlobalChangeListener(receiver->BindNewPipeAndPassRemote());
  receiver->set_disconnect_handler(
      base::BindOnce(&CookiesEventRouter::OnConnectionError,
                     base::Unretained(this), receiver));
}

void CookiesEventRouter::OnConnectionError(
    mojo::Receiver<network::mojom::CookieChangeListener>* receiver) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  receiver->reset();
  MaybeStartListening();
}

void CookiesEventRouter::DispatchEvent(content::BrowserContext* context,
                                       events::HistogramValue histogram_value,
                                       const std::string& event_name,
                                       base::Value::List event_args,
                                       const GURL& cookie_domain) {
  EventRouter* router = context ? EventRouter::Get(context) : nullptr;
  if (!router)
    return;
  auto event = std::make_unique<Event>(histogram_value, event_name,
                                       std::move(event_args), context);
  event->event_url = cookie_domain;
  router->BroadcastEvent(std::move(event));
}

CookiesGetFunction::CookiesGetFunction() = default;
CookiesGetFunction::~CookiesGetFunction() = default;

ExtensionFunction::ResponseAction CookiesGetFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void CookiesGetFunction::GetCookieListCallback(
    const net::CookieAccessResultList& cookie_list,
    const net::CookieAccessResultList& excluded_cookies) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
}

void CookiesGetFunction::NotifyExtensionTelemetry() {
}

CookiesGetAllFunction::CookiesGetAllFunction() {
}

CookiesGetAllFunction::~CookiesGetAllFunction() {
}

ExtensionFunction::ResponseAction CookiesGetAllFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void CookiesGetAllFunction::GetAllCookiesCallback(
    const net::CookieList& cookie_list) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
}

void CookiesGetAllFunction::GetCookieListCallback(
    const net::CookieAccessResultList& cookie_list,
    const net::CookieAccessResultList& excluded_cookies) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
}

void CookiesGetAllFunction::NotifyExtensionTelemetry() {
  // TODO(mshin): Support safe browsing
}

CookiesSetFunction::CookiesSetFunction()
    : state_(NO_RESPONSE), success_(false) {}

CookiesSetFunction::~CookiesSetFunction() {
}

ExtensionFunction::ResponseAction CookiesSetFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void CookiesSetFunction::SetCanonicalCookieCallback(
    net::CookieAccessResult set_cookie_result) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK_EQ(NO_RESPONSE, state_);
  state_ = SET_COMPLETED;
  success_ = set_cookie_result.status.IsInclude();
}

void CookiesSetFunction::GetCookieListCallback(
    const net::CookieAccessResultList& cookie_list,
    const net::CookieAccessResultList& excluded_cookies) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK_EQ(SET_COMPLETED, state_);
  state_ = GET_COMPLETED;
}

CookiesRemoveFunction::CookiesRemoveFunction() {
}

CookiesRemoveFunction::~CookiesRemoveFunction() {
}

ExtensionFunction::ResponseAction CookiesRemoveFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void CookiesRemoveFunction::RemoveCookieCallback(uint32_t /* num_deleted */) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  // Build the callback result
  api::cookies::Remove::Results::Details details;
  details.name = parsed_args_->details.name;
  details.url = url_.spec();
  details.store_id = *parsed_args_->details.store_id;
  if (parsed_args_->details.partition_key) {
    details.partition_key = parsed_args_->details.partition_key->Clone();
  }

  Respond(ArgumentList(api::cookies::Remove::Results::Create(details)));
}

ExtensionFunction::ResponseAction CookiesGetAllCookieStoresFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

CookiesAPI::CookiesAPI(content::BrowserContext* context)
    : browser_context_(context) {
  EventRouter::Get(browser_context_)
      ->RegisterObserver(this, api::cookies::OnChanged::kEventName);
}

CookiesAPI::~CookiesAPI() = default;

void CookiesAPI::Shutdown() {
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

static base::LazyInstance<BrowserContextKeyedAPIFactory<CookiesAPI>>::
    DestructorAtExit g_cookies_api_factory = LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<CookiesAPI>* CookiesAPI::GetFactoryInstance() {
  return g_cookies_api_factory.Pointer();
}

void CookiesAPI::OnListenerAdded(const EventListenerInfo& details) {
  cookies_event_router_ =
      std::make_unique<CookiesEventRouter>(browser_context_);
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

}  // namespace extensions
