// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_COMMON_CHROME_EXTENSIONS_CLIENT_H_
#define COMPONENTS_EXTENSIONS_COMMON_CHROME_EXTENSIONS_CLIENT_H_

#include "base/memory/raw_ptr.h"
#include "components/extensions/common/permissions/chrome_permission_message_provider.h"
#include "extensions/common/extensions_client.h"
#include "url/gurl.h"

namespace components_extensions {

// The implementation of ExtensionsClient for Chrome, which encapsulates the
// global knowledge of features, permissions, and manifest fields.
class ChromeExtensionsClient : public extensions::ExtensionsClient {
 public:
  ChromeExtensionsClient();

  ChromeExtensionsClient(const ChromeExtensionsClient&) = delete;
  ChromeExtensionsClient& operator=(const ChromeExtensionsClient&) = delete;

  ~ChromeExtensionsClient() override;

  void Initialize() override;

  void InitializeWebStoreUrls(base::CommandLine* command_line) override;

  const extensions::PermissionMessageProvider& GetPermissionMessageProvider()
      const override;
  const std::string GetProductName() override;
  void FilterHostPermissions(const extensions::URLPatternSet& hosts,
                             extensions::URLPatternSet* new_hosts,
                             extensions::PermissionIDSet* permissions) const override;
  void SetScriptingAllowlist(const ScriptingAllowlist& allowlist) override;
  const ScriptingAllowlist& GetScriptingAllowlist() const override;
  extensions::URLPatternSet GetPermittedChromeSchemeHosts(
      const extensions::Extension* extension,
      const extensions::APIPermissionSet& api_permissions) const override;
  bool IsScriptableURL(const GURL& url, std::string* error) const override;
  const GURL& GetWebstoreBaseURL() const override;
  const GURL& GetNewWebstoreBaseURL() const override;
  const GURL& GetWebstoreUpdateURL() const override;
  bool IsBlocklistUpdateURL(const GURL& url) const override;
  std::set<base::FilePath> GetBrowserImagePaths(
      const extensions::Extension* extension) override;
  void AddOriginAccessPermissions(
      const extensions::Extension& extension,
      bool is_extension_active,
      std::vector<network::mojom::CorsOriginPatternPtr>* origin_patterns)
      const override;
  std::optional<int> GetExtensionExtendedErrorCode() const override;

 private:
  const ChromePermissionMessageProvider permission_message_provider_;

  // An allowlist of extensions that can script anywhere. Do not add to this
  // list (except in tests) without consulting the Extensions team first.
  // Note: Component extensions have this right implicitly and do not need to be
  // added to this list.
  ScriptingAllowlist scripting_allowlist_;

  GURL webstore_base_url_;
  GURL new_webstore_base_url_;
  GURL webstore_update_url_;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_COMMON_CHROME_EXTENSIONS_CLIENT_H_
