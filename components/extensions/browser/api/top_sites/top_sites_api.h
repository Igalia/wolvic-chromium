// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_TOP_SITES_TOP_SITES_API_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_TOP_SITES_TOP_SITES_API_H_

#include "components/history/core/browser/history_types.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

class TopSitesGetFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("topSites.get", TOPSITES_GET)

  TopSitesGetFunction();

 protected:
  ~TopSitesGetFunction() override;

  // ExtensionFunction:
  ResponseAction Run() override;

 private:
  void OnMostVisitedURLsAvailable(const history::MostVisitedURLList& data);
};

}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_TOP_SITES_TOP_SITES_API_H_
