// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_BROWSER_EXTENSIONS_TEST_EXTENSION_PREFS_H_
#define COMPONENTS_BROWSER_EXTENSIONS_TEST_EXTENSION_PREFS_H_

#include <memory>
#include <string>

#include "base/files/scoped_temp_dir.h"
#include "base/memory/ref_counted.h"
#include "base/values.h"
#include "content/public/test/test_browser_context.h"
#include "extensions/common/mojom/manifest.mojom-shared.h"

class ExtensionPrefValueMap;
class PrefService;

namespace base {
class SequencedTaskRunner;
}

namespace extensions {
class Extension;
class ExtensionPrefs;
}

namespace sync_preferences {
class PrefServiceSyncable;
}

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace components_extensions {
class ChromeAppSorting;
class TestExtensionEnvironment;

// This is a test class intended to make it easier to work with ExtensionPrefs
// in tests.
class TestExtensionPrefs {
 public:
  explicit TestExtensionPrefs(
      const scoped_refptr<base::SequencedTaskRunner>& task_runner);

  TestExtensionPrefs(const TestExtensionPrefs&) = delete;
  TestExtensionPrefs& operator=(const TestExtensionPrefs&) = delete;

  virtual ~TestExtensionPrefs();

  extensions::ExtensionPrefs* prefs();
  content::TestBrowserContext* browser_context();

  PrefService* pref_service();
  const scoped_refptr<user_prefs::PrefRegistrySyncable>& pref_registry();
  void ResetPrefRegistry();
  const base::FilePath& temp_dir() const { return temp_dir_.GetPath(); }
  const base::FilePath& extensions_dir() const { return extensions_dir_; }
  ExtensionPrefValueMap* extension_pref_value_map() {
    return extension_pref_value_map_.get();
  }

  // This will cause the ExtensionPrefs to be deleted and recreated, based on
  // any existing backing file we had previously created.
  void RecreateExtensionPrefs();

  // Creates a new Extension with the given name in our temp dir, adds it to
  // our ExtensionPrefs, and returns it.
  scoped_refptr<extensions::Extension> AddExtension(const std::string& name);

  // As above, but the extension is an app.
  scoped_refptr<extensions::Extension> AddApp(const std::string& name);

  // Similar to AddExtension, but with a specified location.
  scoped_refptr<extensions::Extension> AddExtensionWithLocation(
      const std::string& name,
      extensions::mojom::ManifestLocation location);

  // Similar to AddExtension, but takes a dictionary with manifest values.
  scoped_refptr<extensions::Extension> AddExtensionWithManifest(
      const base::Value::Dict& manifest,
      extensions::mojom::ManifestLocation location);

  // Similar to AddExtension, but takes a dictionary with manifest values
  // and extension flags.
  scoped_refptr<extensions::Extension> AddExtensionWithManifestAndFlags(
      const base::Value::Dict& manifest,
      extensions::mojom::ManifestLocation location,
      int extra_flags);

  // Similar to AddExtension, this adds a new test Extension. This is useful for
  // cases when you don't need the Extension object, but just the id it was
  // assigned.
  std::string AddExtensionAndReturnId(const std::string& name);

  // This will add extension in our ExtensionPrefs.
  void AddExtension(const extensions::Extension* extension);

  std::unique_ptr<PrefService> CreateIncognitoPrefService() const;

  // Allows disabling the loading of preferences of extensions. Becomes
  // active after calling RecreateExtensionPrefs(). Defaults to false.
  void set_extensions_disabled(bool extensions_disabled);

  ChromeAppSorting* app_sorting();

  static void AddDefaultManifestKeys(const std::string& name,
                                     base::Value::Dict& dict);

 protected:
  class IncrementalClock;

  base::ScopedTempDir temp_dir_;
  base::FilePath preferences_file_;
  base::FilePath extensions_dir_;
  scoped_refptr<user_prefs::PrefRegistrySyncable> pref_registry_;
  std::unique_ptr<sync_preferences::PrefServiceSyncable> pref_service_;
  std::unique_ptr<ExtensionPrefValueMap> extension_pref_value_map_;
  const scoped_refptr<base::SequencedTaskRunner> task_runner_;
  std::unique_ptr<TestExtensionEnvironment> env_;

 private:
  // |clock_| is injected to an ExtensionPrefs that associated to |browser_context_|.
  // Put |clock_| above |browser_context_| to outlive it.
  std::unique_ptr<IncrementalClock> clock_;
  content::TestBrowserContext browser_context_;
  bool extensions_disabled_;
};

}  // namespace components_extensions

#endif  // COMPONENTS_BROWSER_EXTENSIONS_TEST_EXTENSION_PREFS_H_
