// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_ACTION_STORAGE_MANAGER_H_
#define COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_ACTION_STORAGE_MANAGER_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "components/extensions/browser/api/extension_action/extension_action_api.h"
#include "extensions/browser/extension_action.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_registry_observer.h"
#include "extensions/common/extension_id.h"

namespace content {
class BrowserContext;
}

namespace extensions {
class StateStore;
}

namespace components_extensions {

// This class manages reading and writing browser action values from storage.
class ExtensionActionStorageManager : public extensions::ExtensionActionAPI::Observer,
                                      public extensions::ExtensionRegistryObserver {
 public:
  explicit ExtensionActionStorageManager(content::BrowserContext* context);

  ExtensionActionStorageManager(const ExtensionActionStorageManager&) = delete;
  ExtensionActionStorageManager& operator=(
      const ExtensionActionStorageManager&) = delete;

  ~ExtensionActionStorageManager() override;

 private:
  // ExtensionActionAPI::Observer:
  void OnExtensionActionUpdated(
      extensions::ExtensionAction* extension_action,
      content::WebContents* web_contents,
      content::BrowserContext* browser_context) override;
  void OnExtensionActionAPIShuttingDown() override;

  // ExtensionRegistryObserver:
  void OnExtensionLoaded(content::BrowserContext* browser_context,
                         const extensions::Extension* extension) override;

  // Reads/Writes the ExtensionAction's default values to/from storage.
  void WriteToStorage(extensions::ExtensionAction* extension_action);
  void ReadFromStorage(const extensions::ExtensionId& extension_id,
                       std::optional<base::Value> value);

  // Returns the Extensions StateStore for the |browser_context_|.
  // May return NULL.
  extensions::StateStore* GetStateStore();

  raw_ptr<content::BrowserContext> browser_context_;

  base::ScopedObservation<extensions::ExtensionActionAPI,
                          extensions::ExtensionActionAPI::Observer>
      extension_action_observation_{this};

  base::ScopedObservation<extensions::ExtensionRegistry,
                          extensions::ExtensionRegistryObserver>
      extension_registry_observation_{this};

  base::WeakPtrFactory<ExtensionActionStorageManager> weak_factory_{this};
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_ACTION_STORAGE_MANAGER_H_
