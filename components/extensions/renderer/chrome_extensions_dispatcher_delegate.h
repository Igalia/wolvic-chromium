// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_RENDERER_CHROME_EXTENSIONS_DISPATCHER_DELEGATE_H_
#define COMPONENTS_EXTENSIONS_RENDERER_CHROME_EXTENSIONS_DISPATCHER_DELEGATE_H_

#include "extensions/renderer/dispatcher_delegate.h"

namespace components_extensions {

class ChromeExtensionsDispatcherDelegate
    : public extensions::DispatcherDelegate {
 public:
  ChromeExtensionsDispatcherDelegate();

  ChromeExtensionsDispatcherDelegate(
      const ChromeExtensionsDispatcherDelegate&) = delete;
  ChromeExtensionsDispatcherDelegate& operator=(
      const ChromeExtensionsDispatcherDelegate&) = delete;

  ~ChromeExtensionsDispatcherDelegate() override;

 private:
  // extensions::DispatcherDelegate implementation.
  void OnActiveExtensionsUpdated(
      const std::set<std::string>& extensions_ids) override;
};

}  // components_extensions

#endif  // COMPONENTS_EXTENSIONS_RENDERER_CHROME_EXTENSIONS_DISPATCHER_DELEGATE_H_
