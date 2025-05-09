// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/chrome_extensions_api_client.h"

#include <memory>
#include <utility>

#include "base/check.h"
#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/memory/scoped_refptr.h"
#include "base/strings/string_util.h"
#include "build/build_config.h"
#include "components/extensions/browser/extension_util.h"
#include "components/signin/core/browser/signin_header_helper.h"
#include "components/supervised_user/core/common/buildflags.h"
#include "components/value_store/value_store_factory.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/api/system_display/display_info_provider.h"
#include "extensions/browser/api/virtual_keyboard_private/virtual_keyboard_delegate.h"
#include "extensions/browser/api/web_request/web_request_info.h"
#include "extensions/browser/extension_action.h"
#include "extensions/browser/extension_action_manager.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/guest_view/web_view/web_view_guest.h"
#include "extensions/browser/guest_view/web_view/web_view_permission_helper.h"
#include "extensions/browser/supervised_user_extensions_delegate.h"
#include "google_apis/gaia/gaia_urls.h"
#include "pdf/buildflags.h"
#include "printing/buildflags/buildflags.h"
#include "services/network/public/mojom/fetch_api.mojom-shared.h"
#include "ui/base/page_transition_types.h"
#include "ui/base/window_open_disposition.h"
#include "url/gurl.h"

using extensions::AppViewGuestDelegate;
using extensions::AutomationInternalApiDelegate;
using extensions::ContentRulesRegistry;
using extensions::DevicePermissionsPrompt;
using extensions::DisplayInfoProvider;
using extensions::Extension;
using extensions::ExtensionId;
using extensions::ExtensionOptionsGuest;
using extensions::ExtensionOptionsGuestDelegate;
using extensions::ExtensionRegistry;
using extensions::FeedbackPrivateDelegate;
using extensions::FileSystemDelegate;
using extensions::ManagementAPIDelegate;
using extensions::MessagingDelegate;
using extensions::MetricsPrivateDelegate;
using extensions::MimeHandlerViewGuest;
using extensions::MimeHandlerViewGuestDelegate;
using extensions::RulesCacheDelegate;
using extensions::SettingsChangedCallback;
using extensions::SupervisedUserExtensionsDelegate;
using extensions::ValueStoreCache;
using extensions::VirtualKeyboardDelegate;
using extensions::WebRequestInfo;
using extensions::WebViewGuest;
using extensions::WebViewGuestDelegate;
using extensions::ContentRulesRegistry;
using extensions::WebViewPermissionHelper;
using extensions::WebViewPermissionHelperDelegate;


namespace components_extensions {

ChromeExtensionsAPIClient::ChromeExtensionsAPIClient() = default;

ChromeExtensionsAPIClient::~ChromeExtensionsAPIClient() {}

void ChromeExtensionsAPIClient::AddAdditionalValueStoreCaches(
    content::BrowserContext* context,
    const scoped_refptr<value_store::ValueStoreFactory>& factory,
    SettingsChangedCallback observer,
    std::map<extensions::settings_namespace::Namespace, ValueStoreCache*>* caches) {
  // TODO(mshin): Enable the below code after migrating SyncValueStoreCache
  // Add support for chrome.storage.sync.
  // (*caches)[settings_namespace::SYNC] =
  //     new SyncValueStoreCache(factory, observer, context->GetPath());

  // TODO(mshin): Enable the below code after migrating ManagedValueStoreCache
  // Add support for chrome.storage.managed.
  // (*caches)[settings_namespace::MANAGED] = new ManagedValueStoreCache(
  //     *context, factory, observer);
}

void ChromeExtensionsAPIClient::AttachWebContentsHelpers(
    content::WebContents* web_contents) const {
  // TODO(mshin): Enable the below code after supporting Favicon
  // favicon::CreateContentFaviconDriverForWebContents(web_contents);

// TODO(mshin): Enable the below code after supporting Printing
// #if BUILDFLAG(ENABLE_PRINTING)
//   printing::InitializePrintingForWebContents(web_contents);
// #endif
}

bool ChromeExtensionsAPIClient::ShouldHideResponseHeader(
    const GURL& url,
    const std::string& header_name) const {
  // Gaia may send a OAUth2 authorization code in the Dice response header,
  // which could allow an extension to generate a refresh token for the account.
  return (
      (url.host_piece() == GaiaUrls::GetInstance()->gaia_url().host_piece()) &&
      (base::CompareCaseInsensitiveASCII(header_name,
                                         signin::kDiceResponseHeader) == 0));
}

bool ChromeExtensionsAPIClient::ShouldHideBrowserNetworkRequest(
    content::BrowserContext* context,
    const WebRequestInfo& request) const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  // Note: browser initiated non-navigation requests are hidden from extensions.
  // But we do still need to protect some sensitive sub-frame navigation
  // requests.
  // Exclude main frame navigation requests.
  // TODO(mshin): Enable the below code after supporting DevTools
  // bool is_browser_request =
  //     request.render_process_id == -1 &&
  //     request.web_request_type != extensions::WebRequestResourceType::MAIN_FRAME;

  // Hide requests made by the Devtools frontend.
  // bool is_sensitive_request =
  //     is_browser_request && DevToolsUI::IsFrontendResourceURL(request.url);

  // return is_sensitive_request;
  return false;
}

void ChromeExtensionsAPIClient::NotifyWebRequestWithheld(
    int render_process_id,
    int render_frame_id,
    const ExtensionId& extension_id) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  // Track down the ExtensionActionRunner and the extension. Since this is
  // asynchronous, we could hit a null anywhere along the path.
  content::RenderFrameHost* render_frame_host =
      content::RenderFrameHost::FromID(render_process_id, render_frame_id);
  if (!render_frame_host) {
    return;
  }
  // We don't count subframes and prerendering blocked actions as yet, since
  // there's no way to surface this to the user. Ignore these (which is also
  // what we do for content scripts).
  if (!render_frame_host->IsInPrimaryMainFrame()) {
    return;
  }
  content::WebContents* web_contents =
      content::WebContents::FromRenderFrameHost(render_frame_host);
  if (!web_contents)
    return;
  // TODO(mshin): Enable the below code after migrating ExtensionActionRunner
  // ExtensionActionRunner* runner =
  //     ExtensionActionRunner::GetForWebContents(web_contents);
  // if (!runner)
  //   return;

  const extensions::Extension* extension =
      extensions::ExtensionRegistry::Get(web_contents->GetBrowserContext())
          ->enabled_extensions()
          .GetByID(extension_id);
  if (!extension)
    return;

  // If the extension doesn't request access to the tab, return. The user
  // invoking the extension on a site grants access to the tab's origin if
  // and only if the extension requested it; without requesting the tab,
  // clicking on the extension won't grant access to the resource.
  // https://crbug.com/891586.
  // TODO(https://157736): We can remove this if extensions require host
  // permissions to the initiator, since then we'll never get into this type
  // of circumstance (the request would be blocked, rather than withheld).
  // TODO(mshin): Enable the below code after migrating Permission
  // if (!extension->permissions_data()
  //          ->withheld_permissions()
  //          .explicit_hosts()
  //          .MatchesURL(render_frame_host->GetLastCommittedURL())) {
  //   return;
  // }

  // TODO(mshin): Enable the below code after migrating ExtensionActionRunner
  // runner->OnWebRequestBlocked(extension);
}

void ChromeExtensionsAPIClient::UpdateActionCount(
    content::BrowserContext* context,
    const ExtensionId& extension_id,
    int tab_id,
    int action_count,
    bool clear_badge_text) {
  const Extension* extension =
      ExtensionRegistry::Get(context)->enabled_extensions().GetByID(
          extension_id);
  DCHECK(extension);

  // TODO(mshin): Enable the below code after migrating ExtensionAction
  // ExtensionAction* action =
  //     ExtensionActionManager::Get(context)->GetExtensionAction(*extension);
  // DCHECK(action);

  // action->SetDNRActionCount(tab_id, action_count);

  // // The badge text should be cleared if |action| contains explicitly set badge
  // // text for the |tab_id| when the preference is then toggled on. In this case,
  // // the matched action count should take precedence over the badge text.
  // if (clear_badge_text)
  //   action->ClearBadgeText(tab_id);

  // content::WebContents* tab_contents = nullptr;
  // if (ExtensionTabUtil::GetTabById(
  //         tab_id, context, true /* include_incognito */, &tab_contents) &&
  //     tab_contents) {
  //   ExtensionActionAPI::Get(context)->NotifyChange(action, tab_contents,
  //                                                  context);
  // }
}

void ChromeExtensionsAPIClient::ClearActionCount(
    content::BrowserContext* context,
    const Extension& extension) {
  // TODO(mshin): Enable the below code after migrating ExtensionAction
  // ExtensionAction* action =
  //     ExtensionActionManager::Get(context)->GetExtensionAction(extension);
  // DCHECK(action);

  // action->ClearDNRActionCountForAllTabs();

  // std::vector<content::WebContents*> contents_to_notify =
  //     ExtensionTabUtil::GetAllActiveWebContentsForContext(
  //         context, true /* include_incognito */);

  // for (auto* active_contents : contents_to_notify) {
  //   ExtensionActionAPI::Get(context)->NotifyChange(action, active_contents,
  //                                                  context);
  // }
}

void ChromeExtensionsAPIClient::OpenFileUrl(
    const GURL& file_url,
    content::BrowserContext* browser_context) {
  CHECK(file_url.is_valid());
  CHECK(file_url.SchemeIsFile());
  components_extensions::util::Navigate(file_url, browser_context);
}

AppViewGuestDelegate* ChromeExtensionsAPIClient::CreateAppViewGuestDelegate()
    const {
  // Not support because it's Platform API
  return nullptr;
}

ExtensionOptionsGuestDelegate*
ChromeExtensionsAPIClient::CreateExtensionOptionsGuestDelegate(
    ExtensionOptionsGuest* guest) const {
  // TODO(mshin): Enable the below code after migrating ChromeExtensionOptionsGuestDelegate
  // return new ChromeExtensionOptionsGuestDelegate(guest);
  return nullptr;
}

std::unique_ptr<guest_view::GuestViewManagerDelegate>
ChromeExtensionsAPIClient::CreateGuestViewManagerDelegate() const {
  // TODO(mshin): Enable the below code after migrating ChromeGuestViewManagerDelegate
  // return std::make_unique<ChromeGuestViewManagerDelegate>();
  return nullptr;
}

std::unique_ptr<MimeHandlerViewGuestDelegate>
ChromeExtensionsAPIClient::CreateMimeHandlerViewGuestDelegate(
    MimeHandlerViewGuest* guest) const {
  // TODO(mshin): Enable the below code after migrating ChromeMimeHandlerViewGuestDelegate
  // return std::make_unique<ChromeMimeHandlerViewGuestDelegate>();
  return nullptr;
}

WebViewGuestDelegate* ChromeExtensionsAPIClient::CreateWebViewGuestDelegate(
    WebViewGuest* web_view_guest) const {
  // Not support because it's Platform API
  return nullptr;
}

WebViewPermissionHelperDelegate*
ChromeExtensionsAPIClient::CreateWebViewPermissionHelperDelegate(
    WebViewPermissionHelper* web_view_permission_helper) const {
  // Not support because it's Platform API
  return nullptr;
}

scoped_refptr<ContentRulesRegistry>
ChromeExtensionsAPIClient::CreateContentRulesRegistry(
    content::BrowserContext* browser_context,
    RulesCacheDelegate* cache_delegate) const {
  // TODO(mshin): Enable the below code after migrating ChromeContentRulesRegistry
  // return base::MakeRefCounted<ChromeContentRulesRegistry>(
  //     browser_context, cache_delegate,
  //     base::BindOnce(&CreateDefaultContentPredicateEvaluators,
  //                    base::Unretained(browser_context)));
  return nullptr;
}

std::unique_ptr<DevicePermissionsPrompt>
ChromeExtensionsAPIClient::CreateDevicePermissionsPrompt(
    content::WebContents* web_contents) const {
  // TODO(mshin): Enable the below code after migrating ChromeDevicePermissionsPrompt
  // return std::make_unique<ChromeDevicePermissionsPrompt>(web_contents);
  return nullptr;
}

std::unique_ptr<VirtualKeyboardDelegate>
ChromeExtensionsAPIClient::CreateVirtualKeyboardDelegate(
    content::BrowserContext* browser_context) const {
  // Not support because it's ChromeOS API
  return nullptr;
}

ManagementAPIDelegate* ChromeExtensionsAPIClient::CreateManagementAPIDelegate()
    const {
  // TODO(mshin): Enable the below code after migrating ChromeManagementAPIDelegate
  // return new ChromeManagementAPIDelegate;
  return nullptr;
}

std::unique_ptr<SupervisedUserExtensionsDelegate>
ChromeExtensionsAPIClient::CreateSupervisedUserExtensionsDelegate(
    content::BrowserContext* browser_context) const {
  // Not support Supervised User
  return nullptr;
}

std::unique_ptr<DisplayInfoProvider>
ChromeExtensionsAPIClient::CreateDisplayInfoProvider() const {
  // TODO(mshin): Enable the below code after migrating CreateChromeDisplayInfoProvider
  // return CreateChromeDisplayInfoProvider();
  return nullptr;
}

MetricsPrivateDelegate* ChromeExtensionsAPIClient::GetMetricsPrivateDelegate() {
  // TODO(mshin): Enable the below code after migrating ChromeMetricsPrivateDelegate
  // if (!metrics_private_delegate_)
  //   metrics_private_delegate_ =
  //       std::make_unique<ChromeMetricsPrivateDelegate>();
  // return metrics_private_delegate_.get();
  return nullptr;
}

FileSystemDelegate* ChromeExtensionsAPIClient::GetFileSystemDelegate() {
  // Not support because it's Platform API
  return nullptr;
}

MessagingDelegate* ChromeExtensionsAPIClient::GetMessagingDelegate() {
  // TODO(mshin): Enable the below code after migrating ChromeMessagingDelegate
  // if (!messaging_delegate_)
  //   messaging_delegate_ = std::make_unique<ChromeMessagingDelegate>();
  // return messaging_delegate_.get();
  return nullptr;
}

FeedbackPrivateDelegate*
ChromeExtensionsAPIClient::GetFeedbackPrivateDelegate() {
  // TODO(mshin): Enable the below code after migrating ChromeFeedbackPrivateDelegate
  // if (!feedback_private_delegate_) {
  //   feedback_private_delegate_ =
  //       std::make_unique<ChromeFeedbackPrivateDelegate>();
  // }
  // return feedback_private_delegate_.get();
  return nullptr;
}

AutomationInternalApiDelegate*
ChromeExtensionsAPIClient::GetAutomationInternalApiDelegate() {
  // TODO(mshin): Enable the below code after migrating ChromeAutomationInternalApiDelegate
  // if (!extensions_automation_api_delegate_) {
  //   extensions_automation_api_delegate_ =
  //       std::make_unique<ChromeAutomationInternalApiDelegate>();
  // }
  // return extensions_automation_api_delegate_.get();
  return nullptr;
}

std::vector<KeyedServiceBaseFactory*>
ChromeExtensionsAPIClient::GetFactoryDependencies() {
  // clang-format off
  return {};
  // clang-format on
}

}  // namespace components_extensions
