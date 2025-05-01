// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_COMMON_INITIALIZE_EXTENSIONS_CLIENT_H_
#define COMPONENTS_EXTENSIONS_COMMON_INITIALIZE_EXTENSIONS_CLIENT_H_

#include "components/extensions/common/buildflags.h"

#if !BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
#error "Extensions in components must be enabled"
#endif

namespace components_extensions {
void EnsureExtensionsClientInitialized();
}

#endif  // COMPONENTS_EXTENSIONS_COMMON_INITIALIZE_EXTENSIONS_CLIENT_H_
