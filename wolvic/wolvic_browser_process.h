// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Copy from chrome/browser/browser_process.h

// This interface is for managing the global services of the application. Each
// service is lazily created when requested the first time. The service getters
// will return NULL if the service is not available, so callers must check for
// this condition.

#ifndef WOLVIC_WOLVIC_BROWSER_PROCESS_H_
#define WOLVIC_WOLVIC_BROWSER_PROCESS_H_

#include "components/extensions/common/buildflags.h"

#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
#include "components/extensions/browser/chrome_extensions_browser_client.h"
#include "components/extensions/browser/event_router_forwarder.h"
#include "wolvic/wolvic_browser_context.h"
#endif

namespace wolvic {

// NOT THREAD SAFE, call only from the main thread.
// These functions shouldn't return NULL unless otherwise noted.
class WolvicBrowserProcess
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
 : public components_extensions::ChromeExtensionsBrowserClient::Delegate
#endif
{
 public:
  WolvicBrowserProcess();

  WolvicBrowserProcess(const WolvicBrowserProcess&) = delete;
  WolvicBrowserProcess& operator=(const WolvicBrowserProcess&) = delete;

  static WolvicBrowserProcess* GetInstance();
 
  void Init(WolvicBrowserContext* context, WolvicBrowserContext* otr_context);
  void StartTearDown();

#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  ~WolvicBrowserProcess() override;

  // Implement ChromeExtensionsBrowserClient::Delegate
  components_extensions::EventRouterForwarder*
  extension_event_router_forwarder() override;
  network::mojom::NetworkContext* GetNetworkContext() override;
  bool IsShuttingDown() override;
  std::string GetApplicationLocale() override;
  std::vector<content::BrowserContext*> GetAllBrowserContexts() override;
  content::BrowserContext*
  GetOriginalBrowserContext(content::BrowserContext* context) override;
  PrefService*
  GetPrefServiceForContext(content::BrowserContext* context) override;
#else
  ~WolvicBrowserProcess();
#endif
 private:
  bool tearing_down_ = false;
  raw_ptr<WolvicBrowserContext> context_;
  raw_ptr<WolvicBrowserContext> otr_context_;
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  std::unique_ptr<components_extensions::ChromeExtensionsBrowserClient>
      extensions_browser_client_;

  scoped_refptr<components_extensions::EventRouterForwarder>
      extension_event_router_forwarder_;
#endif
};

} // namespace wolvic

#endif  // WOLVIC_WOLVIC_BROWSER_PROCESS_H_
