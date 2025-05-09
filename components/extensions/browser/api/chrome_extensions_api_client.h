// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_CHROME_EXTENSIONS_API_CLIENT_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_CHROME_EXTENSIONS_API_CLIENT_H_

#include "extensions/browser/api/extensions_api_client.h"

namespace components_extensions {

class ChromeAutomationInternalApiDelegate;
class ChromeMetricsPrivateDelegate;
class ClipboardExtensionHelper;

// Extra support for extensions APIs in Chrome.
class ChromeExtensionsAPIClient : public extensions::ExtensionsAPIClient {
 public:
  ChromeExtensionsAPIClient();

  ChromeExtensionsAPIClient(const ChromeExtensionsAPIClient&) = delete;
  ChromeExtensionsAPIClient& operator=(const ChromeExtensionsAPIClient&) =
      delete;

  ~ChromeExtensionsAPIClient() override;

  // ExtensionsApiClient implementation.
  void AddAdditionalValueStoreCaches(
      content::BrowserContext* context,
      const scoped_refptr<value_store::ValueStoreFactory>& factory,
      extensions::SettingsChangedCallback observer,
      std::map<extensions::settings_namespace::Namespace, extensions::ValueStoreCache*>* caches)
      override;
  void AttachWebContentsHelpers(content::WebContents* web_contents) const
      override;
  bool ShouldHideResponseHeader(const GURL& url,
                                const std::string& header_name) const override;
  bool ShouldHideBrowserNetworkRequest(
      content::BrowserContext* context,
      const extensions::WebRequestInfo& request) const override;
  void NotifyWebRequestWithheld(int render_process_id,
                                int render_frame_id,
                                const extensions::ExtensionId& extension_id) override;
  void UpdateActionCount(content::BrowserContext* context,
                         const extensions::ExtensionId& extension_id,
                         int tab_id,
                         int action_count,
                         bool clear_badge_text) override;
  void ClearActionCount(content::BrowserContext* context,
                        const extensions::Extension& extension) override;
  void OpenFileUrl(const GURL& file_url,
                   content::BrowserContext* browser_context) override;
  extensions::AppViewGuestDelegate* CreateAppViewGuestDelegate() const override;
  extensions::ExtensionOptionsGuestDelegate* CreateExtensionOptionsGuestDelegate(
      extensions::ExtensionOptionsGuest* guest) const override;
  std::unique_ptr<guest_view::GuestViewManagerDelegate>
  CreateGuestViewManagerDelegate() const override;
  std::unique_ptr<extensions::MimeHandlerViewGuestDelegate>
  CreateMimeHandlerViewGuestDelegate(
      extensions::MimeHandlerViewGuest* guest) const override;
  extensions::WebViewGuestDelegate* CreateWebViewGuestDelegate(
      extensions::WebViewGuest* web_view_guest) const override;
  extensions::WebViewPermissionHelperDelegate* CreateWebViewPermissionHelperDelegate(
      extensions::WebViewPermissionHelper* web_view_permission_helper) const override;
  scoped_refptr<extensions::ContentRulesRegistry> CreateContentRulesRegistry(
      content::BrowserContext* browser_context,
      extensions::RulesCacheDelegate* cache_delegate) const override;
  std::unique_ptr<extensions::DevicePermissionsPrompt> CreateDevicePermissionsPrompt(
      content::WebContents* web_contents) const override;
  std::unique_ptr<extensions::VirtualKeyboardDelegate> CreateVirtualKeyboardDelegate(
      content::BrowserContext* browser_context) const override;
  extensions::ManagementAPIDelegate* CreateManagementAPIDelegate() const override;
  std::unique_ptr<extensions::SupervisedUserExtensionsDelegate>
  CreateSupervisedUserExtensionsDelegate(
      content::BrowserContext* browser_context) const override;

  std::unique_ptr<extensions::DisplayInfoProvider> CreateDisplayInfoProvider()
      const override;
  extensions::MetricsPrivateDelegate* GetMetricsPrivateDelegate() override;
  extensions::FileSystemDelegate* GetFileSystemDelegate() override;
  extensions::MessagingDelegate* GetMessagingDelegate() override;
  extensions::FeedbackPrivateDelegate* GetFeedbackPrivateDelegate() override;

  extensions::AutomationInternalApiDelegate* GetAutomationInternalApiDelegate() override;
  std::vector<KeyedServiceBaseFactory*> GetFactoryDependencies() override;

 private:
  // TODO(mshin): Enable each member after migrating Delegates
  // std::unique_ptr<ChromeMetricsPrivateDelegate> metrics_private_delegate_;
  // std::unique_ptr<FileSystemDelegate> file_system_delegate_;
  // std::unique_ptr<MessagingDelegate> messaging_delegate_;
  // std::unique_ptr<FeedbackPrivateDelegate> feedback_private_delegate_;
  // std::unique_ptr<ChromeAutomationInternalApiDelegate>
  //     extensions_automation_api_delegate_;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_CHROME_EXTENSIONS_API_CLIENT_H_
