// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/mock_crx_installer.h"

namespace components_extensions {

MockCrxInstaller::MockCrxInstaller(ExtensionService* frontend)
    : CrxInstaller(frontend->AsExtensionServiceWeakPtr(), nullptr, nullptr) {}

MockCrxInstaller::~MockCrxInstaller() = default;

}  // namespace components_extensions
