// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_COMMON_CHROME_MANIFEST_HANDLERS_H_
#define COMPONENTS_EXTENSIONS_COMMON_CHROME_MANIFEST_HANDLERS_H_

namespace components_extensions {

// Registers all manifest handlers used in Chrome. Should be called
// once in each process. See also extensions/common/common_manifest_handlers.h.
void RegisterChromeManifestHandlers();

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_COMMON_CHROME_MANIFEST_HANDLERS_H_
