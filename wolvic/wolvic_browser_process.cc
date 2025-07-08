// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/wolvic_browser_process.h"

#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
#include "base/android/locale_utils.h"
#include "content/public/browser/storage_partition.h"
#include "components/extensions/common/initialize_extensions_client.h"
#endif

namespace wolvic {

namespace {
WolvicBrowserProcess* g_wolvic_browser_process = nullptr;
}  // namespace

// static
WolvicBrowserProcess* WolvicBrowserProcess::GetInstance() {
  return g_wolvic_browser_process;
}

WolvicBrowserProcess::WolvicBrowserProcess() {
  g_wolvic_browser_process = this;
}

WolvicBrowserProcess::~WolvicBrowserProcess() {
  context_ = nullptr;
  otr_context_ = nullptr;
  g_wolvic_browser_process = nullptr;
}

void WolvicBrowserProcess::Init(WolvicBrowserContext* context, WolvicBrowserContext* otr_context) {
  context_ = context;
  otr_context_ = otr_context;
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  // TODO(mshin): Enable the below code after migrating AppWindowClient
  // components_extensions::AppWindowClient::Set(ChromeAppWindowClient::GetInstance());

  extension_event_router_forwarder_ =
      base::MakeRefCounted<components_extensions::EventRouterForwarder>();

  components_extensions::EnsureExtensionsClientInitialized();

  extensions_browser_client_ =
      std::make_unique<components_extensions::ChromeExtensionsBrowserClient>(this);

  extensions::ExtensionsBrowserClient::Set(extensions_browser_client_.get());
#endif
}

void WolvicBrowserProcess::StartTearDown() {
  tearing_down_ = true;
  DCHECK(IsShuttingDown());
}

#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
components_extensions::EventRouterForwarder*
WolvicBrowserProcess::extension_event_router_forwarder() {
  return extension_event_router_forwarder_.get();
}

network::mojom::NetworkContext* WolvicBrowserProcess::GetNetworkContext() {
  DCHECK(context_);
  return context_->GetDefaultStoragePartition()->GetNetworkContext();
}


bool WolvicBrowserProcess::IsShuttingDown() {
  return tearing_down_;
}

std::string WolvicBrowserProcess::GetApplicationLocale() {
  return base::android::GetDefaultLocaleString();
}

std::vector<content::BrowserContext*> WolvicBrowserProcess::GetAllBrowserContexts() {
  std::vector<content::BrowserContext*> result;
  if (context_) {
    result.push_back(context_.get());
  }
  if (otr_context_) {
    result.push_back(otr_context_.get());
  }
  return result;
}

content::BrowserContext*
WolvicBrowserProcess::GetOriginalBrowserContext(content::BrowserContext* context) {
  return context_ ? context_.get() : context;
}

PrefService*
WolvicBrowserProcess::GetPrefServiceForContext(content::BrowserContext* context) {
  return static_cast<WolvicBrowserContext*>(context)->GetPrefService();
}
#endif

} // namespace wolvic
