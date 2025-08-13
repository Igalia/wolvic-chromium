// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/event_router_forwarder.h"

#include <utility>

#include "base/functional/bind.h"
#include "base/memory/raw_ptr.h"
#include "base/run_loop.h"
#include "base/test/thread_test_helper.h"
#include "build/build_config.h"
#include "components/extensions/test/test_extension_environment.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/test/browser_task_environment.h"
#include "content/public/test/test_browser_context.h"
#include "extensions/common/extension_id.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace components_extensions {

namespace events = extensions::events;

using extensions::ExtensionId;

namespace {

const events::HistogramValue kHistogramValue = events::FOR_TEST;
const char kEventName[] = "event_name";

class MockEventRouterForwarder : public EventRouterForwarder {
 public:
  MOCK_METHOD6(CallEventRouter,
               void(content::BrowserContext*,
                    const std::string&,
                    events::HistogramValue,
                    const std::string&,
                    content::BrowserContext*,
                    const GURL&));

  void CallEventRouter(content::BrowserContext* profile,
                       const ExtensionId& extension_id,
                       events::HistogramValue histogram_value,
                       const std::string& event_name,
                       base::Value::List args,
                       content::BrowserContext* restrict_to_profile,
                       const GURL& event_url) override {
    CallEventRouter(profile, extension_id, histogram_value, event_name,
                    restrict_to_profile, event_url);
  }

 protected:
  ~MockEventRouterForwarder() override {}
};

static void BroadcastEventToRenderers(
    EventRouterForwarder* event_router,
    events::HistogramValue histogram_value,
    const std::string& event_name,
    const GURL& url,
    bool dispatch_to_off_the_record_profiles) {
  event_router->BroadcastEventToRenderers(histogram_value, event_name,
                                          base::Value::List(), url,
                                          dispatch_to_off_the_record_profiles);
}

static void DispatchEventToRenderers(EventRouterForwarder* event_router,
                                     events::HistogramValue histogram_value,
                                     const std::string& event_name,
                                     void* profile,
                                     bool use_profile_to_restrict_events,
                                     const GURL& url,
                                     bool dispatch_to_off_the_record_profiles) {
  event_router->DispatchEventToRenderers(
      histogram_value, event_name, base::Value::List(), static_cast<content::BrowserContext*>(profile),
      use_profile_to_restrict_events, url, dispatch_to_off_the_record_profiles);
}

}  // namespace

class EventRouterForwarderTest : public testing::Test {
 protected:
  EventRouterForwarderTest()
      : task_environment_(content::BrowserTaskEnvironment::REAL_IO_THREAD)
      , env_(std::make_unique<TestExtensionEnvironment>(nullptr, nullptr)) {
  }

  void SetUp() override {
    browser_context1_ = std::make_unique<content::TestBrowserContext>();
    browser_context2_ = std::make_unique<content::TestBrowserContext>();
    env_->AddBrowserContext(browser_context1_.get());
    env_->AddBrowserContext(browser_context2_.get());
  }

  void TearDown() override {
    env_->DeleteBrowserContext();
    env_.reset();
  }

  content::BrowserTaskEnvironment task_environment_;
  std::unique_ptr<TestExtensionEnvironment> env_;
  std::unique_ptr<content::TestBrowserContext> browser_context1_;
  std::unique_ptr<content::TestBrowserContext> browser_context2_;
};

TEST_F(EventRouterForwarderTest, BroadcastRendererUI) {
  scoped_refptr<MockEventRouterForwarder> event_router(
      new MockEventRouterForwarder);
  GURL url;
  EXPECT_CALL(*event_router,
              CallEventRouter(browser_context1_.get(), "", kHistogramValue, kEventName,
                              browser_context1_.get(), url));
  EXPECT_CALL(*event_router,
              CallEventRouter(browser_context2_.get(), "", kHistogramValue, kEventName,
                              browser_context2_.get(), url));
  BroadcastEventToRenderers(event_router.get(), kHistogramValue, kEventName,
                            url, false);
}

// TODO(mshin): Support Profile for Embedder
// TEST_F(EventRouterForwarderTest, BroadcastRendererUIIncognito) {
//   scoped_refptr<MockEventRouterForwarder> event_router(
//       new MockEventRouterForwarder);
//   using ::testing::_;
//   GURL url;
//   context::BrowserContext* incognito =
//       browser_context1_->GetPrimaryOTRProfile(/*create_if_needed=*/true);
//   EXPECT_CALL(*event_router,
//               CallEventRouter(browser_context1_.get(), "", kHistogramValue, kEventName,
//                               browser_context1_.get(), url));
//   EXPECT_CALL(*event_router, CallEventRouter(incognito, _, _, _, _, _))
//       .Times(0);
//   EXPECT_CALL(*event_router,
//               CallEventRouter(browser_context2_.get(), "", kHistogramValue, kEventName,
//                               browser_context2_.get(), url));
//   BroadcastEventToRenderers(event_router.get(), kHistogramValue, kEventName,
//                             url, false);
// }

// TEST_F(EventRouterForwarderTest,
//        BroadcastRendererUIIncognitoWithDispatchToOffTheRecordProfiles) {
//   scoped_refptr<MockEventRouterForwarder> event_router(
//       new MockEventRouterForwarder);
//   using ::testing::_;
//   GURL url;
//   Profile* incognito1 =
//       browser_context1_->GetPrimaryOTRProfile(/*create_if_needed=*/true);
//   Profile* incognito2 =
//       browser_context2_->GetPrimaryOTRProfile(/*create_if_needed=*/true);
//   EXPECT_CALL(*event_router,
//               CallEventRouter(browser_context1_.get(), "", kHistogramValue, kEventName,
//                               browser_context1_.get(), url));
//   EXPECT_CALL(*event_router, CallEventRouter(incognito1, _, _, _, _, _));
//   EXPECT_CALL(*event_router,
//               CallEventRouter(browser_context2_.get(), "", kHistogramValue, kEventName,
//                               browser_context2_.get(), url));
//   EXPECT_CALL(*event_router, CallEventRouter(incognito2, _, _, _, _, _));
//   BroadcastEventToRenderers(event_router.get(), kHistogramValue, kEventName,
//                             url, true);
// }

// This is the canonical test for passing control flow from the IO thread
// to the UI thread. Repeating this for all public functions of
// EventRouterForwarder would not increase coverage.
TEST_F(EventRouterForwarderTest, BroadcastRendererIO) {
  scoped_refptr<MockEventRouterForwarder> event_router(
      new MockEventRouterForwarder);
  GURL url;
  EXPECT_CALL(*event_router,
              CallEventRouter(browser_context1_.get(), "", kHistogramValue, kEventName,
                              browser_context1_.get(), url));
  EXPECT_CALL(*event_router,
              CallEventRouter(browser_context2_.get(), "", kHistogramValue, kEventName,
                              browser_context2_.get(), url));
  content::GetIOThreadTaskRunner({})->PostTask(
      FROM_HERE, base::BindOnce(&BroadcastEventToRenderers,
                                base::Unretained(event_router.get()),
                                kHistogramValue, kEventName, url, false));

  // Wait for IO thread's message loop to be processed
  scoped_refptr<base::ThreadTestHelper> helper(
      new base::ThreadTestHelper(content::GetIOThreadTaskRunner({}).get()));
  ASSERT_TRUE(helper->Run());

  base::RunLoop().RunUntilIdle();
}

TEST_F(EventRouterForwarderTest, UnicastRendererUIRestricted) {
  scoped_refptr<MockEventRouterForwarder> event_router(
      new MockEventRouterForwarder);
  using ::testing::_;
  GURL url;
  EXPECT_CALL(*event_router,
              CallEventRouter(browser_context1_.get(), "", kHistogramValue, kEventName,
                              browser_context1_.get(), url));
  EXPECT_CALL(*event_router, CallEventRouter(browser_context2_.get(), _, _, _, _, _))
      .Times(0);
  DispatchEventToRenderers(event_router.get(), kHistogramValue, kEventName,
                           browser_context1_.get(), true, url, false);
}

// TODO(mshin): Support Profile for Embedder
// TEST_F(EventRouterForwarderTest, UnicastRendererUIRestrictedIncognito1) {
//   scoped_refptr<MockEventRouterForwarder> event_router(
//       new MockEventRouterForwarder);
//   Profile* incognito =
//       browser_context1_->GetPrimaryOTRProfile(/*create_if_needed=*/true);
//   using ::testing::_;
//   GURL url;
//   EXPECT_CALL(*event_router,
//               CallEventRouter(browser_context1_.get(), "", kHistogramValue, kEventName,
//                               browser_context1_.get(), url));
//   EXPECT_CALL(*event_router, CallEventRouter(incognito, _, _, _, _, _))
//       .Times(0);
//   EXPECT_CALL(*event_router, CallEventRouter(browser_context2_.get(), _, _, _, _, _))
//       .Times(0);
//   DispatchEventToRenderers(event_router.get(), kHistogramValue, kEventName,
//                            browser_context1_.get(), true, url, false);
// }

// TEST_F(
//     EventRouterForwarderTest,
//     UnicastRendererUIRestrictedIncognito1WithDispatchToOffTheRecordProfiles) {
//   scoped_refptr<MockEventRouterForwarder> event_router(
//       new MockEventRouterForwarder);
//   Profile* incognito1 =
//       browser_context1_->GetPrimaryOTRProfile(/*create_if_needed=*/true);
//   Profile* incognito2 =
//       browser_context2_->GetPrimaryOTRProfile(/*create_if_needed=*/true);
//   using ::testing::_;
//   GURL url;
//   EXPECT_CALL(*event_router,
//               CallEventRouter(browser_context1_.get(), "", kHistogramValue, kEventName,
//                               browser_context1_.get(), url));
//   EXPECT_CALL(*event_router, CallEventRouter(incognito1, _, _, _, _, _));
//   EXPECT_CALL(*event_router, CallEventRouter(browser_context2_.get(), _, _, _, _, _))
//       .Times(0);
//   EXPECT_CALL(*event_router, CallEventRouter(incognito2, _, _, _, _, _))
//       .Times(0);
//   DispatchEventToRenderers(event_router.get(), kHistogramValue, kEventName,
//                            browser_context1_.get(), true, url, true);
// }

// TEST_F(EventRouterForwarderTest, UnicastRendererUIRestrictedIncognito2) {
//   scoped_refptr<MockEventRouterForwarder> event_router(
//       new MockEventRouterForwarder);
//   Profile* incognito =
//       browser_context1_->GetPrimaryOTRProfile(/*create_if_needed=*/true);
//   using ::testing::_;
//   GURL url;
//   EXPECT_CALL(*event_router, CallEventRouter(browser_context1_.get(), _, _, _, _, _))
//       .Times(0);
//   EXPECT_CALL(*event_router, CallEventRouter(incognito, "", kHistogramValue,
//                                              kEventName, incognito, url));
//   EXPECT_CALL(*event_router, CallEventRouter(browser_context2_.get(), _, _, _, _, _))
//       .Times(0);
//   DispatchEventToRenderers(event_router.get(), kHistogramValue, kEventName,
//                            incognito, true, url, false);
// }

TEST_F(EventRouterForwarderTest, UnicastRendererUIUnrestricted) {
  scoped_refptr<MockEventRouterForwarder> event_router(
      new MockEventRouterForwarder);
  using ::testing::_;
  GURL url;
  EXPECT_CALL(*event_router,
              CallEventRouter(browser_context1_.get(), "", kHistogramValue, kEventName,
                              nullptr, url));
  EXPECT_CALL(*event_router, CallEventRouter(browser_context2_.get(), _, _, _, _, _))
      .Times(0);
  DispatchEventToRenderers(event_router.get(), kHistogramValue, kEventName,
                           browser_context1_.get(), false, url, false);
}

// TODO(mshin): Support Profile for Embedder
// TEST_F(EventRouterForwarderTest, UnicastRendererUIUnrestrictedIncognito) {
//   scoped_refptr<MockEventRouterForwarder> event_router(
//       new MockEventRouterForwarder);
//   Profile* incognito =
//       browser_context1_->GetPrimaryOTRProfile(/*create_if_needed=*/true);
//   using ::testing::_;
//   GURL url;
//   EXPECT_CALL(*event_router,
//               CallEventRouter(browser_context1_.get(), "", kHistogramValue, kEventName,
//                               nullptr, url));
//   EXPECT_CALL(*event_router, CallEventRouter(incognito, _, _, _, _, _))
//       .Times(0);
//   EXPECT_CALL(*event_router, CallEventRouter(browser_context2_.get(), _, _, _, _, _))
//       .Times(0);
//   DispatchEventToRenderers(event_router.get(), kHistogramValue, kEventName,
//                            browser_context1_.get(), false, url, false);
// }

}  // namespace components_extensions
