// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/blocklist_check.h"

#include "base/task/single_thread_task_runner.h"
#include "components/extensions/browser/blocklist.h"
#include "components/extensions/browser/test_blocklist.h"
#include "components/extensions/browser/test_extension_prefs.h"
#include "content/public/test/browser_task_environment.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/preload_check.h"
#include "extensions/browser/preload_check_test_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using extensions::BlocklistState;
using extensions::Extension;
using extensions::PreloadCheck;
using extensions::PreloadCheckRunner;

namespace components_extensions {
namespace {

class BlocklistCheckTest : public testing::Test {
 public:
  BlocklistCheckTest()
      : test_prefs_(base::SingleThreadTaskRunner::GetCurrentDefault()) {}

 protected:
  void SetUp() override {
    test_blocklist_.Attach(&blocklist_);
    extension_ = test_prefs_.AddExtension("foo");
  }

  void SetBlocklistState(BlocklistState state) {
    test_blocklist_.SetBlocklistState(extension_->id(), state, /*notify=*/true);
  }

  Blocklist* blocklist() { return &blocklist_; }
  scoped_refptr<Extension> extension_;
  PreloadCheckRunner runner_;

 private:
  content::BrowserTaskEnvironment task_environment_;
  TestExtensionPrefs test_prefs_;
  Blocklist blocklist_;
  TestBlocklist test_blocklist_;
};

}  // namespace

// Tests that the blocklist check identifies a blocklisted extension.
TEST_F(BlocklistCheckTest, BlocklistedMalware) {
  SetBlocklistState(extensions::BLOCKLISTED_MALWARE);

  BlocklistCheck check(blocklist(), extension_);
  runner_.RunUntilComplete(&check);

  EXPECT_THAT(runner_.errors(), testing::UnorderedElementsAre(
                                    PreloadCheck::Error::kBlocklistedId));
  EXPECT_TRUE(check.GetErrorMessage().empty());
}

// Tests that the blocklist check ignores a non-blocklisted extension.
TEST_F(BlocklistCheckTest, Pass) {
  SetBlocklistState(extensions::NOT_BLOCKLISTED);

  BlocklistCheck check(blocklist(), extension_);
  runner_.RunUntilComplete(&check);

  EXPECT_EQ(0u, runner_.errors().size());
  EXPECT_TRUE(check.GetErrorMessage().empty());
}

// Tests that destroying the check after starting it does not cause errors.
TEST_F(BlocklistCheckTest, ResetCheck) {
  SetBlocklistState(extensions::BLOCKLISTED_MALWARE);

  {
    BlocklistCheck check(blocklist(), extension_);
    runner_.Run(&check);
  }

  runner_.WaitForIdle();
  EXPECT_FALSE(runner_.called());
}

}  // namespace components_extensions
