// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/extension_api_unittest.h"

#include <array>

#include "extensions/browser/api_test_utils.h"
#include "extensions/browser/extension_function.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_builder.h"
#include "extensions/common/manifest.h"
#include "extensions/common/manifest_handlers/background_info.h"

namespace utils = extensions::api_test_utils;

using extensions::Extension;
using extensions::ExtensionBuilder;

namespace components_extensions {

ExtensionApiUnittest::ExtensionApiUnittest(
    std::unique_ptr<content::BrowserTaskEnvironment> task_environment)
    : task_environment_(std::move(task_environment)) {}

ExtensionApiUnittest::~ExtensionApiUnittest() {
}

void ExtensionApiUnittest::SetUp() {
  testing::Test::SetUp();
  extension_ = ExtensionBuilder("Test").Build();
  browser_context_ = std::make_unique<content::TestBrowserContext>();
  env_ = std::make_unique<TestExtensionEnvironment>(browser_context_.get(), nullptr);
}

std::optional<base::Value> ExtensionApiUnittest::RunFunctionAndReturnValue(
    scoped_refptr<ExtensionFunction> function,
    const std::string& args) {
  function->set_extension(extension());
  return utils::RunFunctionAndReturnSingleResult(std::move(function), args,
                                                 browser_context_.get());
}

std::optional<base::Value::Dict>
ExtensionApiUnittest::RunFunctionAndReturnDictionary(
    scoped_refptr<ExtensionFunction> function,
    const std::string& args) {
  std::optional<base::Value> value =
      RunFunctionAndReturnValue(std::move(function), args);
  // We expect to either have successfully retrieved a dictionary from the
  // value or the value to have been nullopt.
  EXPECT_TRUE(!value || value->is_dict());

  if (!value || !value->is_dict())
    return std::nullopt;

  return std::move(*value).TakeDict();
}

std::optional<base::Value::List> ExtensionApiUnittest::RunFunctionAndReturnList(
    scoped_refptr<ExtensionFunction> function,
    const std::string& args) {
  std::optional<base::Value> value =
      RunFunctionAndReturnValue(std::move(function), args);

  // We expect to have successfully retrieved a list from the value.
  EXPECT_TRUE(!value || value->is_list());

  if (!value || !value->is_list()) {
    return std::nullopt;
  }

  return std::move(*value).TakeList();
}

std::string ExtensionApiUnittest::RunFunctionAndReturnError(
    scoped_refptr<ExtensionFunction> function,
    const std::string& args) {
  function->set_extension(extension());
  return utils::RunFunctionAndReturnError(std::move(function), args,
                                          browser_context_.get());
}

void ExtensionApiUnittest::RunFunction(
    scoped_refptr<ExtensionFunction> function,
    const std::string& args) {
  RunFunctionAndReturnValue(std::move(function), args);
}

}  // namespace components_extensions
