// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_COMPONENT_LOADER_H_
#define COMPONENTS_EXTENSIONS_BROWSER_COMPONENT_LOADER_H_

#include <stddef.h>

#include <optional>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/functional/callback_forward.h"
#include "base/gtest_prod_util.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "build/branding_buildflags.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "extensions/common/extension_id.h"

namespace content {
class BrowserContext;
}

namespace extensions {
class Extension;
class ExtensionSystem;
}

namespace components_extensions {

// For registering, loading, and unloading component extensions.
class ComponentLoader {
 public:
  ComponentLoader(extensions::ExtensionSystem* extension_system, content::BrowserContext* browser_context);

  ComponentLoader(const ComponentLoader&) = delete;
  ComponentLoader& operator=(const ComponentLoader&) = delete;

  virtual ~ComponentLoader();

  size_t registered_extensions_count() const {
    return component_extensions_.size();
  }

  // Creates and loads all registered component extensions.
  void LoadAll();

  // Registers and possibly loads a component extension. If ExtensionService
  // has been initialized, the extension is loaded; otherwise, the load is
  // deferred until LoadAll is called. The ID of the added extension is
  // returned.
  //
  // Component extension manifests must contain a "key" property with a unique
  // public key, serialized in base64. You can create a suitable value with the
  // following commands on a unixy system:
  //
  //   ssh-keygen -t rsa -b 1024 -N '' -f /tmp/key.pem
  //   openssl rsa -pubout -outform DER < /tmp/key.pem 2>/dev/null | base64 -w 0
  extensions::ExtensionId Add(const base::StringPiece& manifest_contents,
                              const base::FilePath& root_directory);

  // Convenience method for registering a component extension by resource id.
  extensions::ExtensionId Add(int manifest_resource_id,
                              const base::FilePath& root_directory);

  // Convenience method for registering a component extension by parsed
  // manifest.
  extensions::ExtensionId Add(base::Value::Dict manifest,
                              const base::FilePath& root_directory);

  // Loads a component extension from file system. Replaces previously added
  // extension with the same ID.
  extensions::ExtensionId AddOrReplace(const base::FilePath& path);

  // Returns true if an extension with the specified id has been added.
  bool Exists(const extensions::ExtensionId& id) const;

  // Unloads a component extension and removes it from the list of component
  // extensions to be loaded.
  void Remove(const base::FilePath& root_directory);
  void Remove(const extensions::ExtensionId& id);

  // Call this during test setup to load component extensions that have
  // background pages for testing, which could otherwise interfere with tests.
  static void EnableBackgroundExtensionsForTesting();

  // Adds the default component extensions. If |skip_session_components|
  // the loader will skip loading component extensions that weren't supposed to
  // be loaded unless we are in signed user session (ChromeOS). For all other
  // platforms this |skip_session_components| is expected to be unset.
  void AddDefaultComponentExtensions(bool skip_session_components);

  // Similar to above but adds the default component extensions for kiosk mode.
  void AddDefaultComponentExtensionsForKioskMode(bool skip_session_components);

  // Reloads a registered component extension.
  void Reload(const extensions::ExtensionId& extension_id);

  // Return ids of all registered extensions.
  std::vector<extensions::ExtensionId> GetRegisteredComponentExtensionsIds() const;

  void set_ignore_allowlist_for_testing(bool value) {
    ignore_allowlist_for_testing_ = value;
  }

  // Allows setting the profile used by the loader for testing purposes.
  void set_profile_for_testing(content::BrowserContext* context) {
    browser_context_ = context;
  }

 private:
  FRIEND_TEST_ALL_PREFIXES(ComponentLoaderTest, ParseManifest);

  // Information about a registered component extension.
  struct ComponentExtensionInfo {
    ComponentExtensionInfo(base::Value::Dict manifest_param,
                           const base::FilePath& root_directory);

    ComponentExtensionInfo(const ComponentExtensionInfo&) = delete;
    ComponentExtensionInfo& operator=(const ComponentExtensionInfo&) = delete;

    ~ComponentExtensionInfo();

    ComponentExtensionInfo(ComponentExtensionInfo&& other);
    ComponentExtensionInfo& operator=(ComponentExtensionInfo&& other);

    // The parsed contents of the extensions's manifest file.
    base::Value::Dict manifest;

    // Directory where the extension is stored.
    base::FilePath root_directory;

    // The component extension's ID.
    extensions::ExtensionId extension_id;
  };

  // Parses the given JSON manifest. Returns `std::nullopt` if it cannot be
  // parsed or if the result is not a base::Value::Dict.
  std::optional<base::Value::Dict> ParseManifest(
      base::StringPiece manifest_contents) const;

  extensions::ExtensionId Add(const base::StringPiece& manifest_contents,
                              const base::FilePath& root_directory,
                              bool skip_allowlist);
  extensions::ExtensionId Add(base::Value::Dict parsed_manifest,
                              const base::FilePath& root_directory,
                              bool skip_allowlist);

  // Loads a registered component extension.
  void Load(const ComponentExtensionInfo& info);

  void AddDefaultComponentExtensionsWithBackgroundPages(
      bool skip_session_components);
  void AddDefaultComponentExtensionsWithBackgroundPagesForKioskMode();

  void AddNetworkSpeechSynthesisExtension();

  void AddWithNameAndDescription(int manifest_resource_id,
                                 const base::FilePath& root_directory,
                                 const std::string& name_string,
                                 const std::string& description_string);
  void AddWebStoreApp();

  scoped_refptr<const extensions::Extension> CreateExtension(
      const ComponentExtensionInfo& info, std::string* utf8_error);

  // Unloads |component| from the memory.
  void UnloadComponent(ComponentExtensionInfo* component);

  // Finishes loading an extension tts engine.
  void FinishLoadSpeechSynthesisExtension(const extensions::ExtensionId& extension_id);

  raw_ptr<content::BrowserContext> browser_context_;

  raw_ptr<extensions::ExtensionSystem, AcrossTasksDanglingUntriaged> extension_system_;

  // List of registered component extensions (see mojom::ManifestLocation).
  typedef std::vector<ComponentExtensionInfo> RegisteredComponentExtensions;
  RegisteredComponentExtensions component_extensions_;

  bool ignore_allowlist_for_testing_;

  base::WeakPtrFactory<ComponentLoader> weak_factory_{this};

  friend class TtsApiTest;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_COMPONENT_LOADER_H_
