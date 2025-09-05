// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_SERVICE_USER_TEST_BASE_H_
#define COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_SERVICE_USER_TEST_BASE_H_

#include "components/extensions/browser/extension_service_test_base.h"

namespace components_extensions {

// Test class used to setup test users in the unit test for browser/lacros and
// ChromeOS Ash.
class ExtensionServiceUserTestBase : public ExtensionServiceTestBase {
 public:
  ExtensionServiceUserTestBase();
  ~ExtensionServiceUserTestBase() override;

  // If browser/lacros: set the testing profile for the test as a guest if
  // `is_guest` is `true`. If ChromeOS Ash: do the above, but also login a
  // `user_manager::User` and set it to be a guest account if `is_guest` is
  // `true`.
  void MaybeSetUpTestUser(bool is_guest);

 protected:
  // Alternatively, a subclass may pass a BrowserTaskEnvironment directly.
  explicit ExtensionServiceUserTestBase(
      std::unique_ptr<content::BrowserTaskEnvironment> task_environment);
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_SERVICE_USER_TEST_BASE_H_
