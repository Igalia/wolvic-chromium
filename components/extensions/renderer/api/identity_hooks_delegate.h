// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_RENDERER_API_IDENTITY_HOOKS_DELEGATE_H_
#define COMPONENTS_EXTENSIONS_RENDERER_API_IDENTITY_HOOKS_DELEGATE_H_

#include "extensions/renderer/bindings/api_binding_hooks_delegate.h"
#include "extensions/renderer/bindings/api_signature.h"
#include "v8/include/v8-forward.h"

namespace components_extensions {

// Custom native hooks for the identity API.
class IdentityHooksDelegate : public extensions::APIBindingHooksDelegate {
 public:
  IdentityHooksDelegate();

  IdentityHooksDelegate(const IdentityHooksDelegate&) = delete;
  IdentityHooksDelegate& operator=(const IdentityHooksDelegate&) = delete;

  ~IdentityHooksDelegate() override;

  // APIBindingHooksDelegate:
  extensions::APIBindingHooks::RequestResult HandleRequest(
      const std::string& method_name,
      const extensions::APISignature* signature,
      v8::Local<v8::Context> context,
      v8::LocalVector<v8::Value>* arguments,
      const extensions::APITypeReferenceMap& refs) override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_RENDERER_API_IDENTITY_HOOKS_DELEGATE_H_
