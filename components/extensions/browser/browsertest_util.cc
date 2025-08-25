// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/browsertest_util.h"

#include <memory>

#include "base/feature_list.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/test/bind.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/test_utils.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/extension.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace components_extensions::browsertest_util {

// TODO(mshin): Enable the below code after migrating Tab API
// content::WebContents* AddTab(Browser* browser, const GURL& url) {
//   int starting_tab_count = browser->tab_strip_model()->count();
//   ui_test_utils::NavigateToURLWithDisposition(
//       browser, url, WindowOpenDisposition::NEW_FOREGROUND_TAB,
//       ui_test_utils::BROWSER_TEST_WAIT_FOR_LOAD_STOP);
//   int tab_count = browser->tab_strip_model()->count();
//   EXPECT_EQ(starting_tab_count + 1, tab_count);
//   return browser->tab_strip_model()->GetActiveWebContents();
// }

bool DidChangeTitle(content::WebContents& web_contents,
                    const std::u16string& original_title,
                    const std::u16string& changed_title) {
  const std::u16string& title = web_contents.GetTitle();
  if (title == changed_title) {
    return true;
  }
  if (title == original_title) {
    return false;
  }
  ADD_FAILURE() << "Unexpected page title found:  " << title;
  return false;
}

BlockedActionWaiter::BlockedActionWaiter(ExtensionActionRunner* runner)
    : runner_(runner) {
  runner_->set_observer_for_testing(this);  // IN-TEST
}

BlockedActionWaiter::~BlockedActionWaiter() {
  runner_->set_observer_for_testing(nullptr);  // IN-TEST
}

void BlockedActionWaiter::Wait() {
  run_loop_.Run();
}

void BlockedActionWaiter::OnBlockedActionAdded() {
  run_loop_.Quit();
}

}  // namespace components_extensions::browsertest_util
