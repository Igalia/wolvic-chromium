// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/common/chrome_manifest_handlers.h"

#include <memory>

// TODO(mshin): Enable the below code after migrating step by step
// #include "components/extensions/common/api/omnibox/omnibox_handler.h"
// #include "components/extensions/common/api/side_panel/side_panel_info.h"
// #include "components/extensions/common/api/speech/tts_engine_manifest_handler.h"
// #include "components/extensions/common/api/storage/storage_schema_manifest_handler.h"
// #include "components/extensions/common/api/system_indicator/system_indicator_handler.h"
#include "components/extensions/common/api/url_handlers/url_handlers_parser.h"
#include "components/extensions/common/chrome_manifest_url_handlers.h"
#include "components/extensions/common/manifest_handlers/app_launch_info.h"
#include "components/extensions/common/manifest_handlers/minimum_chrome_version_checker.h"
#include "components/extensions/common/manifest_handlers/natively_connectable_handler.h"
#include "components/extensions/common/manifest_handlers/settings_overrides_handler.h"
#include "components/extensions/common/manifest_handlers/theme_handler.h"

using extensions::ManifestHandlerRegistry;

namespace components_extensions {

void RegisterChromeManifestHandlers() {
  // TODO(devlin): Pass in |registry| rather than Get()ing it.
  ManifestHandlerRegistry* registry = ManifestHandlerRegistry::Get();

  // TODO(mshin): Enable the below code after migrating step by step
  registry->RegisterHandler(std::make_unique<AppLaunchManifestHandler>());
  registry->RegisterHandler(std::make_unique<DevToolsPageHandler>());
  registry->RegisterHandler(std::make_unique<MinimumChromeVersionChecker>());
  registry->RegisterHandler(std::make_unique<NativelyConnectableHandler>());
  // registry->RegisterHandler(std::make_unique<OmniboxHandler>());
  registry->RegisterHandler(std::make_unique<SettingsOverridesHandler>());
  // registry->RegisterHandler(std::make_unique<SidePanelManifestHandler>());
  // registry->RegisterHandler(std::make_unique<StorageSchemaManifestHandler>());
  // registry->RegisterHandler(std::make_unique<SystemIndicatorHandler>());
  registry->RegisterHandler(std::make_unique<ThemeHandler>());
  // registry->RegisterHandler(std::make_unique<TtsEngineManifestHandler>());
  registry->RegisterHandler(std::make_unique<UrlHandlersParser>());
  registry->RegisterHandler(std::make_unique<URLOverridesHandler>());
}

}  // namespace components_extensions
