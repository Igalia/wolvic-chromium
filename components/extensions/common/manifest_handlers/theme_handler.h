// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_COMMON_MANIFEST_HANDLERS_THEME_HANDLER_H_
#define COMPONENTS_EXTENSIONS_COMMON_MANIFEST_HANDLERS_THEME_HANDLER_H_

#include <memory>

#include "base/values.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_handler.h"

namespace components_extensions {

// A structure to hold the parsed theme data.
struct ThemeInfo : public extensions::Extension::ManifestData {
  // Define out of line constructor/destructor to please Clang.
  ThemeInfo();
  ~ThemeInfo() override;

  static const base::Value::Dict* GetImages(const extensions::Extension* extension);
  static const base::Value::Dict* GetColors(const extensions::Extension* extension);
  static const base::Value::Dict* GetTints(const extensions::Extension* extension);
  static const base::Value::Dict* GetDisplayProperties(
      const extensions::Extension* extension);

  // A map of resource id's to relative file paths.
  base::Value::Dict theme_images_;

  // A map of color names to colors.
  base::Value::Dict theme_colors_;

  // A map of color names to colors.
  base::Value::Dict theme_tints_;

  // A map of display properties.
  base::Value::Dict theme_display_properties_;
};

// Parses the "theme" manifest key.
class ThemeHandler : public extensions::ManifestHandler {
 public:
  ThemeHandler();

  ThemeHandler(const ThemeHandler&) = delete;
  ThemeHandler& operator=(const ThemeHandler&) = delete;

  ~ThemeHandler() override;

  bool Parse(extensions::Extension* extension, std::u16string* error) override;
  bool Validate(const extensions::Extension* extension,
                std::string* error,
                std::vector<extensions::InstallWarning>* warnings) const override;

 private:
  base::span<const char* const> Keys() const override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_COMMON_MANIFEST_HANDLERS_THEME_HANDLER_H_
