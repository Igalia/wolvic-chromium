// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_COMMON_MANIFEST_HANDLERS_APP_LAUNCH_INFO_H_
#define COMPONENTS_EXTENSIONS_COMMON_MANIFEST_HANDLERS_APP_LAUNCH_INFO_H_

#include <string>
#include <vector>

#include "components/extensions/common/extension_constants.h"
#include "components/services/app_service/public/cpp/app_launch_util.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest.h"
#include "extensions/common/manifest_handler.h"
#include "url/gurl.h"

namespace components_extensions {

// Container that holds the parsed app launch data.
class AppLaunchInfo : public extensions::Extension::ManifestData {
 public:
  AppLaunchInfo();

  AppLaunchInfo(const AppLaunchInfo&) = delete;
  AppLaunchInfo& operator=(const AppLaunchInfo&) = delete;

  ~AppLaunchInfo() override;

  // Get the local path inside the extension to use with the launcher.
  static const std::string& GetLaunchLocalPath(const extensions::Extension* extension);

  // Get the absolute web url to use with the launcher.
  static const GURL& GetLaunchWebURL(const extensions::Extension* extension);

  // The window type that an app's manifest specifies to launch into.
  // This is not always the window type an app will open into, because
  // users can override the way each app launches.  See
  // ExtensionPrefs::GetLaunchContainer(), which looks at a per-app pref
  // to decide what container an app will launch in.
  static apps::LaunchContainer GetLaunchContainer(const extensions::Extension* extension);

  // The default size of the container when launching. Only respected for
  // containers like panels and windows.
  static int GetLaunchWidth(const extensions::Extension* extension);
  static int GetLaunchHeight(const extensions::Extension* extension);

  // Get the fully resolved absolute launch URL.
  static GURL GetFullLaunchURL(const extensions::Extension* extension);

  bool Parse(extensions::Extension* extension, std::u16string* error);

 private:
  bool LoadLaunchURL(extensions::Extension* extension, std::u16string* error);
  bool LoadLaunchContainer(extensions::Extension* extension, std::u16string* error);
  void OverrideLaunchURL(extensions::Extension* extension, GURL override_url);

  std::string launch_local_path_;

  GURL launch_web_url_;

  apps::LaunchContainer launch_container_ =
      apps::LaunchContainer::kLaunchContainerTab;

  int launch_width_ = 0;
  int launch_height_ = 0;
};

// Parses all app launch related keys in the manifest.
class AppLaunchManifestHandler : public extensions::ManifestHandler {
 public:
  AppLaunchManifestHandler();

  AppLaunchManifestHandler(const AppLaunchManifestHandler&) = delete;
  AppLaunchManifestHandler& operator=(const AppLaunchManifestHandler&) = delete;

  ~AppLaunchManifestHandler() override;

  bool Parse(extensions::Extension* extension, std::u16string* error) override;
  bool AlwaysParseForType(extensions::Manifest::Type type) const override;

 private:
  base::span<const char* const> Keys() const override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_COMMON_MANIFEST_HANDLERS_APP_LAUNCH_INFO_H_
