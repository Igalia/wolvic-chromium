// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/event_router_forwarder.h"

#include <stddef.h>
#include <memory>
#include <utility>

#include "base/functional/bind.h"
#include "base/values.h"
#include "build/build_config.h"
#include "components/extensions/browser/chrome_extensions_browser_client.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/event_router.h"
#include "extensions/common/extension_id.h"
#include "url/gurl.h"

using content::BrowserThread;

namespace components_extensions {

EventRouterForwarder::EventRouterForwarder() {
}

EventRouterForwarder::~EventRouterForwarder() {
}

void EventRouterForwarder::BroadcastEventToRenderers(
    extensions::events::HistogramValue histogram_value,
    const std::string& event_name,
    base::Value::List event_args,
    const GURL& event_url,
    bool dispatch_to_off_the_record_profiles) {
  HandleEvent(std::string(), histogram_value, event_name, std::move(event_args),
              nullptr, true, event_url, dispatch_to_off_the_record_profiles);
}

void EventRouterForwarder::DispatchEventToRenderers(
    extensions::events::HistogramValue histogram_value,
    const std::string& event_name,
    base::Value::List event_args,
    content::BrowserContext* browser_context,
    bool use_profile_to_restrict_events,
    const GURL& event_url,
    bool dispatch_to_off_the_record_profiles) {
  if (!browser_context)
    return;
  HandleEvent(std::string(), histogram_value, event_name, std::move(event_args),
              browser_context, use_profile_to_restrict_events, event_url,
              dispatch_to_off_the_record_profiles);
}

void EventRouterForwarder::HandleEvent(
    const extensions::ExtensionId& extension_id,
    extensions::events::HistogramValue histogram_value,
    const std::string& event_name,
    base::Value::List event_args,
    content::BrowserContext* browser_context,
    bool use_profile_to_restrict_events,
    const GURL& event_url,
    bool dispatch_to_off_the_record_profiles) {
  if (!BrowserThread::CurrentlyOn(BrowserThread::UI)) {
    content::GetUIThreadTaskRunner({})->PostTask(
        FROM_HERE,
        base::BindOnce(&EventRouterForwarder::HandleEvent, this, extension_id,
                       histogram_value, event_name, std::move(event_args),
                       browser_context, use_profile_to_restrict_events, event_url,
                       dispatch_to_off_the_record_profiles));
    return;
  }

  std::set<content::BrowserContext*> browser_contexts_to_dispatch_to;
  if (browser_context) {
    browser_contexts_to_dispatch_to.insert(browser_context);
  } else {
    auto contexts = static_cast<ChromeExtensionsBrowserClient*>(
                        extensions::ExtensionsBrowserClient::Get())
                        ->GetAllBrowserContexts();

    for (auto* context : contexts) {
      if (context->IsOffTheRecord() && !dispatch_to_off_the_record_profiles) {
        continue;
      }
      browser_contexts_to_dispatch_to.insert(context);
    }
  }

  // There should always be at least one profile when running as Chromium.
  // However, some Chromium embedders are known to run without profiles, in
  // which case there's nothing to dispatch to.
  if (browser_contexts_to_dispatch_to.size() == 0u)
    return;

  for (auto* browser_context_to_dispatch_to : browser_contexts_to_dispatch_to) {
    CallEventRouter(
      browser_context_to_dispatch_to, extension_id, histogram_value, event_name,
      browser_context_to_dispatch_to != *std::prev(browser_contexts_to_dispatch_to.end())
          ? event_args.Clone()
          : std::move(event_args),
      use_profile_to_restrict_events ? browser_context_to_dispatch_to : nullptr,
      event_url);
  }
}

void EventRouterForwarder::CallEventRouter(
    content::BrowserContext* browser_context,
    const extensions::ExtensionId& extension_id,
    extensions::events::HistogramValue histogram_value,
    const std::string& event_name,
    base::Value::List event_args,
    content::BrowserContext* restrict_to_profile,
    const GURL& event_url) {
  auto* event_router = extensions::EventRouter::Get(browser_context);
  // Extension does not exist for chromeos login.  This needs to be
  // removed once we have an extension service for login screen.
  // crosbug.com/12856.
  //
  // Extensions are not available on System Profile.
  if (!event_router)
    return;

  auto event = std::make_unique<extensions::Event>(
      histogram_value, event_name, std::move(event_args));
  event->event_url = event_url;
  if (extension_id.empty()) {
    event_router->BroadcastEvent(std::move(event));
  } else {
    event_router->DispatchEventToExtension(extension_id, std::move(event));
  }
}

}  // namespace components_extensions
