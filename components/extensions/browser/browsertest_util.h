// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_BROWSER_EXTENSIONS_BROWSERTEST_UTIL_H_
#define COMPONENTS_BROWSER_EXTENSIONS_BROWSERTEST_UTIL_H_

#include <string>
#include "base/run_loop.h"
#include "components/extensions/browser/extension_action_runner.h"

class Browser;
class GURL;
class Profile;

namespace base {
class RunLoop;
}

namespace content {
class WebContents;
}

namespace extensions {
class Extension;
}

namespace components_extensions {


namespace browsertest_util {

// Adds a tab to |browser| and returns the newly added WebContents.
// TODO(mshin): Enable the below code after migrating Tab API
// content::WebContents* AddTab(Browser* browser, const GURL& url);

// Returns whether the given `web_contents` has the associated
// `changed_title`. If the web contents has neither `changed_title`
// nor `original_title `, adds a failure to the test (for an unexpected
// title).
bool DidChangeTitle(content::WebContents& web_contents,
                    const std::u16string& original_title,
                    const std::u16string& changed_title);

// Can be used to wait for blocked actions (pending scripts, web requests, etc.)
// to be noticed in tests. Blocked actions recording initiates in the renderer
// so this helps when waiting from the browser side. This should be used on the
// stack for proper destruction.
class BlockedActionWaiter : public ExtensionActionRunner::TestObserver {
 public:
  // `runner` must outlive this object.
  explicit BlockedActionWaiter(ExtensionActionRunner* runner);
  BlockedActionWaiter(const BlockedActionWaiter&) = delete;
  BlockedActionWaiter& operator=(const BlockedActionWaiter&) = delete;
  ~BlockedActionWaiter();

  // Wait for the blocked action until the observer is called with the blocked
  // action being added.
  void Wait();

 private:
  // ExtensionActionRunner::TestObserver:
  void OnBlockedActionAdded() override;

  const raw_ptr<ExtensionActionRunner> runner_;
  base::RunLoop run_loop_;
};

}  // namespace browsertest_util
}  // namespace components_extensions

#endif  // COMPONENTS_BROWSER_EXTENSIONS_BROWSERTEST_UTIL_H_
