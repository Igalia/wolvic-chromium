// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_CHROME_EXTENSION_HOST_DELEGATE_H_
#define COMPONENTS_EXTENSIONS_BROWSER_CHROME_EXTENSION_HOST_DELEGATE_H_

#include "extensions/browser/extension_host_delegate.h"
#include "extensions/common/extension_id.h"

namespace components_extensions {

// Chrome support for ExtensionHost.
class ChromeExtensionHostDelegate : public extensions::ExtensionHostDelegate {
 public:
  ChromeExtensionHostDelegate();
  ~ChromeExtensionHostDelegate() override;

  // ExtensionHostDelegate implementation.
  void OnExtensionHostCreated(content::WebContents* web_contents) override;
  void OnMainFrameCreatedForBackgroundPage(extensions::ExtensionHost* host) override;
  content::JavaScriptDialogManager* GetJavaScriptDialogManager() override;
  void CreateTab(std::unique_ptr<content::WebContents> web_contents,
                 const extensions::ExtensionId& extension_id,
                 WindowOpenDisposition disposition,
                 const blink::mojom::WindowFeatures& window_features,
                 bool user_gesture) override;
  void ProcessMediaAccessRequest(content::WebContents* web_contents,
                                 const content::MediaStreamRequest& request,
                                 content::MediaResponseCallback callback,
                                 const extensions::Extension* extension) override;
  bool CheckMediaAccessPermission(content::RenderFrameHost* render_frame_host,
                                  const url::Origin& security_origin,
                                  blink::mojom::MediaStreamType type,
                                  const extensions::Extension* extension) override;
  content::PictureInPictureResult EnterPictureInPicture(
      content::WebContents* web_contents) override;
  void ExitPictureInPicture() override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_CHROME_EXTENSION_HOST_DELEGATE_H_
