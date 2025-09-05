// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/webrtc_logging_private/webrtc_logging_private_api.h"

#include <memory>
#include <utility>

#include "base/check_op.h"
#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/hash/hash.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/supports_user_data.h"
#include "build/build_config.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/site_instance.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/guest_view/web_view/web_view_guest.h"
#include "extensions/browser/process_manager.h"
#include "extensions/common/error_utils.h"

namespace extensions {

using api::webrtc_logging_private::MetaDataEntry;
using content::BrowserThread;

namespace Discard = api::webrtc_logging_private::Discard;
namespace SetMetaData = api::webrtc_logging_private::SetMetaData;
namespace SetUploadOnRenderClose =
    api::webrtc_logging_private::SetUploadOnRenderClose;
namespace Start = api::webrtc_logging_private::Start;
namespace StartRtpDump = api::webrtc_logging_private::StartRtpDump;
namespace Stop = api::webrtc_logging_private::Stop;
namespace StopRtpDump = api::webrtc_logging_private::StopRtpDump;
namespace Store = api::webrtc_logging_private::Store;
namespace Upload = api::webrtc_logging_private::Upload;
namespace UploadStored = api::webrtc_logging_private::UploadStored;
namespace StartAudioDebugRecordings =
    api::webrtc_logging_private::StartAudioDebugRecordings;
namespace StopAudioDebugRecordings =
    api::webrtc_logging_private::StopAudioDebugRecordings;
namespace StartEventLogging = api::webrtc_logging_private::StartEventLogging;
namespace GetLogsDirectory = api::webrtc_logging_private::GetLogsDirectory;

// TODO(hlundin): Consolidate with WebrtcAudioPrivateFunction and improve.
// http://crbug.com/710371
content::RenderProcessHost* WebrtcLoggingPrivateFunction::RphFromRequest(
    const api::webrtc_logging_private::RequestInfo& request,
    const std::string& security_origin,
    std::string* error) {
  *error = "Not implement";
  return nullptr;
}

// TODO(mshin): Enable the below code after supporting WebRtc Logging
// WebRtcLoggingController*
// WebrtcLoggingPrivateFunction::LoggingControllerFromRequest(
//     const api::webrtc_logging_private::RequestInfo& request,
//     const std::string& security_origin,
//     std::string* error) {
//   content::RenderProcessHost* host =
//       RphFromRequest(request, security_origin, error);
//   if (!host) {
//     DCHECK(!error->empty()) << "|error| must be set by RphFromRequest()";
//     return nullptr;
//   }
//   return WebRtcLoggingController::FromRenderProcessHost(host);
// }
//
// WebRtcLoggingController*
// WebrtcLoggingPrivateFunctionWithGenericCallback::PrepareTask(
//     const api::webrtc_logging_private::RequestInfo& request,
//     const std::string& security_origin,
//     WebRtcLoggingController::GenericDoneCallback* callback,
//     std::string* error) {
//   *callback = base::BindOnce(
//       &WebrtcLoggingPrivateFunctionWithGenericCallback::FireCallback, this);
//   return LoggingControllerFromRequest(request, security_origin, error);
// }

void WebrtcLoggingPrivateFunctionWithGenericCallback::FireCallback(
    bool success, const std::string& error_message) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  if (success) {
    Respond(NoArguments());
  } else {
    Respond(Error(error_message));
  }
}

void WebrtcLoggingPrivateFunctionWithUploadCallback::FireCallback(
    bool success, const std::string& report_id,
    const std::string& error_message) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  if (success) {
    api::webrtc_logging_private::UploadResult result;
    result.report_id = report_id;
    Respond(WithArguments(result.ToValue()));
  } else {
    Respond(Error(error_message));
  }
}

void WebrtcLoggingPrivateFunctionWithRecordingDoneCallback::FireErrorCallback(
    const std::string& error_message) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  Respond(Error(error_message));
}

void WebrtcLoggingPrivateFunctionWithRecordingDoneCallback::FireCallback(
    const std::string& prefix_path,
    bool did_stop,
    bool did_manual_stop) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  api::webrtc_logging_private::RecordingInfo result;
  result.prefix_path = prefix_path;
  result.did_stop = did_stop;
  result.did_manual_stop = did_manual_stop;
  Respond(WithArguments(result.ToValue()));
}

ExtensionFunction::ResponseAction
WebrtcLoggingPrivateSetMetaDataFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateStartFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction
WebrtcLoggingPrivateSetUploadOnRenderCloseFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateStopFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateStoreFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction
WebrtcLoggingPrivateUploadStoredFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateUploadFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction WebrtcLoggingPrivateDiscardFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction
WebrtcLoggingPrivateStartRtpDumpFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction
WebrtcLoggingPrivateStopRtpDumpFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction
WebrtcLoggingPrivateStartAudioDebugRecordingsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction
WebrtcLoggingPrivateStopAudioDebugRecordingsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ExtensionFunction::ResponseAction
WebrtcLoggingPrivateStartEventLoggingFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

void WebrtcLoggingPrivateStartEventLoggingFunction::FireCallback(
    bool success,
    const std::string& log_id,
    const std::string& error_message) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  if (success) {
    DCHECK(!log_id.empty());
    DCHECK(error_message.empty());
    api::webrtc_logging_private::StartEventLoggingResult result;
    result.log_id = log_id;
    Respond(WithArguments(result.ToValue()));
  } else {
    DCHECK(log_id.empty());
    DCHECK(!error_message.empty());
    Respond(Error(error_message));
  }
}

ExtensionFunction::ResponseAction
WebrtcLoggingPrivateGetLogsDirectoryFunction::Run() {
  return RespondNow(Error("Not supported on the current OS"));
}

void WebrtcLoggingPrivateGetLogsDirectoryFunction::FireCallback(
    const std::string& filesystem_id,
    const std::string& base_name) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  base::Value::Dict dict;
  dict.Set("fileSystemId", filesystem_id);
  dict.Set("baseName", base_name);
  Respond(WithArguments(std::move(dict)));
}

void WebrtcLoggingPrivateGetLogsDirectoryFunction::FireErrorCallback(
    const std::string& error_message) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  Respond(Error(error_message));
}

}  // namespace extensions
