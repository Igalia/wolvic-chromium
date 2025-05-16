// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/search/search_api.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "components/extensions/common/api/search.h"
#include "components/search_engines/util.h"

namespace extensions {

using extensions::api::search::Disposition;

ExtensionFunction::ResponseAction SearchQueryFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
