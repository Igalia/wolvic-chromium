// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/test_blocklist.h"

#include <set>

#include "base/containers/contains.h"
#include "base/functional/bind.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_runner.h"
#include "components/extensions/browser/blocklist.h"
#include "components/extensions/browser/blocklist_state_fetcher.h"
#include "components/extensions/browser/fake_safe_browsing_database_manager.h"
#include "components/extensions/test/test_extension_environment.h"

using extensions::BlocklistState;

namespace components_extensions {

using Type = TestExtensionEnvironment::Type;

namespace {

void Assign(BlocklistState* out, BlocklistState in) {
  *out = in;
}

}  // namespace

BlocklistStateFetcherMock::BlocklistStateFetcherMock() : request_count_(0) {}

BlocklistStateFetcherMock::~BlocklistStateFetcherMock() {}

void BlocklistStateFetcherMock::Request(const std::string& id,
                                        RequestCallback callback) {
  ++request_count_;

  BlocklistState result = extensions::NOT_BLOCKLISTED;
  if (base::Contains(states_, id))
    result = states_[id];

  base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(std::move(callback), result));
}

void BlocklistStateFetcherMock::SetState(const std::string& id,
                                         BlocklistState state) {
  states_[id] = state;
}

void BlocklistStateFetcherMock::Clear() {
  states_.clear();
}

TestBlocklist::TestBlocklist()
    : env_(std::make_unique<TestExtensionEnvironment>(Type::kInheritExistingTaskEnvironment)),
      blocklist_(nullptr),
      blocklist_db_(new FakeSafeBrowsingDatabaseManager(true)),
      scoped_blocklist_db_(blocklist_db_) {}

TestBlocklist::TestBlocklist(Blocklist* blocklist)
    : env_(std::make_unique<TestExtensionEnvironment>(Type::kInheritExistingTaskEnvironment)),
      blocklist_(nullptr),
      blocklist_db_(new FakeSafeBrowsingDatabaseManager(true)),
      scoped_blocklist_db_(blocklist_db_) {
  Attach(blocklist);
}

TestBlocklist::~TestBlocklist() {
  Detach();
}

void TestBlocklist::Attach(Blocklist* blocklist) {
  if (blocklist_)
    Detach();

  blocklist_ = blocklist;
  blocklist_->SetBlocklistStateFetcherForTest(&state_fetcher_mock_);
}

void TestBlocklist::Detach() {
  blocklist_->ResetBlocklistStateFetcherForTest();
  blocklist_->ResetDatabaseUpdatedListenerForTest();
}

void TestBlocklist::SetBlocklistState(const std::string& extension_id,
                                      BlocklistState state,
                                      bool notify) {
  state_fetcher_mock_.SetState(extension_id, state);

  switch (state) {
    case extensions::NOT_BLOCKLISTED:
      blocklist_db_->RemoveUnsafe(extension_id);
      break;

    case extensions::BLOCKLISTED_MALWARE:
    case extensions::BLOCKLISTED_SECURITY_VULNERABILITY:
    case extensions::BLOCKLISTED_CWS_POLICY_VIOLATION:
    case extensions::BLOCKLISTED_POTENTIALLY_UNWANTED:
      blocklist_db_->AddUnsafe(extension_id);
      break;

    default:
      break;
  }

  if (notify)
    blocklist_db_->NotifyUpdate();
}

void TestBlocklist::Clear(bool notify) {
  state_fetcher_mock_.Clear();
  blocklist_db_->ClearUnsafe();
  if (notify)
    blocklist_db_->NotifyUpdate();
}

BlocklistState TestBlocklist::GetBlocklistState(
    const std::string& extension_id) {
  BlocklistState blocklist_state;
  blocklist_->IsBlocklisted(extension_id,
                            base::BindOnce(&Assign, &blocklist_state));
  base::RunLoop().RunUntilIdle();
  return blocklist_state;
}

void TestBlocklist::DisableSafeBrowsing() {
  blocklist_db_->Disable();
}

void TestBlocklist::EnableSafeBrowsing() {
  blocklist_db_->Enable();
}

void TestBlocklist::NotifyUpdate() {
  blocklist_db_->NotifyUpdate();
}

}  // namespace components_extensions
