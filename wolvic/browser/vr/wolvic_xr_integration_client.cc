// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/browser/vr/wolvic_xr_integration_client.h"

#include "content/public/browser/xr_install_helper.h"
#include "wolvic/browser/vr/wvr_device_provider.h"

namespace wolvic {

namespace {

class WolvicInstallHelper : public content::XrInstallHelper {
 public:
  explicit WolvicInstallHelper() {}
  ~WolvicInstallHelper() override = default;
  WolvicInstallHelper(const WolvicInstallHelper&) = delete;
  WolvicInstallHelper& operator=(const WolvicInstallHelper&) = delete;

  // content::XrInstallHelper implementation.
  void EnsureInstalled(
      const content::GlobalRenderFrameHostId& frame_id,
      base::OnceCallback<void(content::XrInstallResult)> install_callback)
      override {
    std::move(install_callback)
        .Run(content::XrInstallResult::kSuccessAlreadyInstalled);
  }
};

}  // namespace

std::unique_ptr<content::XrInstallHelper>
WolvicXrIntegrationClient::GetInstallHelper(
    device::mojom::XRDeviceId device_id) {
  switch (device_id) {
    case device::mojom::XRDeviceId::WVR_DEVICE_ID:
      return std::make_unique<WolvicInstallHelper>();
    default:
      return nullptr;
  }
}

content::XRProviderList WolvicXrIntegrationClient::GetAdditionalProviders() {
  content::XRProviderList providers;
  providers.push_back(std::make_unique<WvrDeviceProvider>());

  return providers;
}

std::unique_ptr<content::BrowserXRRuntime::Observer>
WolvicXrIntegrationClient::CreateRuntimeObserver() {
  // TODO: Support AR
  return nullptr;
}

}  // namespace wolvic
