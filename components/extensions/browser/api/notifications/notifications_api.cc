// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/notifications/notifications_api.h"

#include <stddef.h>

#include <utility>

#include "base/functional/callback.h"
#include "base/rand_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "base/uuid.h"
#include "components/keyed_service/content/browser_context_keyed_service_shutdown_notifier_factory.h"
#include "components/keyed_service/core/keyed_service_shutdown_notifier.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/app_window/app_window.h"
#include "extensions/browser/app_window/app_window_registry.h"
#include "extensions/browser/app_window/native_app_window.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_system_provider.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_id.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/base/resource/resource_scale_factor.h"
#include "ui/gfx/geometry/skia_conversions.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_skia_operations.h"
#include "ui/gfx/image/image_skia_rep.h"
#include "ui/message_center/public/cpp/message_center_constants.h"
#include "ui/message_center/public/cpp/notification.h"
#include "ui/message_center/public/cpp/notification_delegate.h"
#include "ui/message_center/public/cpp/notifier_id.h"
#include "url/gurl.h"

using message_center::NotifierId;

namespace extensions {

namespace notifications = api::notifications;

NotificationsApiFunction::NotificationsApiFunction() {
}

NotificationsApiFunction::~NotificationsApiFunction() {
}

bool NotificationsApiFunction::CanRunWhileDisabled() const {
  return false;
}

ExtensionFunction::ResponseAction NotificationsApiFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

NotificationsCreateFunction::NotificationsCreateFunction() {
}

NotificationsCreateFunction::~NotificationsCreateFunction() {
}

ExtensionFunction::ResponseAction
NotificationsCreateFunction::RunNotificationsApi() {
  return RespondNow(Error("Not Implement"));
}

NotificationsUpdateFunction::NotificationsUpdateFunction() {
}

NotificationsUpdateFunction::~NotificationsUpdateFunction() {
}

ExtensionFunction::ResponseAction
NotificationsUpdateFunction::RunNotificationsApi() {
  return RespondNow(Error("Not Implement"));
}

NotificationsClearFunction::NotificationsClearFunction() {
}

NotificationsClearFunction::~NotificationsClearFunction() {
}

ExtensionFunction::ResponseAction
NotificationsClearFunction::RunNotificationsApi() {
  return RespondNow(Error("Not Implement"));
}

NotificationsGetAllFunction::NotificationsGetAllFunction() {}

NotificationsGetAllFunction::~NotificationsGetAllFunction() {}

ExtensionFunction::ResponseAction
NotificationsGetAllFunction::RunNotificationsApi() {
  return RespondNow(Error("Not Implement"));
}

NotificationsGetPermissionLevelFunction::
NotificationsGetPermissionLevelFunction() {}

NotificationsGetPermissionLevelFunction::
~NotificationsGetPermissionLevelFunction() {}

bool NotificationsGetPermissionLevelFunction::CanRunWhileDisabled() const {
  return true;
}

ExtensionFunction::ResponseAction
NotificationsGetPermissionLevelFunction::RunNotificationsApi() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
