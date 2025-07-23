// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <utility>

#include "components/extensions/browser/extension_service_test_base.h"
#include "components/extensions/browser/extension_service_user_test_base.h"
#include "content/public/test/browser_task_environment.h"

namespace components_extensions {

ExtensionServiceUserTestBase::ExtensionServiceUserTestBase() = default;
ExtensionServiceUserTestBase::~ExtensionServiceUserTestBase() = default;
ExtensionServiceUserTestBase::ExtensionServiceUserTestBase(
    std::unique_ptr<content::BrowserTaskEnvironment> task_environment)
    : ExtensionServiceTestBase(std::move(task_environment)) {}


void ExtensionServiceUserTestBase::MaybeSetUpTestUser(bool is_guest) {
  // TODO(mshin): Support the mutiple profile
  // testing_profile()->SetGuestSession(is_guest);

  // ASSERT_EQ(is_guest, testing_profile()->IsGuestSession());
}

}  // namespace components_extensions
