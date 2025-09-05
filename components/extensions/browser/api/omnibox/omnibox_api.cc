// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/omnibox/omnibox_api.h"

#include <stddef.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/functional/bind.h"
#include "base/lazy_instance.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "components/extensions/common/api/omnibox.h"
#include "components/omnibox/browser/omnibox_input_watcher.h"
#include "components/omnibox/browser/omnibox_suggestions_watcher.h"
#include "components/search_engines/template_url.h"
#include "components/search_engines/template_url_service.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/common/extension_id.h"
#include "ui/gfx/image/image.h"

namespace extensions {

namespace omnibox = api::omnibox;
namespace SendSuggestions = omnibox::SendSuggestions;
namespace SetDefaultSuggestion = omnibox::SetDefaultSuggestion;

namespace {

const char kSuggestionContent[] = "content";
const char kCurrentTabDisposition[] = "currentTab";
const char kForegroundTabDisposition[] = "newForegroundTab";
const char kBackgroundTabDisposition[] = "newBackgroundTab";

// Pref key for omnibox.setDefaultSuggestion.
const char kOmniboxDefaultSuggestion[] = "omnibox_default_suggestion";

std::optional<omnibox::SuggestResult> GetOmniboxDefaultSuggestion(
    content::BrowserContext* context,
    const ExtensionId& extension_id) {
  ExtensionPrefs* prefs = ExtensionPrefs::Get(context);
  if (!prefs) {
    return std::nullopt;
  }

  const base::Value::Dict* dict =
      prefs->ReadPrefAsDict(extension_id, kOmniboxDefaultSuggestion);
  if (!dict) {
    return std::nullopt;
  }
  return omnibox::SuggestResult::FromValue(*dict);
}

// Tries to set the omnibox default suggestion; returns true on success or
// false on failure.
bool SetOmniboxDefaultSuggestion(
    content::BrowserContext* context,
    const ExtensionId& extension_id,
    const omnibox::DefaultSuggestResult& suggestion) {
  ExtensionPrefs* prefs = ExtensionPrefs::Get(context);
  if (!prefs)
    return false;

  base::Value::Dict dict = suggestion.ToValue();
  // Add the content field so that the dictionary can be used to populate an
  // omnibox::SuggestResult.
  dict.Set(kSuggestionContent, base::Value(base::Value::Type::STRING));
  prefs->UpdateExtensionPref(extension_id, kOmniboxDefaultSuggestion,
                             base::Value(std::move(dict)));

  return true;
}

// Returns a string used as a template URL string of the extension.
// TODO(mshin): Enable the below code after migrating OmniboxInfo
// std::string GetTemplateURLStringForExtension(const ExtensionId& extension_id) {
//   // This URL is not actually used for navigation. It holds the extension's ID.
//   return std::string(extensions::kExtensionScheme) + "://" +
//       extension_id + "/?q={searchTerms}";
// }

}  // namespace

// static
void ExtensionOmniboxEventRouter::OnInputStarted(
    content::BrowserContext* context,
    const ExtensionId& extension_id) {
  auto event = std::make_unique<Event>(events::OMNIBOX_ON_INPUT_STARTED,
                                       omnibox::OnInputStarted::kEventName,
                                       base::Value::List(), context);
  EventRouter::Get(context)
      ->DispatchEventToExtension(extension_id, std::move(event));
}

// static
bool ExtensionOmniboxEventRouter::OnInputChanged(
    content::BrowserContext* context,
    const ExtensionId& extension_id,
    const std::string& input,
    int suggest_id) {
  EventRouter* event_router = EventRouter::Get(context);
  if (!event_router->ExtensionHasEventListener(
          extension_id, omnibox::OnInputChanged::kEventName))
    return false;

  base::Value::List args;
  args.Append(input);
  args.Append(suggest_id);

  auto event = std::make_unique<Event>(events::OMNIBOX_ON_INPUT_CHANGED,
                                       omnibox::OnInputChanged::kEventName,
                                       std::move(args), context);
  event_router->DispatchEventToExtension(extension_id, std::move(event));
  return true;
}

// static
void ExtensionOmniboxEventRouter::OnInputEntered(
    content::WebContents* web_contents,
    const ExtensionId& extension_id,
    const std::string& input,
    WindowOpenDisposition disposition) {
  content::BrowserContext* context = web_contents->GetBrowserContext();
  const Extension* extension =
      ExtensionRegistry::Get(context)->enabled_extensions().GetByID(
          extension_id);
  CHECK(extension);
  // TODO(mshin): Enable the below code after migrating TabHelper
  // extensions::TabHelper::FromWebContents(web_contents)->
  //     active_tab_permission_granter()->GrantIfRequested(extension);

  base::Value::List args;
  args.Append(input);
  if (disposition == WindowOpenDisposition::NEW_FOREGROUND_TAB)
    args.Append(kForegroundTabDisposition);
  else if (disposition == WindowOpenDisposition::NEW_BACKGROUND_TAB)
    args.Append(kBackgroundTabDisposition);
  else
    args.Append(kCurrentTabDisposition);

  auto event = std::make_unique<Event>(events::OMNIBOX_ON_INPUT_ENTERED,
                                       omnibox::OnInputEntered::kEventName,
                                       std::move(args), context);
  EventRouter::Get(context)
      ->DispatchEventToExtension(extension_id, std::move(event));

  OmniboxInputWatcher::GetForBrowserContext(context)->NotifyInputEntered();
}

// static
void ExtensionOmniboxEventRouter::OnInputCancelled(
    content::BrowserContext* context,
    const ExtensionId& extension_id) {
  auto event = std::make_unique<Event>(events::OMNIBOX_ON_INPUT_CANCELLED,
                                       omnibox::OnInputCancelled::kEventName,
                                       base::Value::List(), context);
  EventRouter::Get(context)
      ->DispatchEventToExtension(extension_id, std::move(event));
}

void ExtensionOmniboxEventRouter::OnDeleteSuggestion(
    content::BrowserContext* context,
    const ExtensionId& extension_id,
    const std::string& suggestion_text) {
  base::Value::List args;
  args.Append(suggestion_text);

  auto event = std::make_unique<Event>(events::OMNIBOX_ON_DELETE_SUGGESTION,
                                       omnibox::OnDeleteSuggestion::kEventName,
                                       std::move(args), context);

  EventRouter::Get(context)->DispatchEventToExtension(extension_id,
                                                      std::move(event));
}

OmniboxAPI::OmniboxAPI(content::BrowserContext* context)
    // TODO(mshin): Support TemplateURLServiceFactory
    /* : url_service_(TemplateURLServiceFactory::GetForProfile(context)) */ {
  extension_registry_observation_.Observe(ExtensionRegistry::Get(context));
  if (url_service_) {
    template_url_subscription_ =
        url_service_->RegisterOnLoadedCallback(base::BindOnce(
            &OmniboxAPI::OnTemplateURLsLoaded, base::Unretained(this)));
  }

  // Use monochrome icons for Omnibox icons.
  // omnibox_icon_manager_.set_monochrome(true);
}

void OmniboxAPI::Shutdown() {
  template_url_subscription_ = {};
}

OmniboxAPI::~OmniboxAPI() {
}

static base::LazyInstance<BrowserContextKeyedAPIFactory<OmniboxAPI>>::
    DestructorAtExit g_omnibox_api_factory = LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<OmniboxAPI>* OmniboxAPI::GetFactoryInstance() {
  return g_omnibox_api_factory.Pointer();
}

// static
OmniboxAPI* OmniboxAPI::Get(content::BrowserContext* context) {
  return BrowserContextKeyedAPIFactory<OmniboxAPI>::Get(context);
}

void OmniboxAPI::OnExtensionLoaded(content::BrowserContext* browser_context,
                                   const Extension* extension) {
  // TODO(mshin): Enable the below code after migrating OmniboxInfo
  // const std::string& keyword = OmniboxInfo::GetKeyword(extension);
  // if (!keyword.empty()) {
  //   // Load the omnibox icon so it will be ready to display in the URL bar.
  //   // omnibox_icon_manager_.LoadIcon(browser_context, extension);
  //   if (url_service_) {
  //     url_service_->Load();
  //     if (url_service_->loaded()) {
  //       url_service_->RegisterOmniboxKeyword(
  //           extension->id(), extension->short_name(), keyword,
  //           GetTemplateURLStringForExtension(extension->id()),
  //           ExtensionPrefs::Get(browser_context_)->GetLastUpdateTime(extension->id()));
  //     } else {
  //       pending_extensions_.insert(extension);
  //     }
  //   }
  // }
}

void OmniboxAPI::OnExtensionUnloaded(content::BrowserContext* browser_context,
                                     const Extension* extension,
                                     UnloadedExtensionReason reason) {
  // TODO(mshin): Enable the below code after migrating OmniboxInfo
  // if (!OmniboxInfo::GetKeyword(extension).empty() && url_service_) {
  //   if (url_service_->loaded()) {
  //     url_service_->RemoveExtensionControlledTURL(
  //         extension->id(), TemplateURL::OMNIBOX_API_EXTENSION);
  //   } else {
  //     pending_extensions_.erase(extension);
  //   }
  // }
}

gfx::Image OmniboxAPI::GetOmniboxIcon(const ExtensionId& extension_id) {
  // return omnibox_icon_manager_.GetIcon(extension_id);
  return gfx::Image();
}

void OmniboxAPI::OnTemplateURLsLoaded() {
  // Register keywords for pending extensions.
  template_url_subscription_ = {};
  // TODO(mshin): Enable the below code after migrating OmniboxInfo
  // for (const Extension* i : pending_extensions_) {
  //   url_service_->RegisterOmniboxKeyword(
  //       i->id(), i->short_name(), OmniboxInfo::GetKeyword(i),
  //       GetTemplateURLStringForExtension(i->id()),
  //       ExtensionPrefs::Get(browser_context_)->GetLastUpdateTime(i->id()));
  // }
  pending_extensions_.clear();
}

template <>
void BrowserContextKeyedAPIFactory<OmniboxAPI>::DeclareFactoryDependencies() {
  DependsOn(ExtensionsBrowserClient::Get()->GetExtensionSystemFactory());
  DependsOn(ExtensionPrefsFactory::GetInstance());
  // DependsOn(TemplateURLServiceFactory::GetInstance());
}

OmniboxSendSuggestionsFunction::OmniboxSendSuggestionsFunction() = default;
OmniboxSendSuggestionsFunction::~OmniboxSendSuggestionsFunction() = default;

ExtensionFunction::ResponseAction OmniboxSendSuggestionsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void OmniboxSendSuggestionsFunction::NotifySuggestionsReady() {
  OmniboxSuggestionsWatcher::GetForBrowserContext(browser_context())
      ->NotifySuggestionsReady(&*params_);
}

ExtensionFunction::ResponseAction OmniboxSetDefaultSuggestionFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void OmniboxSetDefaultSuggestionFunction::SetDefaultSuggestion(
    const omnibox::DefaultSuggestResult& suggestion) {
  if (SetOmniboxDefaultSuggestion(browser_context(), extension_id(), suggestion)) {
    OmniboxSuggestionsWatcher::GetForBrowserContext(
        browser_context())
        ->NotifyDefaultSuggestionChanged();
  }
}

// This function converts style information populated by the JSON schema
// compiler into an ACMatchClassifications object.
ACMatchClassifications StyleTypesToACMatchClassifications(
    const omnibox::SuggestResult &suggestion) {
  ACMatchClassifications match_classifications;
  if (suggestion.description_styles) {
    std::u16string description = base::UTF8ToUTF16(suggestion.description);
    std::vector<int> styles(description.length(), 0);

    for (const omnibox::MatchClassification& style :
         *suggestion.description_styles) {
      int length = style.length ? *style.length : description.length();
      size_t offset = style.offset >= 0
                          ? style.offset
                          : std::max(0, static_cast<int>(description.length()) +
                                            style.offset);

      int type_class;
      switch (style.type) {
        case omnibox::DescriptionStyleType::kUrl:
          type_class = AutocompleteMatch::ACMatchClassification::URL;
          break;
        case omnibox::DescriptionStyleType::kMatch:
          type_class = AutocompleteMatch::ACMatchClassification::MATCH;
          break;
        case omnibox::DescriptionStyleType::kDim:
          type_class = AutocompleteMatch::ACMatchClassification::DIM;
          break;
        default:
          type_class = AutocompleteMatch::ACMatchClassification::NONE;
          return match_classifications;
      }

      for (size_t j = offset; j < offset + length && j < styles.size(); ++j)
        styles[j] |= type_class;
    }

    for (size_t i = 0; i < styles.size(); ++i) {
      if (i == 0 || styles[i] != styles[i-1])
        match_classifications.push_back(
            ACMatchClassification(i, styles[i]));
    }
  } else {
    match_classifications.push_back(
        ACMatchClassification(0, ACMatchClassification::NONE));
  }

  return match_classifications;
}

void ApplyDefaultSuggestionForExtensionKeyword(
    content::BrowserContext* context,
    const TemplateURL* keyword,
    const std::u16string& remaining_input,
    AutocompleteMatch* match) {
  DCHECK(keyword->type() == TemplateURL::OMNIBOX_API_EXTENSION);

  std::optional<omnibox::SuggestResult> suggestion(
      GetOmniboxDefaultSuggestion(context, keyword->GetExtensionId()));
  if (!suggestion || suggestion->description.empty())
    return;  // fall back to the universal default

  const std::u16string kPlaceholderText(u"%s");
  const std::u16string kReplacementText(u"<input>");

  std::u16string description = base::UTF8ToUTF16(suggestion->description);
  ACMatchClassifications& description_styles = match->contents_class;
  description_styles = StyleTypesToACMatchClassifications(*suggestion);

  // Replace "%s" with the user's input and adjust the style offsets to the
  // new length of the description.
  size_t placeholder(description.find(kPlaceholderText, 0));
  if (placeholder != std::u16string::npos) {
    std::u16string replacement =
        remaining_input.empty() ? kReplacementText : remaining_input;
    description.replace(placeholder, kPlaceholderText.length(), replacement);

    for (auto& description_style : description_styles) {
      if (description_style.offset > placeholder)
        description_style.offset += replacement.length() - 2;
    }
  }

  match->contents.assign(description);
}

}  // namespace extensions
