// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Font Settings Extension API implementation.

#include "components/extensions/browser/api/font_settings/font_settings_api.h"

#include <stddef.h>

#include <utility>

#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/json/json_writer.h"
#include "base/lazy_instance.h"
#include "base/memory/raw_ptr.h"
#include "base/strings/string_util.h"
#include "base/trace_event/trace_event.h"
#include "base/values.h"
#include "components/extensions/common/api/font_settings.h"
#include "components/prefs/pref_change_registrar.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/font_list_async.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_event_histogram_value.h"
#include "extensions/browser/extension_prefs_helper.h"
#include "extensions/browser/extension_prefs_helper_factory.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/api/types.h"
#include "extensions/common/error_utils.h"

namespace extensions {

namespace fonts = api::font_settings;
using extensions::api::types::ChromeSettingScope;

namespace {

const char kFontIdKey[] = "fontId";
const char kDisplayNameKey[] = "displayName";
const char kPixelSizeKey[] = "pixelSize";

const char kSetFromIncognitoError[] =
    "Can't modify regular settings from an incognito context.";

// TODO(mshin): Replace the below code after migrating Preference
constexpr char kWebKitDefaultFontSize[] =
    "webkit.webprefs.default_font_size";
constexpr char kWebKitDefaultFixedFontSize[] =
    "webkit.webprefs.default_fixed_font_size";
constexpr char kWebKitMinimumFontSize[] =
    "webkit.webprefs.minimum_font_size";
}  // namespace

// TODO(mshin): Implement FontSettingsEventRouter
FontSettingsAPI::FontSettingsAPI(content::BrowserContext* context) {}

FontSettingsAPI::~FontSettingsAPI() {
}

static base::LazyInstance<BrowserContextKeyedAPIFactory<FontSettingsAPI>>::
    DestructorAtExit g_font_settings_api_factory = LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<FontSettingsAPI>*
FontSettingsAPI::GetFactoryInstance() {
  return g_font_settings_api_factory.Pointer();
}

ExtensionFunction::ResponseAction FontSettingsClearFontFunction::Run() {
  if (browser_context()->IsOffTheRecord())
    return RespondNow(Error(kSetFromIncognitoError));

  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction FontSettingsGetFontFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction FontSettingsSetFontFunction::Run() {
  if (browser_context()->IsOffTheRecord())
    return RespondNow(Error(kSetFromIncognitoError));

  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction FontSettingsGetFontListFunction::Run() {
  content::GetFontListAsync(
      BindOnce(&FontSettingsGetFontListFunction::FontListHasLoaded, this));
  return RespondLater();
}

void FontSettingsGetFontListFunction::FontListHasLoaded(
    base::Value::List list) {
  ExtensionFunction::ResponseValue response = CopyFontsToResult(list);
  Respond(std::move(response));
}

ExtensionFunction::ResponseValue
FontSettingsGetFontListFunction::CopyFontsToResult(
    const base::Value::List& fonts) {
  base::Value::List result;
  for (const auto& entry : fonts) {
    if (!entry.is_list()) {
      NOTREACHED();
      return Error("");
    }
    const base::Value::List& font_list_value = entry.GetList();

    if (font_list_value.size() < 2 || !font_list_value[0].is_string() ||
        !font_list_value[1].is_string()) {
      NOTREACHED();
      return Error("");
    }
    const std::string& name = font_list_value[0].GetString();
    const std::string& localized_name = font_list_value[1].GetString();

    base::Value::Dict font_name;
    font_name.Set(kFontIdKey, name);
    font_name.Set(kDisplayNameKey, localized_name);
    result.Append(std::move(font_name));
  }

  return WithArguments(std::move(result));
}

ExtensionFunction::ResponseAction ClearFontPrefExtensionFunction::Run() {
  if (browser_context()->IsOffTheRecord())
    return RespondNow(Error(kSetFromIncognitoError));

  ExtensionPrefsHelper::Get(browser_context())->RemoveExtensionControlledPref(
      extension_id(), GetPrefName(), ChromeSettingScope::kRegular);
  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction GetFontPrefExtensionFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction SetFontPrefExtensionFunction::Run() {
  if (browser_context()->IsOffTheRecord())
    return RespondNow(Error(kSetFromIncognitoError));

  EXTENSION_FUNCTION_VALIDATE(args().size() >= 1);
  EXTENSION_FUNCTION_VALIDATE(args()[0].is_dict());
  const base::Value& details = args()[0];
  const base::Value* value = details.GetDict().Find(GetKey());
  EXTENSION_FUNCTION_VALIDATE(value);

  ExtensionPrefsHelper::Get(browser_context())->SetExtensionControlledPref(
      extension_id(), GetPrefName(), ChromeSettingScope::kRegular,
      value->Clone());
  return RespondNow(NoArguments());
}

const char* FontSettingsClearDefaultFontSizeFunction::GetPrefName() {
  return kWebKitDefaultFontSize;
}

const char* FontSettingsGetDefaultFontSizeFunction::GetPrefName() {
  return kWebKitDefaultFontSize;
}

const char* FontSettingsGetDefaultFontSizeFunction::GetKey() {
  return kPixelSizeKey;
}

const char* FontSettingsSetDefaultFontSizeFunction::GetPrefName() {
  return kWebKitDefaultFontSize;
}

const char* FontSettingsSetDefaultFontSizeFunction::GetKey() {
  return kPixelSizeKey;
}

const char* FontSettingsClearDefaultFixedFontSizeFunction::GetPrefName() {
  return kWebKitDefaultFixedFontSize;
}

const char* FontSettingsGetDefaultFixedFontSizeFunction::GetPrefName() {
  return kWebKitDefaultFixedFontSize;
}

const char* FontSettingsGetDefaultFixedFontSizeFunction::GetKey() {
  return kPixelSizeKey;
}

const char* FontSettingsSetDefaultFixedFontSizeFunction::GetPrefName() {
  return kWebKitDefaultFixedFontSize;
}

const char* FontSettingsSetDefaultFixedFontSizeFunction::GetKey() {
  return kPixelSizeKey;
}

const char* FontSettingsClearMinimumFontSizeFunction::GetPrefName() {
  return kWebKitMinimumFontSize;
}

const char* FontSettingsGetMinimumFontSizeFunction::GetPrefName() {
  return kWebKitMinimumFontSize;
}

const char* FontSettingsGetMinimumFontSizeFunction::GetKey() {
  return kPixelSizeKey;
}

const char* FontSettingsSetMinimumFontSizeFunction::GetPrefName() {
  return kWebKitMinimumFontSize;
}

const char* FontSettingsSetMinimumFontSizeFunction::GetKey() {
  return kPixelSizeKey;
}

}  // namespace extensions
