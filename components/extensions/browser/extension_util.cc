// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/extension_util.h"

#include <vector>

#include "base/check_is_test.h"
#include "base/check_op.h"
#include "base/command_line.h"
#include "build/build_config.h"
#include "components/extensions/browser/extension_management.h"
#include "components/extensions/browser/extension_service.h"
#include "components/extensions/browser/permissions_updater.h"
#include "components/extensions/browser/shared_module_service.h"
#include "components/prefs/pref_service.h"
#include "components/variations/variations_associated_data.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/site_instance.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_client.h"
#include "extensions/browser/disable_reason.h"
#include "extensions/browser/extension_host.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/extension_util.h"
#include "extensions/browser/pref_names.h"
#include "extensions/browser/process_manager.h"
#include "extensions/browser/renderer_startup_helper.h"
#include "extensions/browser/user_script_manager.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_icon_set.h"
#include "extensions/common/extension_urls.h"
#include "extensions/common/features/feature_developer_mode_only.h"
#include "extensions/common/manifest_handlers/incognito_info.h"
#include "extensions/common/manifest_handlers/permissions_parser.h"
#include "extensions/common/permissions/permissions_data.h"
#include "url/gurl.h"

using extensions::Extension;
using extensions::ExtensionHost;
using extensions::ExtensionId;
using extensions::ExtensionPrefs;
using extensions::ExtensionRegistry;
using extensions::ExtensionSet;
using extensions::ExtensionSystem;
using extensions::PermissionsParser;
using extensions::PermissionSet;
using extensions::ProcessManager;
using extensions::RendererStartupHelperFactory;
using extensions::UserScript;
using extensions::UserScriptManager;

namespace components_extensions {
namespace util {

namespace {

// Returns |extension_id|. See note below.
std::string ReloadExtension(const std::string& extension_id,
                            content::BrowserContext* context) {
  // When we reload the extension the ID may be invalidated if we've passed it
  // by const ref everywhere. Make a copy to be safe. http://crbug.com/103762
  std::string id = extension_id;
  ExtensionService* service =
      ExtensionSystem::Get(context)->extension_service();
  CHECK(service);
  service->ReloadExtension(id);
  return id;
}

std::string ReloadExtensionIfEnabled(const std::string& extension_id,
                                     content::BrowserContext* context) {
  ExtensionRegistry* registry = ExtensionRegistry::Get(context);
  bool extension_is_enabled =
      registry->enabled_extensions().Contains(extension_id);

  if (!extension_is_enabled) {
    return extension_id;
  }
  return ReloadExtension(extension_id, context);
}

}  // namespace

bool HasIsolatedStorage(const ExtensionId& extension_id,
                        content::BrowserContext* context) {
  const Extension* extension =
      ExtensionRegistry::Get(context)->GetInstalledExtension(extension_id);
  // Extension is null when the extension is cleaned up after it's unloaded and
  // won't be present in the ExtensionRegistry.
  return extension ? HasIsolatedStorage(*extension, context) : false;
}

bool HasIsolatedStorage(const Extension& extension,
                        content::BrowserContext* context) {

  return extension.is_platform_app();
}

void SetIsIncognitoEnabled(const std::string& extension_id,
                           content::BrowserContext* context,
                           bool enabled) {
  ExtensionRegistry* registry = ExtensionRegistry::Get(context);
  const Extension* extension =
      registry->GetExtensionById(extension_id, ExtensionRegistry::EVERYTHING);

  if (extension) {
    if (!extensions::util::CanBeIncognitoEnabled(extension))
      return;

    // TODO(treib,kalman): Should this be Manifest::IsComponentLocation(..)?
    // (which also checks for kExternalComponent).
    if (extension->location() == extensions::mojom::ManifestLocation::kComponent) {
      // This shouldn't be called for component extensions unless it is called
      // by sync, for syncable component extensions.
      // See http://crbug.com/112290 and associated CLs for the sordid history.
      // TODO(mshin): Enable the below code after supporting sync
      // bool syncable = sync_helper::IsSyncableComponentExtension(extension);
      // DCHECK(syncable);

      // If we are here, make sure the we aren't trying to change the value.
      DCHECK_EQ(enabled, extensions::util::IsIncognitoEnabled(extension_id, context));
      return;
    }
  }

  ExtensionPrefs* extension_prefs = ExtensionPrefs::Get(context);
  // Broadcast unloaded and loaded events to update browser state. Only bother
  // if the value changed and the extension is actually enabled, since there is
  // no UI otherwise.
  bool old_enabled = extension_prefs->IsIncognitoEnabled(extension_id);
  if (enabled == old_enabled)
    return;

  extension_prefs->SetIsIncognitoEnabled(extension_id, enabled);

  std::string id = ReloadExtensionIfEnabled(extension_id, context);

  // Reloading the extension invalidates the |extension| pointer.
  // TODO(mshin): Enable the below code after supporting Sync
  // extension = registry->GetExtensionById(id, ExtensionRegistry::EVERYTHING);
  // if (extension) {
  //   ExtensionSyncService::Get(context)->SyncExtensionChangeIfNeeded(*extension);
  // }
}

void SetAllowFileAccess(const std::string& extension_id,
                        content::BrowserContext* context,
                        bool allow) {
  // Reload to update browser state if the value changed. We need to reload even
  // if the extension is disabled, in order to make sure file access is
  // reinitialized correctly.
  if (allow == extensions::util::AllowFileAccess(extension_id, context)) {
    return;
  }

  ExtensionPrefs::Get(context)->SetAllowFileAccess(extension_id, allow);

  ReloadExtension(extension_id, context);
}

bool ShouldSync(const Extension* extension,
                content::BrowserContext* context) {
  ExtensionManagement* extension_management =
      ExtensionManagementFactory::GetForBrowserContext(context);
  // Update URL is overridden only for non webstore extensions and offstore
  // extensions should not be synced.
  if (extension_management->IsUpdateUrlOverridden(extension->id())) {
    const GURL update_url =
        extension_management->GetEffectiveUpdateURL(*extension);
    DCHECK(!extension_urls::IsWebstoreUpdateUrl(update_url))
        << "Update URL cannot be overridden to be the webstore URL!";
    return false;
  }

  // TODO(mshin): Enable the below code after supporting sync
  // return sync_helper::IsSyncable(extension) &&
  //        !ExtensionPrefs::Get(context)->DoNotSync(extension->id());
  return !ExtensionPrefs::Get(context)->DoNotSync(extension->id());
}

bool IsExtensionIdle(const std::string& extension_id,
                     content::BrowserContext* context) {
  std::vector<std::string> ids_to_check;
  ids_to_check.push_back(extension_id);

  const Extension* extension =
      ExtensionRegistry::Get(context)->enabled_extensions().GetByID(
          extension_id);
  if (extension && extension->is_shared_module()) {
    // We have to check all the extensions that use this shared module for idle
    // to tell whether it is really 'idle'.
    SharedModuleService* service = ExtensionSystem::Get(context)
                                       ->extension_service()
                                       ->shared_module_service();
    std::unique_ptr<ExtensionSet> dependents =
        service->GetDependentExtensions(extension);
    for (ExtensionSet::const_iterator i = dependents->begin();
         i != dependents->end();
         i++) {
      ids_to_check.push_back((*i)->id());
    }
  }

  ProcessManager* process_manager = ProcessManager::Get(context);
  for (std::vector<std::string>::const_iterator i = ids_to_check.begin();
       i != ids_to_check.end();
       i++) {
    const std::string id = (*i);
    ExtensionHost* host = process_manager->GetBackgroundHostForExtension(id);
    if (host)
      return false;

    scoped_refptr<content::SiteInstance> site_instance =
        process_manager->GetSiteInstanceForURL(
            Extension::GetBaseURLFromExtensionId(id));
    if (site_instance && site_instance->HasProcess())
      return false;

    if (!process_manager->GetRenderFrameHostsForExtension(id).empty())
      return false;
  }
  return true;
}

base::Value::Dict GetExtensionInfo(const Extension* extension) {
  DCHECK(extension);
  base::Value::Dict dict;

  dict.Set("id", extension->id());
  dict.Set("name", extension->name());

  // TODO(mshin): Enable the below code after migrating ExtensionIconSource
  // GURL icon = extensions::ExtensionIconSource::GetIconURL(
  //     extension, extension_misc::EXTENSION_ICON_SMALLISH,
  //     ExtensionIconSet::MATCH_BIGGER,
  //     false);  // Not grayscale.
  // dict.Set("icon", icon.spec());

  return dict;
}

std::unique_ptr<const PermissionSet> GetInstallPromptPermissionSetForExtension(
    const Extension* extension,
    content::BrowserContext* context,
    bool include_optional_permissions) {
  // Initialize permissions if they have not already been set so that
  // any transformations are correctly reflected in the install prompt.
  PermissionsUpdater(context, PermissionsUpdater::INIT_FLAG_TRANSIENT)
      .InitializePermissions(extension);

  std::unique_ptr<const PermissionSet> permissions_to_display =
      extension->permissions_data()->active_permissions().Clone();

  if (include_optional_permissions) {
    const PermissionSet& optional_permissions =
        PermissionsParser::GetOptionalPermissions(extension);
    permissions_to_display = PermissionSet::CreateUnion(*permissions_to_display,
                                                        optional_permissions);
  }
  return permissions_to_display;
}

std::vector<content::BrowserContext*> GetAllRelatedProfiles(
    content::BrowserContext* context,
    const Extension& extension) {
  std::vector<content::BrowserContext*> related_contexts;
  // Support only a single profile with off the recode profile

  related_contexts.push_back(context);

  // The returned `related_contexts` should include all the related incognito
  // profiles if the extension is globally allowed in incognito (this is a
  // global, rather than per-profile toggle - this is why we it can be checked
  // globally here, rather than once for every incognito profile looped over
  // below).
  if (extensions::util::IsIncognitoEnabled(extension.id(), context)) {
    content::BrowserContext* off_the_record_context =
        context->GetOTRBrowserContext();

    related_contexts.push_back(off_the_record_context);
  }

  return related_contexts;
}

void SetDeveloperModeForProfile(content::BrowserContext* context, bool in_developer_mode) {
  // TODO(mshin): Enable the below code after supporting preference.
  // context->GetPrefs()->SetBoolean(prefs::kExtensionsUIDeveloperMode,
  //                                 in_developer_mode);
  extensions::SetCurrentDeveloperMode(
      extensions::util::GetBrowserContextId(context), in_developer_mode);
  RendererStartupHelperFactory::GetForBrowserContext(context)
      ->OnDeveloperModeChanged(in_developer_mode);

  // kDynamicUserScript scripts are allowed if and only if the user is in dev
  // mode (since they allow raw code execution). Notify the user script manager
  // to properly enable or disable any scripts.
  UserScriptManager* user_script_manager =
      ExtensionSystem::Get(context)->user_script_manager();
  if (!user_script_manager) {
    CHECK_IS_TEST();  // `user_script_manager` can be null in unit tests.
    return;
  }

  user_script_manager->SetUserScriptSourceEnabledForExtensions(
      UserScript::Source::kDynamicUserScript, in_developer_mode);
}

void Navigate(
    const GURL& url,
    content::BrowserContext* context,
    content::WebContents* target_contents,
    WindowOpenDisposition disposition) {
  if (!target_contents) {
    // TODO(mshin): Plumbing the interface to Embbeder to get WebContents
    // target_contents = GetWebContents();
  }

  DCHECK(target_contents);

  content::NavigationController::LoadURLParams params(url);
  // TODO(mshin): Fill more params

  target_contents->GetController().LoadURLWithParams(params);
}

}  // namespace util
}  // namespace components_extensions
