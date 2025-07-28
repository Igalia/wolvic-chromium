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

  // Only support a single profile
  CallEventRouter(
      browser_context, extension_id, histogram_value, event_name,
      std::move(event_args),
      use_profile_to_restrict_events ? browser_context : nullptr,
      event_url);
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
