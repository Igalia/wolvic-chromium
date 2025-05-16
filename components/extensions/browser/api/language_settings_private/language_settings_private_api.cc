// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/language_settings_private/language_settings_private_api.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "base/containers/contains.h"
#include "base/containers/flat_set.h"
#include "base/feature_list.h"
#include "base/ranges/algorithm.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "components/extensions/common/api/language_settings_private.h"
#include "components/language/core/browser/language_model_manager.h"
#include "components/language/core/browser/pref_names.h"
#include "components/language/core/common/language_util.h"
#include "components/language/core/common/locale_util.h"
#include "components/spellcheck/browser/spellcheck_platform.h"
#include "components/spellcheck/common/spellcheck_common.h"
#include "components/spellcheck/common/spellcheck_features.h"
#include "components/spellcheck/spellcheck_buildflags.h"
#include "components/translate/core/browser/translate_download_manager.h"
#include "components/translate/core/browser/translate_prefs.h"
#include "third_party/icu/source/i18n/unicode/coll.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/l10n/l10n_util_collator.h"

namespace extensions {

namespace language_settings_private = api::language_settings_private;

LanguageSettingsPrivateGetLanguageListFunction::
    LanguageSettingsPrivateGetLanguageListFunction() = default;

LanguageSettingsPrivateGetLanguageListFunction::
    ~LanguageSettingsPrivateGetLanguageListFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateGetLanguageListFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateEnableLanguageFunction::
    LanguageSettingsPrivateEnableLanguageFunction() = default;

LanguageSettingsPrivateEnableLanguageFunction::
    ~LanguageSettingsPrivateEnableLanguageFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateEnableLanguageFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateDisableLanguageFunction::
    LanguageSettingsPrivateDisableLanguageFunction() = default;

LanguageSettingsPrivateDisableLanguageFunction::
    ~LanguageSettingsPrivateDisableLanguageFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateDisableLanguageFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateSetEnableTranslationForLanguageFunction::
    LanguageSettingsPrivateSetEnableTranslationForLanguageFunction() = default;

LanguageSettingsPrivateSetEnableTranslationForLanguageFunction::
    ~LanguageSettingsPrivateSetEnableTranslationForLanguageFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateSetEnableTranslationForLanguageFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateGetAlwaysTranslateLanguagesFunction::
    LanguageSettingsPrivateGetAlwaysTranslateLanguagesFunction() = default;

LanguageSettingsPrivateGetAlwaysTranslateLanguagesFunction::
    ~LanguageSettingsPrivateGetAlwaysTranslateLanguagesFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateGetAlwaysTranslateLanguagesFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateSetLanguageAlwaysTranslateStateFunction::
    LanguageSettingsPrivateSetLanguageAlwaysTranslateStateFunction() = default;

LanguageSettingsPrivateSetLanguageAlwaysTranslateStateFunction::
    ~LanguageSettingsPrivateSetLanguageAlwaysTranslateStateFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateSetLanguageAlwaysTranslateStateFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateGetNeverTranslateLanguagesFunction::
    LanguageSettingsPrivateGetNeverTranslateLanguagesFunction() = default;

LanguageSettingsPrivateGetNeverTranslateLanguagesFunction::
    ~LanguageSettingsPrivateGetNeverTranslateLanguagesFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateGetNeverTranslateLanguagesFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateMoveLanguageFunction::
    LanguageSettingsPrivateMoveLanguageFunction() = default;

LanguageSettingsPrivateMoveLanguageFunction::
    ~LanguageSettingsPrivateMoveLanguageFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateMoveLanguageFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateGetSpellcheckDictionaryStatusesFunction::
    LanguageSettingsPrivateGetSpellcheckDictionaryStatusesFunction() = default;

LanguageSettingsPrivateGetSpellcheckDictionaryStatusesFunction::
    ~LanguageSettingsPrivateGetSpellcheckDictionaryStatusesFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateGetSpellcheckDictionaryStatusesFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateGetSpellcheckWordsFunction::
    LanguageSettingsPrivateGetSpellcheckWordsFunction() = default;

LanguageSettingsPrivateGetSpellcheckWordsFunction::
    ~LanguageSettingsPrivateGetSpellcheckWordsFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateGetSpellcheckWordsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateAddSpellcheckWordFunction::
    LanguageSettingsPrivateAddSpellcheckWordFunction() = default;

LanguageSettingsPrivateAddSpellcheckWordFunction::
    ~LanguageSettingsPrivateAddSpellcheckWordFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateAddSpellcheckWordFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateRemoveSpellcheckWordFunction::
    LanguageSettingsPrivateRemoveSpellcheckWordFunction() = default;

LanguageSettingsPrivateRemoveSpellcheckWordFunction::
    ~LanguageSettingsPrivateRemoveSpellcheckWordFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateRemoveSpellcheckWordFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateGetTranslateTargetLanguageFunction::
    LanguageSettingsPrivateGetTranslateTargetLanguageFunction() = default;

LanguageSettingsPrivateGetTranslateTargetLanguageFunction::
    ~LanguageSettingsPrivateGetTranslateTargetLanguageFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateGetTranslateTargetLanguageFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateSetTranslateTargetLanguageFunction::
    LanguageSettingsPrivateSetTranslateTargetLanguageFunction() = default;

LanguageSettingsPrivateSetTranslateTargetLanguageFunction::
    ~LanguageSettingsPrivateSetTranslateTargetLanguageFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateSetTranslateTargetLanguageFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateGetInputMethodListsFunction::
    LanguageSettingsPrivateGetInputMethodListsFunction() = default;

LanguageSettingsPrivateGetInputMethodListsFunction::
    ~LanguageSettingsPrivateGetInputMethodListsFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateGetInputMethodListsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateAddInputMethodFunction::
    LanguageSettingsPrivateAddInputMethodFunction() = default;

LanguageSettingsPrivateAddInputMethodFunction::
    ~LanguageSettingsPrivateAddInputMethodFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateAddInputMethodFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateRemoveInputMethodFunction::
    LanguageSettingsPrivateRemoveInputMethodFunction() = default;

LanguageSettingsPrivateRemoveInputMethodFunction::
    ~LanguageSettingsPrivateRemoveInputMethodFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateRemoveInputMethodFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

LanguageSettingsPrivateRetryDownloadDictionaryFunction::
    LanguageSettingsPrivateRetryDownloadDictionaryFunction() = default;

LanguageSettingsPrivateRetryDownloadDictionaryFunction::
    ~LanguageSettingsPrivateRetryDownloadDictionaryFunction() = default;

ExtensionFunction::ResponseAction
LanguageSettingsPrivateRetryDownloadDictionaryFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
