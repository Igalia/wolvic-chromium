// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_API_BROWSER_CONTEXT_KEYED_SERVICE_FACTORIES_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_API_BROWSER_CONTEXT_KEYED_SERVICE_FACTORIES_H_

namespace components_extensions {

// Ensures the existence of any BrowserContextKeyedServiceFactory provided by
// the Chrome extensions APIs.
void EnsureApiBrowserContextKeyedServiceFactoriesBuilt();

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_API_BROWSER_CONTEXT_KEYED_SERVICE_FACTORIES_H_
