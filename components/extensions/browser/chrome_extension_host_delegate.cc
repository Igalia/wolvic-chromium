// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/chrome_extension_host_delegate.h"

#include <memory>
#include <string>

#include "components/extensions/browser/extension_service.h"
#include "components/javascript_dialogs/app_modal_dialog_manager.h"
#include "extensions/browser/extension_host.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/extension_id.h"
#include "third_party/blink/public/mojom/mediastream/media_stream.mojom.h"

using extensions::Extension;
using extensions::ExtensionId;
using extensions::ExtensionHost;
using extensions::ExtensionSystem;

namespace components_extensions {

ChromeExtensionHostDelegate::ChromeExtensionHostDelegate() {}

ChromeExtensionHostDelegate::~ChromeExtensionHostDelegate() {}

void ChromeExtensionHostDelegate::OnExtensionHostCreated(
    content::WebContents* web_contents) {
}

void ChromeExtensionHostDelegate::OnMainFrameCreatedForBackgroundPage(
    ExtensionHost* host) {
  ExtensionService* service =
      ExtensionSystem::Get(host->browser_context())->extension_service();
  if (service)
    service->DidCreateMainFrameForBackgroundPage(host);
}

content::JavaScriptDialogManager*
ChromeExtensionHostDelegate::GetJavaScriptDialogManager() {
  return javascript_dialogs::AppModalDialogManager::GetInstance();
}

void ChromeExtensionHostDelegate::CreateTab(
    std::unique_ptr<content::WebContents> web_contents,
    const ExtensionId& extension_id,
    WindowOpenDisposition disposition,
    const blink::mojom::WindowFeatures& window_features,
    bool user_gesture) {
  // TODO(mshin): Enable the below code after migrating ExtensionTabUtil
  // Verify that the browser is not shutting down. It can be the case if the
  // call is propagated through a posted task that was already in the queue when
  // shutdown started. See crbug.com/625646
  // if (g_browser_process->IsShuttingDown())
  //   return;

  // ExtensionTabUtil::CreateTab(std::move(web_contents), extension_id,
  //                             disposition, window_features, user_gesture);
}

void ChromeExtensionHostDelegate::ProcessMediaAccessRequest(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    content::MediaResponseCallback callback,
    const Extension* extension) {
  // TODO(mshin): Implement Media Capture permission
  // MediaCaptureDevicesDispatcher::GetInstance()->ProcessMediaAccessRequest(
  //     web_contents, request, std::move(callback), extension);
  std::move(callback).Run(blink::mojom::StreamDevicesSet(),
                          blink::mojom::MediaStreamRequestResult::NOT_SUPPORTED,
                          nullptr);
}

bool ChromeExtensionHostDelegate::CheckMediaAccessPermission(
    content::RenderFrameHost* render_frame_host,
    const url::Origin& security_origin,
    blink::mojom::MediaStreamType type,
    const Extension* extension) {
  // TODO(mshin): Implement Media access permission
  // return MediaCaptureDevicesDispatcher::GetInstance()
  //     ->CheckMediaAccessPermission(render_frame_host, security_origin, type,
  //                                  extension);
  return false;
}

content::PictureInPictureResult
ChromeExtensionHostDelegate::EnterPictureInPicture(
    content::WebContents* web_contents) {
  // TODO(mshin): Implement PIP
  return content::PictureInPictureResult::kNotSupported;
}

void ChromeExtensionHostDelegate::ExitPictureInPicture() {
  // TODO(mshin): Implement PIP
}

}  // namespace components_extensions
