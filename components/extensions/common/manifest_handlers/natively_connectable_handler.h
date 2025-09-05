// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_COMMON_MANIFEST_HANDLERS_NATIVELY_CONNECTABLE_HANDLER_H_
#define COMPONENTS_EXTENSIONS_COMMON_MANIFEST_HANDLERS_NATIVELY_CONNECTABLE_HANDLER_H_

#include <set>
#include <string>

#include "base/containers/span.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_handler.h"

namespace components_extensions {

// A structure to hold the parsed list of native messaging hosts that can
// connect to this extension.
struct NativelyConnectableHosts : public extensions::Extension::ManifestData {
  NativelyConnectableHosts();
  ~NativelyConnectableHosts() override;

  static const std::set<std::string>* GetConnectableNativeMessageHosts(
      const extensions::Extension& extension);

  // A set of native messaging hosts allowed to initiate connection to this
  // extension.
  std::set<std::string> hosts;
};

// Parses the "natively_connectable" manifest key.
class NativelyConnectableHandler : public extensions::ManifestHandler {
 public:
  NativelyConnectableHandler();

  NativelyConnectableHandler(const NativelyConnectableHandler&) = delete;
  NativelyConnectableHandler& operator=(const NativelyConnectableHandler&) =
      delete;

  ~NativelyConnectableHandler() override;

  bool Parse(extensions::Extension* extension, std::u16string* error) override;

 private:
  base::span<const char* const> Keys() const override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_COMMON_MANIFEST_HANDLERS_NATIVELY_CONNECTABLE_HANDLER_H_
