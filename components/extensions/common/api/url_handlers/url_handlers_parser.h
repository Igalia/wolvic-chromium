// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_COMMON_API_URL_HANDLERS_URL_HANDLERS_PARSER_H_
#define COMPONENTS_EXTENSIONS_COMMON_API_URL_HANDLERS_URL_HANDLERS_PARSER_H_

#include <string>
#include <vector>

#include "extensions/common/extension.h"
#include "extensions/common/manifest_handler.h"
#include "extensions/common/url_pattern.h"

class GURL;

namespace components_extensions {

struct UrlHandlerInfo {
  UrlHandlerInfo();

  UrlHandlerInfo(const UrlHandlerInfo&) = delete;
  UrlHandlerInfo& operator=(const UrlHandlerInfo&) = delete;

  UrlHandlerInfo(UrlHandlerInfo&& other);

  ~UrlHandlerInfo();

  // ID identifying this handler in the manifest.
  std::string id;
  // Handler title to display in all relevant UI.
  std::string title;
  // URL patterns associated with this handler.
  extensions::URLPatternSet patterns;
};

struct UrlHandlers : public extensions::Extension::ManifestData {
  UrlHandlers();
  ~UrlHandlers() override;

  // Returns an array of URL handlers |extension| has defined in its manifest.
  static const std::vector<UrlHandlerInfo>* GetUrlHandlers(
      const extensions::Extension* extension);

  // Determines whether |app| has at least one URL handler that matches
  // |url|.
  static bool CanPlatformAppHandleUrl(const extensions::Extension* app, const GURL& url);

  // Determines whether |app| has at least one URL handler that matches |url|.
  static bool CanBookmarkAppHandleUrl(const extensions::Extension* app, const GURL& url);

  // Finds a matching URL handler for |app|, if any. Returns nullptr if none
  // are found.
  static const UrlHandlerInfo* GetMatchingPlatformAppUrlHandler(
      const extensions::Extension* app,
      const GURL& url);

  std::vector<UrlHandlerInfo> handlers;
};

// Parses the "url_handlers" manifest key.
class UrlHandlersParser : public extensions::ManifestHandler {
 public:
  UrlHandlersParser();

  UrlHandlersParser(const UrlHandlersParser&) = delete;
  UrlHandlersParser& operator=(const UrlHandlersParser&) = delete;

  ~UrlHandlersParser() override;

  // ManifestHandler API
  bool Parse(extensions::Extension* extension, std::u16string* error) override;

 private:
  base::span<const char* const> Keys() const override;
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_COMMON_API_URL_HANDLERS_URL_HANDLERS_PARSER_H_
