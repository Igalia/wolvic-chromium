// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/renderer/renderer_permissions_policy_delegate.h"

#include "base/command_line.h"
#include "components/extensions/common/extension_constants.h"
#include "extensions/common/constants.h"
#include "extensions/common/extensions_client.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/common/switches.h"
#include "extensions/renderer/dispatcher.h"

using extensions::Dispatcher;
using extensions::PermissionsData;

namespace components_extensions {

namespace errors = extensions::manifest_errors;

RendererPermissionsPolicyDelegate::RendererPermissionsPolicyDelegate(
    Dispatcher* dispatcher) : dispatcher_(dispatcher) {
  PermissionsData::SetPolicyDelegate(this);
}
RendererPermissionsPolicyDelegate::~RendererPermissionsPolicyDelegate() {
  PermissionsData::SetPolicyDelegate(nullptr);
}

bool RendererPermissionsPolicyDelegate::IsRestrictedUrl(
    const GURL& document_url,
    std::string* error) {
  if (dispatcher_->IsExtensionActive(extensions:: kWebStoreAppId)) {
    if (error)
      *error = errors::kCannotScriptGallery;
    return true;
  }

  return false;
}

}  // namespace components_extensions
