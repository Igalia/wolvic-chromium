// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/wolvic_content_client.h"

#include "components/cdm/common/android_cdm_registration.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "wolvic/browser/media/wolvic_media_drm_bridge_client.h"

#include "components/extensions/common/buildflags.h"
#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
#include "extensions/common/constants.h"
#endif

namespace wolvic {

// Copied from //content/shell/common/shell_content_client.cc

WolvicContentClient::WolvicContentClient() {}

WolvicContentClient::~WolvicContentClient() {}

std::u16string WolvicContentClient::GetLocalizedString(int message_id) {
  return l10n_util::GetStringUTF16(message_id);
}

base::StringPiece WolvicContentClient::GetDataResource(
    int resource_id,
    ui::ResourceScaleFactor scale_factor) {
  return ui::ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(
      resource_id, scale_factor);
}

base::RefCountedMemory* WolvicContentClient::GetDataResourceBytes(
    int resource_id) {
  return ui::ResourceBundle::GetSharedInstance().LoadDataResourceBytes(
      resource_id);
}

std::string WolvicContentClient::GetDataResourceString(int resource_id) {
  return ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(
      resource_id);
}

gfx::Image& WolvicContentClient::GetNativeImageNamed(int resource_id) {
  return ui::ResourceBundle::GetSharedInstance().GetNativeImageNamed(
      resource_id);
}

void WolvicContentClient::AddAdditionalSchemes(Schemes* schemes) {
  schemes->local_schemes.push_back(url::kContentScheme);


#if BUILDFLAG(ENABLE_EXTENSIONS_IN_COMPONENTS)
  schemes->standard_schemes.push_back(extensions::kExtensionScheme);
  schemes->extension_schemes.push_back(extensions::kExtensionScheme);
  schemes->savable_schemes.push_back(extensions::kExtensionScheme);
  // Treat extensions as secure because communication with them is entirely in
  // the browser, so there is no danger of manipulation or eavesdropping on
  // communication with them by third parties.
  schemes->secure_schemes.push_back(extensions::kExtensionScheme);

  schemes->service_worker_schemes.push_back(extensions::kExtensionScheme);
  schemes->service_worker_schemes.push_back(url::kFileScheme);

  // As far as Blink is concerned, they should be allowed to receive CORS
  // requests. At the Extensions layer, requests will actually be blocked unless
  // overridden by the web_accessible_resources manifest key.
  // TODO(kalman): See what happens with a service worker.
  schemes->cors_enabled_schemes.push_back(extensions::kExtensionScheme);
  schemes->csp_bypassing_schemes.push_back(extensions::kExtensionScheme);
#endif
}

void WolvicContentClient::AddContentDecryptionModules(
    std::vector<content::CdmInfo>* cdms,
    std::vector<media::CdmHostFilePath>* cdm_host_file_paths) {
  cdm::AddAndroidWidevineCdm(cdms);
  cdm::AddOtherAndroidCdms(cdms);
}

media::MediaDrmBridgeClient* WolvicContentClient::GetMediaDrmBridgeClient() {
  // This is stored as a global variable in browser_main_loop, so we don't need
  // to manage this pointer.
  return new WolvicMediaDrmBridgeClient();
}

}  // namespace wolvic
