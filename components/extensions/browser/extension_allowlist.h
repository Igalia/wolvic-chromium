// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_ALLOWLIST_H_
#define COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_ALLOWLIST_H_

#include "base/memory/raw_ptr.h"
#include "base/observer_list.h"
#include "base/scoped_observation.h"
#include "components/prefs/pref_change_registrar.h"
#include "extensions/browser/allowlist_state.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_prefs_observer.h"
#include "extensions/common/extension_id.h"

namespace base {
class Value;
}  // namespace base

namespace extensions {
class ExtensionRegistry;
}

namespace components_extensions {
class ExtensionService;

// Manages the Safe Browsing CRX Allowlist.
class ExtensionAllowlist : private extensions::ExtensionPrefsObserver {
 public:
  class Observer : public base::CheckedObserver {
   public:
    // Called when an extension's allowlist warning state is changed.
    //
    // This can occur when an extension is included/excluded of the allowlist,
    // or when the user turns on/off the Enhanced Safe Browsing setting.
    virtual void OnExtensionAllowlistWarningStateChanged(
        const extensions::ExtensionId& extension_id,
        bool show_warning) {}
  };

  // Constructor stores pointers to `browser_context`, `extension_prefs` and
  // `extension_service`. They must outlive this object and the ownership
  // remains at caller.
  ExtensionAllowlist(content::BrowserContext* browser_context,
                     extensions::ExtensionPrefs* extension_prefs,
                     ExtensionService* extension_service);
  ExtensionAllowlist(const ExtensionAllowlist&) = delete;
  ExtensionAllowlist& operator=(const ExtensionAllowlist&) = delete;
  ~ExtensionAllowlist();

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  void Init();

  // Gets the Safe Browsing allowlist state.
  extensions::AllowlistState GetExtensionAllowlistState(
      const extensions::ExtensionId& extension_id) const;

  // Sets the Safe Browsing allowlist state.
  void SetExtensionAllowlistState(const extensions::ExtensionId& extension_id,
                                  extensions::AllowlistState state);

  // Gets the Safe Browsing allowlist acknowledge state.
  extensions::AllowlistAcknowledgeState GetExtensionAllowlistAcknowledgeState(
      const extensions::ExtensionId& extension_id) const;

  // Sets the Safe Browsing allowlist acknowledge state.
  void SetExtensionAllowlistAcknowledgeState(const extensions::ExtensionId& extension_id,
                                             extensions::AllowlistAcknowledgeState state);

  // Performs action based on Omaha attributes for the extension.
  void PerformActionBasedOnOmahaAttributes(const extensions::ExtensionId& extension_id,
                                           const base::Value::Dict& attributes);

  // Whether a warning should be displayed for an extension, `true` if the
  // extension is not allowlisted and the allowlist is enforced.
  bool ShouldDisplayWarning(const extensions::ExtensionId& extension_id) const;

  // Informs the allowlist that a new extension was installed.
  //
  // `extension_id` is the id of the extension that was installed, and
  // `install_flags` is a bitmask of InstallFlags for the installation.
  void OnExtensionInstalled(const extensions::ExtensionId& extension_id, int install_flags);

  // Whether warnings should be shown for extensions not included in the
  // allowlist (considers Enhanced Safe Browsing setting and finch feature).
  bool warnings_enabled() const { return warnings_enabled_; }

 private:
  // Set if the allowlist should be enforced or not.
  void SetAllowlistEnforcementFields();

  // Apply the allowlist enforcement by disabling a not allowlisted extension if
  // allowed by policy.
  void ApplyEnforcement(const extensions::ExtensionId& extension_id);

  // Blocklist all extensions with allowlist state `ALLOWLIST_NOT_ALLOWLISTED`.
  void ActivateAllowlistEnforcement();

  // Unblocklist all extensions with allowlist state
  // `ALLOWLIST_NOT_ALLOWLISTED`.
  void DeactivateAllowlistEnforcement();

  // Called when the 'Enhanced Safe Browsing' setting changes.
  void OnSafeBrowsingEnhancedChanged();

  // extensions::ExtensionPrefsObserver:
  // Observes extension state changes to set
  // `ALLOWLIST_ACKNOWLEDGE_ENABLED_BY_USER` when a not allowlisted extension is
  // re-enabled by the user.
  void OnExtensionStateChanged(const extensions::ExtensionId& extension_id,
                               bool is_now_enabled) override;

  void NotifyExtensionAllowlistWarningStateChanged(
      const extensions::ExtensionId& extension_id,
      bool show_warning);

  // Adds extension acknowledged events to Safe Browsing metrics collector for
  // further metrics logging. Called when a user decides to re-enable an
  // extension that is not on the allowlist.
  void ReportExtensionReEnabledEvent();

  base::ObserverList<Observer> observers_;

  raw_ptr<content::BrowserContext> browser_context_ = nullptr;
  raw_ptr<extensions::ExtensionPrefs> extension_prefs_ = nullptr;
  raw_ptr<ExtensionService> extension_service_ = nullptr;
  raw_ptr<extensions::ExtensionRegistry> registry_ = nullptr;

  bool init_done_ = false;

  // Specifies if warnings should be shown for extensions not included in the
  // allowlist for this profile (considers ESB setting and finch feature).
  bool warnings_enabled_ = false;

  // Specifies if extensions not included in the allowlist should be
  // automatically disabled on this profile (considers ESB setting and finch
  // feature).
  bool should_auto_disable_extensions_ = false;

  // Used to subscribe to profile preferences updates.
  PrefChangeRegistrar pref_change_registrar_;

  base::ScopedObservation<extensions::ExtensionPrefs,
                          extensions::ExtensionPrefsObserver>
      extension_prefs_observation_{this};
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_EXTENSION_ALLOWLIST_H_
