// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/test/test_extension_environment.h"

#include <utility>

#include "base/command_line.h"
#include "base/json/json_writer.h"
#include "base/values.h"
#include "components/extensions/browser/extension_service.h"
#include "components/extensions/browser/test_extension_system.h"
#include "components/prefs/pref_service.h"
#include "components/sessions/content/session_tab_helper.h"
#include "content/public/test/browser_task_environment.h"
#include "content/public/test/test_utils.h"
#include "content/public/test/web_contents_tester.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/test_extensions_browser_client.h"
#include "extensions/common/extension_builder.h"
#include "extensions/shell/browser/shell_prefs.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace components_extensions {

using content::BrowserThread;
using content::TestBrowserContext;
using extensions::Extension;
using extensions::ExtensionBuilder;
using extensions::ExtensionPrefs;
using extensions::ExtensionSystem;

namespace {

static TestExtensionEnvironment* g_test_extensions_environment;

base::Value::Dict MakeExtensionManifest(
    const base::Value::Dict& manifest_extra) {
  base::Value::Dict manifest = base::Value::Dict()
                                   .Set("name", "Extension")
                                   .Set("version", "1.0")
                                   .Set("manifest_version", 2);
  manifest.Merge(manifest_extra.Clone());
  return manifest;
}

base::Value::Dict MakePackagedAppManifest() {
  return base::Value::Dict()
      .Set("name", "Test App Name")
      .Set("version", "2.0")
      .Set("manifest_version", 2)
      .Set("app",
           base::Value::Dict().Set(
               "background",
               base::Value::Dict().Set(
                   "scripts", base::Value::List().Append("background.js"))));
}

}  // namespace

// static
ExtensionService* TestExtensionEnvironment::CreateExtensionServiceForBrowserContext(
    TestBrowserContext* context) {
  TestExtensionSystem* extension_system =
      static_cast<TestExtensionSystem*>(ExtensionSystem::Get(context));
  return extension_system->CreateExtensionService(
      base::CommandLine::ForCurrentProcess(), base::FilePath(), false);
}

// static
TestExtensionEnvironment* TestExtensionEnvironment::GetInstance() {
  DCHECK(g_test_extensions_environment);
  return g_test_extensions_environment;
}

TestExtensionEnvironment::TestExtensionEnvironment(
    Type type,
    ProfileCreationType profile_creation_mode)
    : task_environment_(
          type == Type::kWithTaskEnvironment
              ? std::make_unique<content::BrowserTaskEnvironment>()
              : nullptr),
      browser_context_(profile_creation_mode != ProfileCreationType::kCreate
                   ? nullptr
                   : std::make_unique<TestBrowserContext>()),
      browser_context_ptr_(browser_context_.get()) {
  DCHECK(!g_test_extensions_environment);
  g_test_extensions_environment = this;

  if (browser_context_) {
    local_state_ = extensions::shell_prefs::CreateLocalState(
        browser_context_->GetPath());
    user_pref_service_ = extensions::shell_prefs::CreateUserPrefService(
        browser_context_.get());
    extensions::ExtensionSystem::Get(browser_context_.get())->InitForRegularProfile(true);
  }
}

TestExtensionEnvironment::~TestExtensionEnvironment()  {
  g_test_extensions_environment = nullptr;
}

void TestExtensionEnvironment::SetBrowserContext(TestBrowserContext* browser_context) {
  browser_context_ptr_ = browser_context;
}

TestBrowserContext* TestExtensionEnvironment::browser_context() const {
  return browser_context_ptr_.get();
}

TestExtensionSystem* TestExtensionEnvironment::GetExtensionSystem() {
  return static_cast<TestExtensionSystem*>(ExtensionSystem::Get(browser_context()));
}

ExtensionService* TestExtensionEnvironment::GetExtensionService() {
  if (!extension_service_) {
    extension_service_ =
        extensions::ExtensionSystem::Get(browser_context())
            ->extension_service();
        // CreateExtensionServiceForBrowserContext(browser_context());
  }
  return extension_service_;
}

ExtensionPrefs* TestExtensionEnvironment::GetExtensionPrefs() {
  return ExtensionPrefs::Get(browser_context());
}

const Extension* TestExtensionEnvironment::MakeExtension(
    const base::Value::Dict& manifest_extra) {
  base::Value::Dict manifest = MakeExtensionManifest(manifest_extra);
  scoped_refptr<const Extension> result =
      ExtensionBuilder().SetManifest(std::move(manifest)).Build();
  GetExtensionService()->AddExtension(result.get());
  return result.get();
}

const Extension* TestExtensionEnvironment::MakeExtension(
    const base::Value::Dict& manifest_extra,
    const std::string& id) {
  base::Value::Dict manifest = MakeExtensionManifest(manifest_extra);
  scoped_refptr<const Extension> result =
      ExtensionBuilder().SetManifest(std::move(manifest)).SetID(id).Build();
  GetExtensionService()->AddExtension(result.get());
  return result.get();
}

scoped_refptr<const Extension> TestExtensionEnvironment::MakePackagedApp(
    const std::string& id,
    bool install) {
  scoped_refptr<const Extension> result =
      ExtensionBuilder()
          .SetManifest(MakePackagedAppManifest())
          .AddFlags(Extension::FROM_WEBSTORE)
          .SetID(id)
          .Build();
  if (install)
    GetExtensionService()->AddExtension(result.get());
  return result;
}

std::unique_ptr<content::WebContents> TestExtensionEnvironment::MakeTab()
    const {
  std::unique_ptr<content::WebContents> contents(
      content::WebContentsTester::CreateTestWebContents(browser_context(), nullptr));
  // Create a tab id.
  // TODO(mshin): Enable the below code after migrating SessionServiceTabHelper
  // CreateSessionServiceTabHelper(contents.get());
  return contents;
}

void TestExtensionEnvironment::DeleteBrowserContext() {
  browser_context_ptr_ = nullptr;
  browser_context_.reset();
  extension_service_ = nullptr;
}

}  // namespace components_extensions
