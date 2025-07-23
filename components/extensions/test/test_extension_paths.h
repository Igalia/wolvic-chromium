// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_COMMON_EXTENSION_PATHS_H_
#define COMPONENTS_EXTENSIONS_COMMON_EXTENSION_PATHS_H_

// This file declares path keys for extensions.  These can be used with
// the PathService to access various special directories and files.

namespace components_extensions {

enum {
  PATH_START = 20000,

  // Valid only in development environment
  DIR_TEST_DATA,

  PATH_END
};

// Call once to register the provider for the path keys defined above.
void RegisterPathProvider();

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_COMMON_EXTENSION_PATHS_H_
