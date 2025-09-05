// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_RENDERER_API_CHROME_EXTENSIONS_RENDERER_API_PROVIDER_H_
#define COMPONENTS_EXTENSIONS_RENDERER_API_CHROME_EXTENSIONS_RENDERER_API_PROVIDER_H_

#include "extensions/renderer/extensions_renderer_api_provider.h"

namespace extensions {
class ScriptContext;
class ResourceBundleSourceMap;
}

namespace components_extensions {

// Provides capabilities for extension APIs defined at the //chrome layer.
class ChromeExtensionsRendererAPIProvider
    : public extensions::ExtensionsRendererAPIProvider {
 public:
  ChromeExtensionsRendererAPIProvider() = default;
  ChromeExtensionsRendererAPIProvider(
      const ChromeExtensionsRendererAPIProvider&) = delete;
  ChromeExtensionsRendererAPIProvider& operator=(
      const ChromeExtensionsRendererAPIProvider&) = delete;
  ~ChromeExtensionsRendererAPIProvider() override = default;

  // ExtensionsRendererAPIProvider:
  void RegisterNativeHandlers(extensions::ModuleSystem* module_system,
                              extensions::NativeExtensionBindingsSystem* bindings_system,
                              extensions::V8SchemaRegistry* v8_schema_registry,
                              extensions::ScriptContext* context) const override;
  void AddBindingsSystemHooks(
      extensions::Dispatcher* dispatcher,
      extensions::NativeExtensionBindingsSystem* bindings_system) const override;
  void PopulateSourceMap(extensions::ResourceBundleSourceMap* source_map) const override;
  void EnableCustomElementAllowlist() const override;
  void RequireWebViewModules(extensions::ScriptContext* context) const override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_RENDERER_API_CHROME_EXTENSIONS_RENDERER_API_PROVIDER_H_
