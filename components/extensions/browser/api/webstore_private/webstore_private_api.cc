// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/webstore_private/webstore_private_api.h"

#include <stddef.h>

#include <memory>
#include <utility>
#include <vector>

#include "base/auto_reset.h"
#include "base/base64.h"
#include "base/functional/bind.h"
#include "base/json/values_util.h"
#include "base/lazy_instance.h"
#include "base/memory/scoped_refptr.h"
#include "base/metrics/histogram_functions.h"
#include "base/metrics/histogram_macros.h"
#include "base/scoped_multi_source_observation.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/single_thread_task_runner.h"
#include "base/values.h"
#include "base/version.h"
#include "components/crx_file/id_util.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "components/safe_browsing/content/browser/safe_browsing_navigation_observer_manager.h"
#include "components/safe_browsing/core/browser/safe_browsing_metrics_collector.h"
#include "components/signin/public/identity_manager/identity_manager.h"
#include "components/supervised_user/core/common/pref_names.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/gpu_feature_checker.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/extension_dialog_auto_confirm.h"
#include "extensions/browser/extension_function_constants.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/extension_util.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/common/manifest_handlers/permissions_parser.h"
#include "extensions/common/permissions/permission_set.h"
#include "net/base/load_flags.h"
#include "ui/base/l10n/l10n_util.h"
#include "url/gurl.h"

using safe_browsing::SafeBrowsingNavigationObserverManager;

namespace extensions {

namespace BeginInstallWithManifest3 =
    api::webstore_private::BeginInstallWithManifest3;
namespace CompleteInstall = api::webstore_private::CompleteInstall;
namespace GetBrowserLogin = api::webstore_private::GetBrowserLogin;
namespace GetExtensionStatus = api::webstore_private::GetExtensionStatus;
namespace GetIsLauncherEnabled = api::webstore_private::GetIsLauncherEnabled;
namespace GetStoreLogin = api::webstore_private::GetStoreLogin;
namespace GetWebGLStatus = api::webstore_private::GetWebGLStatus;
namespace IsPendingCustodianApproval =
    api::webstore_private::IsPendingCustodianApproval;
namespace IsInIncognitoMode = api::webstore_private::IsInIncognitoMode;
namespace SetStoreLogin = api::webstore_private::SetStoreLogin;

WebstorePrivateBeginInstallWithManifest3Function::
    WebstorePrivateBeginInstallWithManifest3Function() = default;

WebstorePrivateBeginInstallWithManifest3Function::
    ~WebstorePrivateBeginInstallWithManifest3Function() = default;

ExtensionFunction::ResponseAction
WebstorePrivateBeginInstallWithManifest3Function::Run() {
  return RespondNow(Error("Not Implement"));
}

WebstorePrivateCompleteInstallFunction::
    WebstorePrivateCompleteInstallFunction() = default;

WebstorePrivateCompleteInstallFunction::
    ~WebstorePrivateCompleteInstallFunction() = default;

ExtensionFunction::ResponseAction
WebstorePrivateCompleteInstallFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

WebstorePrivateEnableAppLauncherFunction::
    WebstorePrivateEnableAppLauncherFunction() = default;

WebstorePrivateEnableAppLauncherFunction::
    ~WebstorePrivateEnableAppLauncherFunction() {}

ExtensionFunction::ResponseAction
WebstorePrivateEnableAppLauncherFunction::Run() {
  // TODO(crbug.com/822900): Check if this API is still in use and whether we
  // can remove it.
  return RespondNow(NoArguments());
}

WebstorePrivateGetBrowserLoginFunction::
    WebstorePrivateGetBrowserLoginFunction() = default;

WebstorePrivateGetBrowserLoginFunction::
    ~WebstorePrivateGetBrowserLoginFunction() {}

ExtensionFunction::ResponseAction
WebstorePrivateGetBrowserLoginFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

WebstorePrivateGetStoreLoginFunction::WebstorePrivateGetStoreLoginFunction() =
    default;

WebstorePrivateGetStoreLoginFunction::~WebstorePrivateGetStoreLoginFunction() {}

ExtensionFunction::ResponseAction WebstorePrivateGetStoreLoginFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

WebstorePrivateSetStoreLoginFunction::WebstorePrivateSetStoreLoginFunction() =
    default;

WebstorePrivateSetStoreLoginFunction::~WebstorePrivateSetStoreLoginFunction() {}

ExtensionFunction::ResponseAction WebstorePrivateSetStoreLoginFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

WebstorePrivateGetWebGLStatusFunction::WebstorePrivateGetWebGLStatusFunction()
    : feature_checker_(content::GpuFeatureChecker::Create(
          gpu::GPU_FEATURE_TYPE_ACCELERATED_WEBGL,
          base::BindOnce(&WebstorePrivateGetWebGLStatusFunction::OnFeatureCheck,
                         base::Unretained(this)))) {}

WebstorePrivateGetWebGLStatusFunction::
    ~WebstorePrivateGetWebGLStatusFunction() {}

ExtensionFunction::ResponseAction WebstorePrivateGetWebGLStatusFunction::Run() {
  feature_checker_->CheckGpuFeatureAvailability();
  return RespondLater();
}

void WebstorePrivateGetWebGLStatusFunction::OnFeatureCheck(
    bool feature_allowed) {
  Respond(ArgumentList(
      GetWebGLStatus::Results::Create(api::webstore_private::ParseWebGlStatus(
          feature_allowed ? "webgl_allowed" : "webgl_blocked"))));
}

WebstorePrivateGetIsLauncherEnabledFunction::
    WebstorePrivateGetIsLauncherEnabledFunction() {}

WebstorePrivateGetIsLauncherEnabledFunction::
    ~WebstorePrivateGetIsLauncherEnabledFunction() {}

ExtensionFunction::ResponseAction
WebstorePrivateGetIsLauncherEnabledFunction::Run() {
  return RespondNow(ArgumentList(
      GetIsLauncherEnabled::Results::Create(false)));
}

WebstorePrivateIsInIncognitoModeFunction::
    WebstorePrivateIsInIncognitoModeFunction() = default;

WebstorePrivateIsInIncognitoModeFunction::
    ~WebstorePrivateIsInIncognitoModeFunction() {}

ExtensionFunction::ResponseAction
WebstorePrivateIsInIncognitoModeFunction::Run() {
  return RespondNow(ArgumentList(IsInIncognitoMode::Results::Create(
      browser_context()->IsOffTheRecord())));
}

WebstorePrivateIsPendingCustodianApprovalFunction::
    WebstorePrivateIsPendingCustodianApprovalFunction() = default;

WebstorePrivateIsPendingCustodianApprovalFunction::
    ~WebstorePrivateIsPendingCustodianApprovalFunction() {}

ExtensionFunction::ResponseAction
WebstorePrivateIsPendingCustodianApprovalFunction::Run() {
  return RespondNow(BuildResponse(false));
}

ExtensionFunction::ResponseValue
WebstorePrivateIsPendingCustodianApprovalFunction::BuildResponse(bool result) {
  return WithArguments(result);
}

WebstorePrivateGetReferrerChainFunction::
    WebstorePrivateGetReferrerChainFunction() = default;

WebstorePrivateGetReferrerChainFunction::
    ~WebstorePrivateGetReferrerChainFunction() {}

ExtensionFunction::ResponseAction
WebstorePrivateGetReferrerChainFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

WebstorePrivateGetExtensionStatusFunction::
    WebstorePrivateGetExtensionStatusFunction() = default;
WebstorePrivateGetExtensionStatusFunction::
    ~WebstorePrivateGetExtensionStatusFunction() = default;

ExtensionFunction::ResponseAction
WebstorePrivateGetExtensionStatusFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
