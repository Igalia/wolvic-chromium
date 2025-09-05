// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/blocklist_check.h"

#include "base/functional/bind.h"
#include "components/extensions/browser/blocklist.h"
#include "extensions/common/extension.h"

namespace components_extensions {

using extensions::Extension;
using extensions::BlocklistState;

BlocklistCheck::BlocklistCheck(Blocklist* blocklist,
                               scoped_refptr<const Extension> extension)
    : PreloadCheck(extension), blocklist_(blocklist) {}

BlocklistCheck::~BlocklistCheck() {}

void BlocklistCheck::Start(ResultCallback callback) {
  callback_ = std::move(callback);

  blocklist_->IsBlocklisted(
      extension()->id(),
      base::BindOnce(&BlocklistCheck::OnBlocklistedStateRetrieved,
                     weak_ptr_factory_.GetWeakPtr()));
}

void BlocklistCheck::OnBlocklistedStateRetrieved(
    BlocklistState blocklist_state) {
  Errors errors;
  if (blocklist_state == BlocklistState::BLOCKLISTED_MALWARE)
    errors.insert(PreloadCheck::Error::kBlocklistedId);
  else if (blocklist_state == BlocklistState::BLOCKLISTED_UNKNOWN)
    errors.insert(PreloadCheck::Error::kBlocklistedUnknown);
  std::move(callback_).Run(errors);
}

}  // namespace components_extensions
