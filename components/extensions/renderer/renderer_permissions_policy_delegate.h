// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_RENDERER_RENDERER_PERMISSIONS_POLICY_DELEGATE_H_
#define COMPONENTS_EXTENSIONS_RENDERER_RENDERER_PERMISSIONS_POLICY_DELEGATE_H_

#include "base/memory/raw_ptr.h"
#include "extensions/common/permissions/permissions_data.h"

namespace extensions {
class Dispatcher;
}

namespace components_extensions {

// Policy delegate for the renderer process.
class RendererPermissionsPolicyDelegate
    : public extensions::PermissionsData::PolicyDelegate {
 public:
  explicit RendererPermissionsPolicyDelegate(extensions::Dispatcher* dispatcher);

  RendererPermissionsPolicyDelegate(const RendererPermissionsPolicyDelegate&) =
      delete;
  RendererPermissionsPolicyDelegate& operator=(
      const RendererPermissionsPolicyDelegate&) = delete;

  ~RendererPermissionsPolicyDelegate() override;

  // PermissionsData::PolicyDelegate:
  bool IsRestrictedUrl(const GURL& document_url, std::string* error) override;

 private:
  raw_ptr<extensions::Dispatcher> dispatcher_;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_RENDERER_RENDERER_PERMISSIONS_POLICY_DELEGATE_H_