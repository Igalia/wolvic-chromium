// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_DOWNLOADS_INTERNAL_DOWNLOADS_INTERNAL_API_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_DOWNLOADS_INTERNAL_DOWNLOADS_INTERNAL_API_H_

#include "extensions/browser/extension_function.h"

namespace extensions {

class DownloadsInternalDetermineFilenameFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("downloadsInternal.determineFilename",
                             DOWNLOADSINTERNAL_DETERMINEFILENAME)
  DownloadsInternalDetermineFilenameFunction();

  DownloadsInternalDetermineFilenameFunction(
      const DownloadsInternalDetermineFilenameFunction&) = delete;
  DownloadsInternalDetermineFilenameFunction& operator=(
      const DownloadsInternalDetermineFilenameFunction&) = delete;

  ResponseAction Run() override;

 protected:
  ~DownloadsInternalDetermineFilenameFunction() override;
};

}  // namespace extensions
#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_DOWNLOADS_INTERNAL_DOWNLOADS_INTERNAL_API_H_
