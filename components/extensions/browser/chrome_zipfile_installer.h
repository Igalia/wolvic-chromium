// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_CHROME_ZIPFILE_INSTALLER_H_
#define COMPONENTS_EXTENSIONS_BROWSER_CHROME_ZIPFILE_INSTALLER_H_

#include "extensions/browser/zipfile_installer.h"

namespace components_extensions {
class ExtensionService;

// Creates a ZipFileInstaller::DoneCallback that when passed to
// ZipFileInstaller::Create() causes the unzipped extension to be loaded with
// extensions::UnpackedInstaller on success.
extensions::ZipFileInstaller::DoneCallback MakeRegisterInExtensionServiceCallback(
    ExtensionService* service);

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_CHROME_ZIPFILE_INSTALLER_H_
