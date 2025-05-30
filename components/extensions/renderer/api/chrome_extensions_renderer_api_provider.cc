// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/renderer/api/chrome_extensions_renderer_api_provider.h"

#include "components/grit/components_resources.h"
#include "extensions/renderer/bindings/api_bindings_system.h"
#include "extensions/renderer/dispatcher.h"
#include "extensions/renderer/lazy_background_page_native_handler.h"
#include "extensions/renderer/module_system.h"
#include "extensions/renderer/native_extension_bindings_system.h"
#include "extensions/renderer/native_handler.h"
#include "extensions/renderer/resource_bundle_source_map.h"
#include "extensions/renderer/script_context.h"

using extensions::Dispatcher;
using extensions::ModuleSystem;
using extensions::NativeExtensionBindingsSystem;
using extensions::ResourceBundleSourceMap;
using extensions::ScriptContext;
using extensions::V8SchemaRegistry;

namespace components_extensions {

void ChromeExtensionsRendererAPIProvider::RegisterNativeHandlers(
    ModuleSystem* module_system,
    NativeExtensionBindingsSystem* bindings_system,
    V8SchemaRegistry* v8_schema_registry,
    ScriptContext* context) const {
  // TODO(mshin): Enable each handler after migrating APIs
  // module_system->RegisterNativeHandler(
  //     "sync_file_system",
  //     std::make_unique<SyncFileSystemCustomBindings>(context));
  // module_system->RegisterNativeHandler(
  //     "notifications_private",
  //     std::make_unique<NotificationsNativeHandler>(context));
  // module_system->RegisterNativeHandler(
  //     "mediaGalleries",
  //     std::make_unique<MediaGalleriesCustomBindings>(context));
  // module_system->RegisterNativeHandler(
  //     "page_capture", std::make_unique<PageCaptureCustomBindings>(
  //                         context, bindings_system->GetIPCMessageSender()));

  // // The following are native handlers that are defined in //extensions, but
  // // are only used for APIs defined in Chrome.
  // // TODO(devlin): We should clean this up. If an API is defined in Chrome,
  // // there's no reason to have its native handlers residing and being compiled
  // // in //extensions.
  // module_system->RegisterNativeHandler(
  //     "lazy_background_page",
  //     std::make_unique<LazyBackgroundPageNativeHandler>(context));
}

void ChromeExtensionsRendererAPIProvider::AddBindingsSystemHooks(
    Dispatcher* dispatcher,
    NativeExtensionBindingsSystem* bindings_system) const {
  // TODO(mshin): Enable each handler after migrating APIs
  // APIBindingsSystem* bindings = bindings_system->api_system();
  // bindings->RegisterHooksDelegate(
  //     "app", std::make_unique<extensions::AppHooksDelegate>(
  //                dispatcher, bindings->request_handler(),
  //                bindings_system->GetIPCMessageSender()));
  // bindings->RegisterHooksDelegate(
  //     "extension", std::make_unique<extensions::ExtensionHooksDelegate>(
  //                      bindings_system->messaging_service()));
  // bindings->RegisterHooksDelegate(
  //     "tabs", std::make_unique<extensions::TabsHooksDelegate>(
  //                 bindings_system->messaging_service()));
  // bindings->RegisterHooksDelegate(
  //     "identity", std::make_unique<extensions::IdentityHooksDelegate>());
}

void ChromeExtensionsRendererAPIProvider::PopulateSourceMap(
    ResourceBundleSourceMap* source_map) const {
  // Custom bindings.
  source_map->RegisterSource("action", IDR_ACTION_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("browserAction",
                             IDR_BROWSER_ACTION_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("declarativeContent",
                             IDR_DECLARATIVE_CONTENT_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("desktopCapture",
                             IDR_DESKTOP_CAPTURE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("developerPrivate",
                             IDR_DEVELOPER_PRIVATE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("downloads", IDR_DOWNLOADS_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("gcm", IDR_GCM_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("identity", IDR_IDENTITY_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("imageWriterPrivate",
                             IDR_IMAGE_WRITER_PRIVATE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("notifications",
                             IDR_NOTIFICATIONS_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("omnibox", IDR_OMNIBOX_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("pageAction", IDR_PAGE_ACTION_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("pageCapture",
                             IDR_PAGE_CAPTURE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("syncFileSystem",
                             IDR_SYNC_FILE_SYSTEM_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("systemIndicator",
                             IDR_SYSTEM_INDICATOR_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("tabCapture", IDR_TAB_CAPTURE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("tts", IDR_TTS_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("ttsEngine", IDR_TTS_ENGINE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource(
      "webrtcDesktopCapturePrivate",
      IDR_WEBRTC_DESKTOP_CAPTURE_PRIVATE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("webrtcLoggingPrivate",
                             IDR_WEBRTC_LOGGING_PRIVATE_CUSTOM_BINDINGS_JS);
}

void ChromeExtensionsRendererAPIProvider::EnableCustomElementAllowlist() const {
}

void ChromeExtensionsRendererAPIProvider::RequireWebViewModules(
    ScriptContext* context) const { }

}  // namespace components_extensions
