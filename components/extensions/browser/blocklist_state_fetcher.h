// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_BLOCKLIST_STATE_FETCHER_H_
#define COMPONENTS_EXTENSIONS_BROWSER_BLOCKLIST_STATE_FETCHER_H_

#include <algorithm>
#include <memory>
#include <set>
#include <string>

#include "base/functional/callback.h"
#include "base/memory/weak_ptr.h"
#include "components/safe_browsing/core/browser/db/util.h"
#include "extensions/browser/blocklist_state.h"

namespace network {
class SharedURLLoaderFactory;
class SimpleURLLoader;
}  // namespace network

namespace components_extensions {
class TestBlocklistStateFetcher;

class BlocklistStateFetcher {
 public:
  using RequestCallback = base::OnceCallback<void(extensions::BlocklistState)>;

  BlocklistStateFetcher();

  BlocklistStateFetcher(const BlocklistStateFetcher&) = delete;
  BlocklistStateFetcher& operator=(const BlocklistStateFetcher&) = delete;

  virtual ~BlocklistStateFetcher();

  virtual void Request(const std::string& id, RequestCallback callback);

  void SetSafeBrowsingConfig(const safe_browsing::V4ProtocolConfig& config);

 protected:
  void OnURLLoaderComplete(network::SimpleURLLoader* url_loader,
                           std::unique_ptr<std::string> response_body);

  // Used for ease unit tests.
  void OnURLLoaderCompleteInternal(network::SimpleURLLoader* url_loader,
                                   const std::string& response_body,
                                   int response_code,
                                   int net_error);

 private:
  friend class TestBlocklistStateFetcher;
  using CallbackMultiMap = std::multimap<std::string, RequestCallback>;

  GURL RequestUrl() const;

  void SendRequest(const std::string& id);

  std::unique_ptr<safe_browsing::V4ProtocolConfig> safe_browsing_config_;
  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;

  // SimpleURLLoader -> (owned loader, extension id).
  std::map<const network::SimpleURLLoader*,
           std::pair<std::unique_ptr<network::SimpleURLLoader>, std::string>>
      requests_;

  // Callbacks by extension ID.
  CallbackMultiMap callbacks_;

  base::WeakPtrFactory<BlocklistStateFetcher> weak_ptr_factory_{this};
};

}  // namespace components_extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_BLOCKLIST_STATE_FETCHER_H_
