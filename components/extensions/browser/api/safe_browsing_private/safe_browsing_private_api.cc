// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/safe_browsing_private/safe_browsing_private_api.h"

#include <utility>

#include "base/strings/stringprintf.h"
#include "components/extensions/common/api/safe_browsing_private.h"
#include "components/safe_browsing/content/browser/safe_browsing_navigation_observer_manager.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/extension_function.h"

using safe_browsing::SafeBrowsingNavigationObserverManager;

namespace extensions {

////////////////////////////////////////////////////////////////////////////////
// SafeBrowsingPrivateGetReferrerChainFunction

SafeBrowsingPrivateGetReferrerChainFunction::
    SafeBrowsingPrivateGetReferrerChainFunction() {}

SafeBrowsingPrivateGetReferrerChainFunction::
    ~SafeBrowsingPrivateGetReferrerChainFunction() {}

ExtensionFunction::ResponseAction
SafeBrowsingPrivateGetReferrerChainFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
