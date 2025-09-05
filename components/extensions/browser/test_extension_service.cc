// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/test_extension_service.h"

#include "components/extensions/browser/crx_installer.h"
#include "testing/gtest/include/gtest/gtest.h"

using extensions::Extension;

namespace components_extensions {

TestExtensionService::TestExtensionService() = default;

TestExtensionService::~TestExtensionService() = default;

PendingExtensionManager*
TestExtensionService::pending_extension_manager() {
  ADD_FAILURE();
  return nullptr;
}

CorruptedExtensionReinstaller*
TestExtensionService::corrupted_extension_reinstaller() {
  ADD_FAILURE();
  return nullptr;
}

scoped_refptr<CrxInstaller>
TestExtensionService::CreateUpdateInstaller(const extensions::CRXFileInfo& file,
                                            bool file_ownership_passed) {
  ADD_FAILURE();
  return nullptr;
}

const Extension* TestExtensionService::GetPendingExtensionUpdate(
    const std::string& id) const {
  ADD_FAILURE();
  return nullptr;
}

bool TestExtensionService::FinishDelayedInstallationIfReady(
    const std::string& extension_id,
    bool install_immediately) {
  ADD_FAILURE();
  return false;
}

bool TestExtensionService::IsExtensionEnabled(
    const std::string& extension_id) const {
  ADD_FAILURE();
  return false;
}

void TestExtensionService::CheckManagementPolicy() {
  ADD_FAILURE();
}

void TestExtensionService::CheckForUpdatesSoon() {
  ADD_FAILURE();
}

void TestExtensionService::AddExtension(const Extension* extension) {
  ADD_FAILURE();
}

void TestExtensionService::AddComponentExtension(const Extension* extension) {
  ADD_FAILURE();
}

void TestExtensionService::UnloadExtension(
    const std::string& extension_id,
    extensions::UnloadedExtensionReason reason) {
  ADD_FAILURE();
}

void TestExtensionService::RemoveComponentExtension(
    const std::string& extension_id) {
  ADD_FAILURE();
}

bool TestExtensionService::UserCanDisableInstalledExtension(
    const std::string& extension_id) {
  ADD_FAILURE();
  return false;
}

void TestExtensionService::ReinstallProviderExtensions() {
  ADD_FAILURE();
}

base::WeakPtr<ExtensionServiceInterface>
TestExtensionService::AsWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

}  // namespace components_extensions
