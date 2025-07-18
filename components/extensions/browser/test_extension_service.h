// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_TEST_EXTENSION_SERVICE_H_
#define COMPONENTS_EXTENSIONS_BROWSER_TEST_EXTENSION_SERVICE_H_

#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "components/extensions/browser/extension_service.h"

namespace extensions {
class Extension;
}  // namespace extensions

namespace components_extensions {
class CWSInfoServiceInterface;
class CrxInstaller;
class PendingExtensionManager;

// Implementation of ExtensionServiceInterface with default
// implementations for methods that add failures.  You should subclass
// this and override the methods you care about.
class TestExtensionService : public ExtensionServiceInterface {
 public:
  TestExtensionService();
  ~TestExtensionService() override;

  // ExtensionServiceInterface implementation.
  PendingExtensionManager* pending_extension_manager() override;
  CorruptedExtensionReinstaller* corrupted_extension_reinstaller()
      override;

  scoped_refptr<CrxInstaller> CreateUpdateInstaller(
      const extensions::CRXFileInfo& file,
      bool file_ownership_passed) override;
  const extensions::Extension* GetPendingExtensionUpdate(
      const std::string& extension_id) const override;
  bool FinishDelayedInstallationIfReady(const std::string& extension_id,
                                        bool install_immediately) override;
  bool IsExtensionEnabled(const std::string& extension_id) const override;

  void CheckManagementPolicy() override;
  void CheckForUpdatesSoon() override;

  void AddExtension(const extensions::Extension* extension) override;
  void AddComponentExtension(const extensions::Extension* extension) override;

  void UnloadExtension(const std::string& extension_id,
                       extensions::UnloadedExtensionReason reason) override;
  void RemoveComponentExtension(const std::string& extension_id) override;

  bool UserCanDisableInstalledExtension(
      const std::string& extension_id) override;

  void ReinstallProviderExtensions() override;

  base::WeakPtr<ExtensionServiceInterface> AsWeakPtr() override;

 private:
  // TODO(mshin): Enable the below code after migrating CWSInfoServiceInterface
  // std::unique_ptr<CWSInfoServiceInterface> cws_info_service_;
  base::WeakPtrFactory<TestExtensionService> weak_ptr_factory_{this};
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_TEST_EXTENSION_SERVICE_H_
