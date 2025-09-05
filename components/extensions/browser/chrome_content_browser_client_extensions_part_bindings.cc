// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/chrome_content_browser_client_extensions_part.h"

#include "base/functional/bind.h"
#include "content/public/browser/service_worker_version_base_info.h"
#include "extensions/browser/api/automation_internal/automation_event_router.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/guest_view/extensions_guest_view.h"
#include "extensions/browser/renderer_startup_helper.h"
#include "extensions/browser/service_worker/service_worker_host.h"
#include "extensions/common/mojom/automation_registry.mojom.h"
#include "extensions/common/mojom/event_router.mojom.h"
#include "extensions/common/mojom/guest_view.mojom.h"
#include "extensions/common/mojom/renderer_host.mojom.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"

namespace components_extensions {

void ChromeContentBrowserClientExtensionsPart::ExposeInterfacesToRenderer(
    service_manager::BinderRegistry* registry,
    blink::AssociatedInterfaceRegistry* associated_registry,
    content::RenderProcessHost* host) {
  associated_registry->AddInterface<extensions::mojom::RendererHost>(
      base::BindRepeating(&extensions::RendererStartupHelper::BindForRenderer,
                          host->GetID()));
}

void ChromeContentBrowserClientExtensionsPart::
    ExposeInterfacesToRendererForServiceWorker(
        const content::ServiceWorkerVersionBaseInfo&
            service_worker_version_info,
        blink::AssociatedInterfaceRegistry& associated_registry) {
  CHECK(service_worker_version_info.process_id !=
        content::ChildProcessHost::kInvalidUniqueID);
  associated_registry.AddInterface<extensions::mojom::RendererHost>(
      base::BindRepeating(&extensions::RendererStartupHelper::BindForRenderer,
                          service_worker_version_info.process_id));
  associated_registry.AddInterface<extensions::mojom::ServiceWorkerHost>(
      base::BindRepeating(&extensions::ServiceWorkerHost::BindReceiver,
                          service_worker_version_info.process_id));
  associated_registry.AddInterface<extensions::mojom::RendererAutomationRegistry>(
      base::BindRepeating(&extensions::AutomationEventRouter::BindForRenderer,
                          service_worker_version_info.process_id));
  associated_registry.AddInterface<extensions::mojom::EventRouter>(
        base::BindRepeating(&extensions::EventRouter::BindForRenderer,
                            service_worker_version_info.process_id));
}

void ChromeContentBrowserClientExtensionsPart::
    ExposeInterfacesToRendererForRenderFrameHost(
        content::RenderFrameHost& frame_host,
        blink::AssociatedInterfaceRegistry& associated_registry) {
  int render_process_id = frame_host.GetProcess()->GetID();
  associated_registry.AddInterface<extensions::mojom::RendererHost>(
      base::BindRepeating(&extensions::RendererStartupHelper::BindForRenderer,
                          render_process_id));
  associated_registry.AddInterface<extensions::mojom::RendererAutomationRegistry>(
      base::BindRepeating(&extensions::AutomationEventRouter::BindForRenderer,
                          render_process_id));
  associated_registry.AddInterface<extensions::mojom::EventRouter>(
      base::BindRepeating(&extensions::EventRouter::BindForRenderer,
                           render_process_id));
  associated_registry.AddInterface<guest_view::mojom::GuestViewHost>(
      base::BindRepeating(&extensions::ExtensionsGuestView::CreateForComponents,
                          frame_host.GetGlobalId()));
  associated_registry.AddInterface<extensions::mojom::GuestView>(
        base::BindRepeating(&extensions::ExtensionsGuestView::CreateForExtensions,
                            frame_host.GetGlobalId()));
}

}  // namespace components_extensions
