// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/chrome_extension_frame_host.h"

#include "components/extensions/common/extension_constants.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_util.h"
#include "extensions/common/extension_id.h"
#include "extensions/common/extension_set.h"
#include "extensions/common/extension_urls.h"
#include "third_party/blink/public/common/logging/logging_utils.h"
#include "url/gurl.h"

using extensions::ExtensionId;
using extensions::ExtensionRegistry;
using extensions::ExtensionSet;
using extensions::StackTrace;

namespace components_extensions {

ChromeExtensionFrameHost::ChromeExtensionFrameHost(
    content::WebContents* web_contents)
    : ExtensionFrameHost(web_contents) {}

ChromeExtensionFrameHost::~ChromeExtensionFrameHost() = default;

void ChromeExtensionFrameHost::RequestScriptInjectionPermission(
    const ExtensionId& extension_id,
    extensions::mojom::InjectionType script_type,
    extensions::mojom::RunLocation run_location,
    RequestScriptInjectionPermissionCallback callback) {

  // TODO(mshin): Remove & Enable the below code when migrating ExtensionActionRunner
  std::move(callback).Run(false);
  // ExtensionActionRunner* runner =
  //     ExtensionActionRunner::GetForWebContents(web_contents_);
  // if (!runner) {
  //   std::move(callback).Run(false);
  //   return;
  // }
  // runner->OnRequestScriptInjectionPermission(extension_id, script_type,
  //                                            run_location, std::move(callback));
}

void ChromeExtensionFrameHost::GetAppInstallState(
    const GURL& requestor_url,
    GetAppInstallStateCallback callback) {
  ExtensionRegistry* registry =
      ExtensionRegistry::Get(web_contents_->GetBrowserContext());
  const ExtensionSet& extensions = registry->enabled_extensions();
  const ExtensionSet& disabled_extensions = registry->disabled_extensions();

  std::string state;
  if (extensions.GetHostedAppByURL(requestor_url))
    state = extension_misc::kAppStateInstalled;
  else if (disabled_extensions.GetHostedAppByURL(requestor_url))
    state = extension_misc::kAppStateDisabled;
  else
    state = extension_misc::kAppStateNotInstalled;

  std::move(callback).Run(state);
}

void ChromeExtensionFrameHost::WatchedPageChange(
    const std::vector<std::string>& css_selectors) {
  // TODO(mshin) : Enable the below code after migrating TabHelper
  // TabHelper* tab_helper = TabHelper::FromWebContents(web_contents_);
  // if (!tab_helper)
  //   return;
  // tab_helper->OnWatchedPageChanged(css_selectors);
}

void ChromeExtensionFrameHost::DetailedConsoleMessageAdded(
    const std::u16string& message,
    const std::u16string& source,
    const StackTrace& stack_trace,
    blink::mojom::ConsoleMessageLevel level) {
  if (!extensions::IsSourceFromAnExtension(source))
    return;

  content::RenderFrameHost* render_frame_host =
      receivers_.GetCurrentTargetFrame();
  ExtensionId extension_id = extensions::util::GetExtensionIdFromFrame(render_frame_host);
  if (extension_id.empty())
    extension_id = GURL(source).host();

  // TODO(mshin) : Enable the below code aftre migrating ErrorConsole
  // content::BrowserContext* browser_context = web_contents_->GetBrowserContext();
  // ErrorConsole::Get(browser_context)
  //     ->ReportError(std::unique_ptr<ExtensionError>(new RuntimeError(
  //         extension_id, browser_context->IsOffTheRecord(), source, message,
  //         stack_trace, web_contents_->GetLastCommittedURL(),
  //         blink::ConsoleMessageLevelToLogSeverity(level),
  //         render_frame_host->GetRoutingID(),
  //         render_frame_host->GetProcess()->GetID())));
}

void ChromeExtensionFrameHost::ContentScriptsExecuting(
    const base::flat_map<ExtensionId, std::vector<std::string>>&
        extension_id_to_scripts,
    const GURL& frame_url) {
  // TODO(mshin) : Enable the below code after migrating ActivityLog
  // ActivityLog::GetInstance(web_contents_->GetBrowserContext())
  //     ->OnScriptsExecuted(web_contents_, extension_id_to_scripts, frame_url);
}

}  // namespace components_extensions
