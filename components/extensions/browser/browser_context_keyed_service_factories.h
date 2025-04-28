// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_BROWSER_CONTEXT_KEYED_SERVICE_FACTORIES_H_
#define COMPONENTS_EXTENSIONS_BROWSER_BROWSER_CONTEXT_KEYED_SERVICE_FACTORIES_H_

#include "base/component_export.h"

namespace components_extensions {

// Ensures the existence of any BrowserContextKeyedServiceFactory provided by
// the Chrome extensions code or its corresponding APIs.
COMPONENT_EXPORT(EXTENSIONS)
void EnsureBrowserContextKeyedServiceFactoriesBuilt();

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_BROWSER_CONTEXT_KEYED_SERVICE_FACTORIES_H_
