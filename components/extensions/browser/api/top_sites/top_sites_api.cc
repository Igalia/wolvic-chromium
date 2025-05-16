// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/top_sites/top_sites_api.h"

#include <stddef.h>

#include <memory>
#include <utility>

#include "base/functional/bind.h"
#include "base/values.h"
#include "components/history/core/browser/top_sites.h"

namespace extensions {

TopSitesGetFunction::TopSitesGetFunction() = default;
TopSitesGetFunction::~TopSitesGetFunction() = default;

ExtensionFunction::ResponseAction TopSitesGetFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
