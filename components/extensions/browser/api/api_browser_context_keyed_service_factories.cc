// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/api_browser_context_keyed_service_factories.h"

namespace components_extensions {

void EnsureApiBrowserContextKeyedServiceFactoriesBuilt() {
// TODO(mshin): Enable the below code whenever migrating each API
//   extensions::ActivityLogAPI::GetFactoryInstance();
//   extensions::AutofillPrivateEventRouterFactory::GetInstance();
//   extensions::BluetoothLowEnergyAPI::GetFactoryInstance();
//   extensions::BookmarksAPI::GetFactoryInstance();
//   extensions::BookmarkManagerPrivateAPI::GetFactoryInstance();
//   extensions::BrailleDisplayPrivateAPI::GetFactoryInstance();
//   extensions::CommandService::GetFactoryInstance();
//   extensions::CookiesAPI::GetFactoryInstance();
//   extensions::DeveloperPrivateAPI::GetFactoryInstance();
//   extensions::ExtensionActionAPI::GetFactoryInstance();
//   extensions::FontSettingsAPI::GetFactoryInstance();
//   extensions::HistoryAPI::GetFactoryInstance();
//   extensions::IdentityAPI::GetFactoryInstance();
//   extensions::LanguageSettingsPrivateDelegateFactory::GetInstance();
// #if BUILDFLAG(IS_CHROMEOS_ASH) || BUILDFLAG(IS_WIN) || BUILDFLAG(IS_MAC)
//   auto networking_private_ui_delegate_factory =
//       std::make_unique<extensions::NetworkingPrivateUIDelegateFactoryImpl>();
//   extensions::NetworkingPrivateDelegateFactory::GetInstance()
//       ->SetUIDelegateFactory(std::move(networking_private_ui_delegate_factory));
// #endif
//   extensions::OmniboxAPI::GetFactoryInstance();
//   extensions::PasswordsPrivateDelegateFactory::GetInstance();
//   extensions::PasswordsPrivateEventRouterFactory::GetInstance();
// #if BUILDFLAG(ENABLE_PDF) && BUILDFLAG(ENABLE_SCREEN_AI_SERVICE)
//   extensions::PdfViewerPrivateEventRouterFactory::GetInstance();
// #endif  // BUILDFLAG(ENABLE_PDF) && BUILDFLAG(ENABLE_SCREEN_AI_SERVICE)
//   extensions::PreferenceAPI::GetFactoryInstance();
//   extensions::ProcessesAPI::GetFactoryInstance();
//   extensions::ReadingListEventRouter::GetFactoryInstance();
//   extensions::SafeBrowsingPrivateEventRouterFactory::GetInstance();
//   extensions::SessionsAPI::GetFactoryInstance();
//   extensions::SettingsPrivateEventRouterFactory::GetInstance();
//   extensions::SettingsOverridesAPI::GetFactoryInstance();
//   extensions::SidePanelService::GetFactoryInstance();
// #if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_MAC) || BUILDFLAG(IS_WIN)
//   extensions::SystemIndicatorManagerFactory::GetInstance();
// #endif
//   extensions::TabGroupsEventRouterFactory::GetInstance();
//   extensions::TabCaptureRegistry::GetFactoryInstance();
//   extensions::TabsWindowsAPI::GetFactoryInstance();
//   extensions::TtsAPI::GetFactoryInstance();
//   extensions::WebAuthenticationProxyAPI::GetFactoryInstance();
//   extensions::WebNavigationAPI::GetFactoryInstance();
//   extensions::WebrtcAudioPrivateEventService::GetFactoryInstance();
}

}  // namespace components_extensions
