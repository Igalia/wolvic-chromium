// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_SYSTEM_IMPL_H_
#define COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_SYSTEM_IMPL_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "base/one_shot_event.h"
#include "extensions/browser/app_sorting.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/unloaded_extension_reason.h"

namespace content {
class BrowserContext;
}

namespace extensions {
class UninstallPingSender;
}

namespace value_store {
class ValueStoreFactory;
class ValueStoreFactoryImpl;
}  // namespace value_store

namespace components_extensions {
class ExtensionService;
class ExtensionSystemSharedFactory;
class InstallGate;
class ExtensionsPermissionsTracker;

// The ExtensionSystem for ProfileImpl and OffTheRecordProfileImpl.
// Implementation details: non-shared services are owned by
// ExtensionSystemImpl, a KeyedService with separate incognito
// instances. A private Shared class (also a KeyedService,
// but with a shared instance for incognito) keeps the common services.
class ExtensionSystemImpl : public extensions::ExtensionSystem {
 public:
  using InstallUpdateCallback = ExtensionSystem::InstallUpdateCallback;

  explicit ExtensionSystemImpl(content::BrowserContext* context);

  ExtensionSystemImpl(const ExtensionSystemImpl&) = delete;
  ExtensionSystemImpl& operator=(const ExtensionSystemImpl&) = delete;

  ~ExtensionSystemImpl() override;

  // KeyedService implementation.
  void Shutdown() override;

  void InitForRegularProfile(bool extensions_enabled) override;

  ExtensionService* extension_service() override;  // shared
  extensions::ManagementPolicy* management_policy() override;  // shared
  extensions::ServiceWorkerManager* service_worker_manager() override;  // shared
  extensions::UserScriptManager* user_script_manager() override;        // shared
  extensions::StateStore* state_store() override;                              // shared
  extensions::StateStore* rules_store() override;                              // shared
  extensions::StateStore* dynamic_user_scripts_store() override;               // shared
  scoped_refptr<value_store::ValueStoreFactory> store_factory()
      override;                            // shared
  extensions::QuotaService* quota_service() override;  // shared
  extensions::AppSorting* app_sorting() override;      // shared
  const base::OneShotEvent& ready() const override;
  bool is_ready() const override;
  extensions::ContentVerifier* content_verifier() override;  // shared
  std::unique_ptr<extensions::ExtensionSet> GetDependentExtensions(
      const extensions::Extension* extension) override;
  void InstallUpdate(const std::string& extension_id,
                     const std::string& public_key,
                     const base::FilePath& unpacked_dir,
                     bool install_immediately,
                     InstallUpdateCallback install_update_callback) override;
  void PerformActionBasedOnOmahaAttributes(
      const std::string& extension_id,
      const base::Value::Dict& attributes) override;
  bool FinishDelayedInstallationIfReady(const std::string& extension_id,
                                        bool install_immediately) override;

 private:
  friend class ExtensionSystemSharedFactory;

  // Owns the Extension-related systems that have a single instance
  // shared between normal and incognito profiles.
  class Shared : public KeyedService {
   public:
    explicit Shared(content::BrowserContext* context);
    ~Shared() override;

    // Initialization takes place in phases.
    virtual void InitPrefs();
    // This must not be called until all the providers have been created.
    void RegisterManagementPolicyProviders();
    void InitInstallGates();
    void Init(bool extensions_enabled);

    // KeyedService implementation.
    void Shutdown() override;

    extensions::StateStore* state_store();
    extensions::StateStore* rules_store();
    extensions::StateStore* dynamic_user_scripts_store();
    scoped_refptr<value_store::ValueStoreFactory> store_factory() const;
    ExtensionService* extension_service();
    extensions::ManagementPolicy* management_policy();
    extensions::ServiceWorkerManager* service_worker_manager();
    extensions::UserScriptManager* user_script_manager();
    extensions::QuotaService* quota_service();
    extensions::AppSorting* app_sorting();
    const base::OneShotEvent& ready() const { return ready_; }
    bool is_ready() const { return ready_.is_signaled(); }
    extensions::ContentVerifier* content_verifier();

   private:
    raw_ptr<content::BrowserContext> browser_context_;

    // The services that are shared between normal and incognito profiles.

    std::unique_ptr<extensions::StateStore> state_store_;
    std::unique_ptr<extensions::StateStore> rules_store_;
    std::unique_ptr<extensions::StateStore> dynamic_user_scripts_store_;
    scoped_refptr<value_store::ValueStoreFactoryImpl> store_factory_;
    std::unique_ptr<extensions::ServiceWorkerManager> service_worker_manager_;
    // Shared memory region manager for scripts statically declared in extension
    // manifests. This region is shared between all extensions.
    std::unique_ptr<extensions::UserScriptManager> user_script_manager_;
    // ExtensionService depends on StateStore and Blocklist.
    std::unique_ptr<ExtensionService> extension_service_;
    std::unique_ptr<extensions::ManagementPolicy> management_policy_;
    std::unique_ptr<extensions::QuotaService> quota_service_;
    std::unique_ptr<extensions::AppSorting> app_sorting_;
    // TODO(mshin): Enable the below code after migrating UpdateInstallGate
    // std::unique_ptr<InstallGate> update_install_gate_;

    // For verifying the contents of extensions read from disk.
    scoped_refptr<extensions::ContentVerifier> content_verifier_;

    std::unique_ptr<extensions::UninstallPingSender> uninstall_ping_sender_;
    base::OneShotEvent ready_;
  };

  raw_ptr<content::BrowserContext> browser_context_;

  raw_ptr<Shared> shared_;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_SYSTEM_IMPL_H_
