// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/extension_system_factory.h"

#include "components/extensions/browser/extension_management.h"
#include "components/extensions/browser/install_verifier_factory.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "extensions/browser/event_router_factory.h"
#include "extensions/browser/extension_host_registry.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/browser/extension_registry_factory.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/browser/process_manager_factory.h"
#include "extensions/browser/renderer_startup_helper.h"

using extensions::EventRouterFactory;
using extensions::ExtensionsBrowserClient;
using extensions::ExtensionHostRegistry;
using extensions::ExtensionPrefsFactory;
using extensions::ExtensionRegistryFactory;
using extensions::ExtensionSystem;
using extensions::ProcessManagerFactory;
using extensions::RendererStartupHelperFactory;

namespace components_extensions {

// ExtensionSystemSharedFactory

// static
ExtensionSystemImpl::Shared*
ExtensionSystemSharedFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<ExtensionSystemImpl::Shared*>(
      GetInstance()->GetServiceForBrowserContext(
          ExtensionsBrowserClient::Get()->GetOriginalContext(context), true));
}

// static
ExtensionSystemSharedFactory* ExtensionSystemSharedFactory::GetInstance() {
  static base::NoDestructor<ExtensionSystemSharedFactory> instance;
  return instance.get();
}

ExtensionSystemSharedFactory::ExtensionSystemSharedFactory()
    : BrowserContextKeyedServiceFactory(
          "ExtensionSystemShared",
          BrowserContextDependencyManager::GetInstance()) {
  DependsOn(ExtensionPrefsFactory::GetInstance());
  DependsOn(ExtensionManagementFactory::GetInstance());
  // This depends on ExtensionService, which depends on ExtensionRegistry.
  DependsOn(ExtensionRegistryFactory::GetInstance());
  // TODO(mshin): Enable the below code after migrating GlobalErrorServiceFactory
  // DependsOn(GlobalErrorServiceFactory::GetInstance());
  DependsOn(InstallVerifierFactory::GetInstance());
  DependsOn(ProcessManagerFactory::GetInstance());
  DependsOn(RendererStartupHelperFactory::GetInstance());
  // TODO(mshin): Enable the below code after migrating BlocklistFactory
  // DependsOn(BlocklistFactory::GetInstance());
  DependsOn(EventRouterFactory::GetInstance());
  // This depends on ExtensionDownloader, which depends on
  // IdentityManager for webstore authentication.
  // TODO(mshin): Enable the below code after migrating IdentityManagerFactory
  // DependsOn(IdentityManagerFactory::GetInstance());
  // TODO(mshin): Enable the below code after migrating InstallStageTrackerFactory
  // DependsOn(InstallStageTrackerFactory::GetInstance());
  // ExtensionService (owned by the ExtensionSystem) depends on
  // ExtensionHostRegistry.
  DependsOn(ExtensionHostRegistry::GetFactory());
}

ExtensionSystemSharedFactory::~ExtensionSystemSharedFactory() = default;

std::unique_ptr<KeyedService>
ExtensionSystemSharedFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return std::make_unique<ExtensionSystemImpl::Shared>(context);
}

// ExtensionSystemFactory

// static
ExtensionSystem* ExtensionSystemFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<ExtensionSystem*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
ExtensionSystemFactory* ExtensionSystemFactory::GetInstance() {
  static base::NoDestructor<ExtensionSystemFactory> instance;
  return instance.get();
}

ExtensionSystemFactory::ExtensionSystemFactory()
    : ExtensionSystemProvider("ExtensionSystem",
                              BrowserContextDependencyManager::GetInstance()) {
  DCHECK(ExtensionsBrowserClient::Get())
      << "ExtensionSystemFactory must be initialized after BrowserProcess";
  DependsOn(ExtensionSystemSharedFactory::GetInstance());
}

ExtensionSystemFactory::~ExtensionSystemFactory() = default;

KeyedService* ExtensionSystemFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  return new ExtensionSystemImpl(context);
}

content::BrowserContext* ExtensionSystemFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return context;
}

bool ExtensionSystemFactory::ServiceIsCreatedWithBrowserContext() const {
  return true;
}

}  // namespace components_extensions
