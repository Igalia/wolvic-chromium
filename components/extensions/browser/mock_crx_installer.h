// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_MOCK_CRX_INSTALLER_H_
#define COMPONENTS_EXTENSIONS_BROWSER_MOCK_CRX_INSTALLER_H_

#include "components/extensions/browser/crx_installer.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace components_extensions {

// A mock around CrxInstaller to track extension installations.
class MockCrxInstaller : public CrxInstaller {
 public:
  explicit MockCrxInstaller(ExtensionService* frontend);

  MOCK_METHOD(void, InstallCrxFile, (const extensions::CRXFileInfo& info), (override));

  MOCK_METHOD(void,
              AddInstallerCallback,
              (InstallerResultCallback callback),
              (override));

 protected:
  ~MockCrxInstaller() override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_MOCK_CRX_INSTALLER_H_
