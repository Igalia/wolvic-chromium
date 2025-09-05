// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_COMMAND_LINE_PRIVATE_COMMAND_LINE_PRIVATE_API_H__
#define COMPONENTS_EXTENSIONS_BROWSER_API_COMMAND_LINE_PRIVATE_COMMAND_LINE_PRIVATE_API_H__

#include "extensions/browser/extension_function.h"

namespace extensions {

class CommandLinePrivateHasSwitchFunction : public ExtensionFunction {
  DECLARE_EXTENSION_FUNCTION("commandLinePrivate.hasSwitch",
                             COMMANDLINEPRIVATE_HASSWITCH)
 protected:
  ~CommandLinePrivateHasSwitchFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_COMMAND_LINE_PRIVATE_COMMAND_LINE_PRIVATE_API_H__
