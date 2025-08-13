// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/forced_extensions/install_stage_tracker_factory.h"

#include "base/no_destructor.h"
#include "components/extensions/browser/forced_extensions/install_stage_tracker.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"

namespace components_extensions {

using extensions::InstallationStage;
using extensions::Manifest;
using extensions::ManifestInvalidError;
using extensions::SandboxedUnpackerFailureReason;

// static
InstallStageTracker* InstallStageTrackerFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<InstallStageTracker*>(
      GetInstance()->GetServiceForBrowserContext(context, /*create=*/true));
}

// static
InstallStageTrackerFactory* InstallStageTrackerFactory::GetInstance() {
  static base::NoDestructor<InstallStageTrackerFactory> instance;
  return instance.get();
}

InstallStageTrackerFactory::InstallStageTrackerFactory()
    : BrowserContextKeyedServiceFactory(
          "InstallStageTracker",
          BrowserContextDependencyManager::GetInstance()) {}              


InstallStageTrackerFactory::~InstallStageTrackerFactory() = default;

std::unique_ptr<KeyedService>
InstallStageTrackerFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return std::make_unique<InstallStageTracker>(context);
}

}  // namespace components_extensions
