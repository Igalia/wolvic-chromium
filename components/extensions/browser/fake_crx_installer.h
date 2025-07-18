// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_FAKE_CRX_INSTALLER_H_
#define COMPONENTS_EXTENSIONS_BROWSER_FAKE_CRX_INSTALLER_H_

#include "components/extensions/browser/crx_installer.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace components_extensions {

// A fake CrxInstaller.
//
// Has InstallCrxFile as a NOOP, letting test code
// decide when to call RunInstallerCallbacks to fake installation
// completion.
class FakeCrxInstaller : public CrxInstaller {
 public:
  explicit FakeCrxInstaller(ExtensionService* frontend);

  void InstallCrxFile(const extensions::CRXFileInfo& info) override;

  using CrxInstaller::RunInstallerCallbacks;

 protected:
  ~FakeCrxInstaller() override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_FAKE_CRX_INSTALLER_H_
