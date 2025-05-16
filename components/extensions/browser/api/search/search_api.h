// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_SEARCH_SEARCH_API_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_SEARCH_SEARCH_API_H_

#include "extensions/browser/extension_function.h"

namespace extensions {

class SearchQueryFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("search.query", SEARCH_QUERY)
  SearchQueryFunction() = default;

 private:
  ~SearchQueryFunction() override = default;
  ResponseAction Run() override;
};

}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_SEARCH_SEARCH_API_H_
