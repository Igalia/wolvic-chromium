// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_WEBSTORE_PRIVATE_WEBSTORE_PRIVATE_API_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_WEBSTORE_PRIVATE_WEBSTORE_PRIVATE_API_H_

#include <memory>
#include <optional>
#include <string>

#include "base/auto_reset.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "components/extensions/common/api/webstore_private.h"
#include "components/supervised_user/core/common/buildflags.h"
#include "extensions/browser/extension_function.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace content {
class GpuFeatureChecker;
class WebContents;
}

namespace extensions {

class Extension;
class ScopedActiveInstall;

class WebstorePrivateBeginInstallWithManifest3Function
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webstorePrivate.beginInstallWithManifest3",
                             WEBSTOREPRIVATE_BEGININSTALLWITHMANIFEST3)

  WebstorePrivateBeginInstallWithManifest3Function();

 private:
  using Params = api::webstore_private::BeginInstallWithManifest3::Params;

  ~WebstorePrivateBeginInstallWithManifest3Function() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;
};

class WebstorePrivateCompleteInstallFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webstorePrivate.completeInstall",
                             WEBSTOREPRIVATE_COMPLETEINSTALL)

  WebstorePrivateCompleteInstallFunction();

 private:
  ~WebstorePrivateCompleteInstallFunction() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;
};

class WebstorePrivateEnableAppLauncherFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webstorePrivate.enableAppLauncher",
                             WEBSTOREPRIVATE_ENABLEAPPLAUNCHER)

  WebstorePrivateEnableAppLauncherFunction();

 private:
  ~WebstorePrivateEnableAppLauncherFunction() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;
};

class WebstorePrivateGetBrowserLoginFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webstorePrivate.getBrowserLogin",
                             WEBSTOREPRIVATE_GETBROWSERLOGIN)

  WebstorePrivateGetBrowserLoginFunction();

 private:
  ~WebstorePrivateGetBrowserLoginFunction() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;
};

class WebstorePrivateGetStoreLoginFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webstorePrivate.getStoreLogin",
                             WEBSTOREPRIVATE_GETSTORELOGIN)

  WebstorePrivateGetStoreLoginFunction();

 private:
  ~WebstorePrivateGetStoreLoginFunction() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;
};

class WebstorePrivateSetStoreLoginFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webstorePrivate.setStoreLogin",
                             WEBSTOREPRIVATE_SETSTORELOGIN)

  WebstorePrivateSetStoreLoginFunction();

 private:
  ~WebstorePrivateSetStoreLoginFunction() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;
};

class WebstorePrivateGetWebGLStatusFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webstorePrivate.getWebGLStatus",
                             WEBSTOREPRIVATE_GETWEBGLSTATUS)

  WebstorePrivateGetWebGLStatusFunction();

 private:
  ~WebstorePrivateGetWebGLStatusFunction() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;

  void OnFeatureCheck(bool feature_allowed);

  scoped_refptr<content::GpuFeatureChecker> feature_checker_;
};

class WebstorePrivateGetIsLauncherEnabledFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webstorePrivate.getIsLauncherEnabled",
                             WEBSTOREPRIVATE_GETISLAUNCHERENABLED)

  WebstorePrivateGetIsLauncherEnabledFunction();

 private:
  ~WebstorePrivateGetIsLauncherEnabledFunction() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;

  void OnIsLauncherCheckCompleted(bool is_enabled);
};

class WebstorePrivateIsInIncognitoModeFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webstorePrivate.isInIncognitoMode",
                             WEBSTOREPRIVATE_ISININCOGNITOMODEFUNCTION)

  WebstorePrivateIsInIncognitoModeFunction();

 private:
  ~WebstorePrivateIsInIncognitoModeFunction() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;
};

class WebstorePrivateIsPendingCustodianApprovalFunction
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webstorePrivate.isPendingCustodianApproval",
                             WEBSTOREPRIVATE_ISPENDINGCUSTODIANAPPROVAL)

  WebstorePrivateIsPendingCustodianApprovalFunction();

 private:
  ~WebstorePrivateIsPendingCustodianApprovalFunction() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;

  ExtensionFunction::ResponseValue BuildResponse(bool result);
};

class WebstorePrivateGetReferrerChainFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webstorePrivate.getReferrerChain",
                             WEBSTOREPRIVATE_GETREFERRERCHAIN)

  WebstorePrivateGetReferrerChainFunction();

  WebstorePrivateGetReferrerChainFunction(
      const WebstorePrivateGetReferrerChainFunction&) = delete;
  WebstorePrivateGetReferrerChainFunction& operator=(
      const WebstorePrivateGetReferrerChainFunction&) = delete;

 private:
  ~WebstorePrivateGetReferrerChainFunction() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;
};

class WebstorePrivateGetExtensionStatusFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webstorePrivate.getExtensionStatus",
                             WEBSTOREPRIVATE_GETEXTENSIONSTATUS)

  WebstorePrivateGetExtensionStatusFunction();

  WebstorePrivateGetExtensionStatusFunction(
      const WebstorePrivateGetExtensionStatusFunction&) = delete;
  WebstorePrivateGetExtensionStatusFunction& operator=(
      const WebstorePrivateGetExtensionStatusFunction&) = delete;

 private:
  ~WebstorePrivateGetExtensionStatusFunction() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;
};

}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_WEBSTORE_PRIVATE_WEBSTORE_PRIVATE_API_H_
