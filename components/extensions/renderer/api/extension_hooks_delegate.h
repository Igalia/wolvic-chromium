// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_RENDERER_API_EXTENSION_HOOKS_DELEGATE_H_
#define COMPONENTS_EXTENSIONS_RENDERER_API_EXTENSION_HOOKS_DELEGATE_H_

#include "base/memory/raw_ptr.h"
#include "extensions/renderer/bindings/api_binding_hooks_delegate.h"
#include "extensions/renderer/bindings/api_signature.h"
#include "v8/include/v8.h"

namespace extensions {
class NativeRendererMessagingService;
class ScriptContext;
}

namespace components_extensions {

// The custom hooks for the chrome.extension API.
class ExtensionHooksDelegate : public extensions::APIBindingHooksDelegate {
 public:
  explicit ExtensionHooksDelegate(
      extensions::NativeRendererMessagingService* messaging_service);

  ExtensionHooksDelegate(const ExtensionHooksDelegate&) = delete;
  ExtensionHooksDelegate& operator=(const ExtensionHooksDelegate&) = delete;

  ~ExtensionHooksDelegate() override;

  // APIBindingHooksDelegate:
  extensions::APIBindingHooks::RequestResult HandleRequest(
      const std::string& method_name,
      const extensions::APISignature* signature,
      v8::Local<v8::Context> context,
      v8::LocalVector<v8::Value>* arguments,
      const extensions::APITypeReferenceMap& refs) override;
  void InitializeTemplate(v8::Isolate* isolate,
                          v8::Local<v8::ObjectTemplate> object_template,
                          const extensions::APITypeReferenceMap& type_refs) override;
  void InitializeInstance(v8::Local<v8::Context> context,
                          v8::Local<v8::Object> instance) override;

 private:
  // Request handlers for the corresponding API methods.
  extensions::APIBindingHooks::RequestResult HandleSendRequest(
      extensions::ScriptContext* script_context,
      const extensions::APISignature::V8ParseResult& parse_result);
  extensions::APIBindingHooks::RequestResult HandleGetURL(
      extensions::ScriptContext* script_context,
      const extensions::APISignature::V8ParseResult& parse_result);
  extensions::APIBindingHooks::RequestResult HandleGetViews(
      extensions::ScriptContext* script_context,
      const extensions::APISignature::V8ParseResult& parse_result);
  extensions::APIBindingHooks::RequestResult HandleGetExtensionTabs(
      extensions::ScriptContext* script_context,
      const extensions::APISignature::V8ParseResult& parse_result);
  extensions::APIBindingHooks::RequestResult HandleGetBackgroundPage(
      extensions::ScriptContext* script_context,
      const extensions::APISignature::V8ParseResult& parse_result);

  // The messaging service to handle messaging calls.
  // Guaranteed to outlive this object.
  const raw_ptr<extensions::NativeRendererMessagingService, DanglingUntriaged>
      messaging_service_;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_RENDERER_API_EXTENSION_HOOKS_DELEGATE_H_
