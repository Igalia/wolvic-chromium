// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Implements the Chrome Extensions Debugger API.

#include "components/extensions/browser/api/debugger/debugger_api.h"

#include <stddef.h>

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <utility>

#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/lazy_instance.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/singleton.h"
#include "base/metrics/histogram_functions.h"
#include "base/ranges/algorithm.h"
#include "base/scoped_observation.h"
#include "base/strings/string_number_conversions.h"
#include "base/types/optional_util.h"
#include "base/values.h"
#include "components/security_interstitials/content/security_interstitial_tab_helper.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_client.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/url_utils.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_host.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_registry_observer.h"
#include "extensions/browser/extension_util.h"
#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest.h"
#include "extensions/common/constants.h"
#include "extensions/common/error_utils.h"
#include "extensions/common/extension.h"
#include "extensions/common/extension_id.h"
#include "extensions/common/manifest_constants.h"
#include "extensions/common/permissions/permissions_data.h"
#include "extensions/common/switches.h"
#include "url/origin.h"

using content::DevToolsAgentHost;
using content::RenderProcessHost;
using content::RenderWidgetHost;
using content::WebContents;

namespace Attach = extensions::api::debugger::Attach;
namespace Detach = extensions::api::debugger::Detach;
namespace OnDetach = extensions::api::debugger::OnDetach;
namespace OnEvent = extensions::api::debugger::OnEvent;
namespace SendCommand = extensions::api::debugger::SendCommand;

namespace extensions {
class ExtensionRegistry;

// DebuggerFunction -----------------------------------------------------------

DebuggerFunction::DebuggerFunction() : client_host_(nullptr) {}

DebuggerFunction::~DebuggerFunction() = default;

std::string DebuggerFunction::FormatErrorMessage(const std::string& format) {
  return ErrorUtils::FormatErrorMessage(
      format, "Not Implement", *debuggee_.target_id);
}

bool DebuggerFunction::InitAgentHost(std::string* error) {
  *error = "Not Implement";
  return false;
}

bool DebuggerFunction::InitClientHost(std::string* error) {
  if (!InitAgentHost(error))
    return false;

  client_host_ = FindClientHost();
  if (!client_host_) {
    *error = "Not Implement";
    return false;
  }

  return true;
}

ExtensionDevToolsClientHost* DebuggerFunction::FindClientHost() {
  return nullptr;
}

// DebuggerAttachFunction -----------------------------------------------------

DebuggerAttachFunction::DebuggerAttachFunction() = default;

DebuggerAttachFunction::~DebuggerAttachFunction() = default;

ExtensionFunction::ResponseAction DebuggerAttachFunction::Run() {
  std::optional<Attach::Params> params = Attach::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  return RespondNow(Error("Not Implement"));
}

// DebuggerDetachFunction -----------------------------------------------------

DebuggerDetachFunction::DebuggerDetachFunction() = default;

DebuggerDetachFunction::~DebuggerDetachFunction() = default;

ExtensionFunction::ResponseAction DebuggerDetachFunction::Run() {
  std::optional<Detach::Params> params = Detach::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  return RespondNow(Error("Not Implement"));
}

// DebuggerSendCommandFunction ------------------------------------------------

DebuggerSendCommandFunction::DebuggerSendCommandFunction() = default;

DebuggerSendCommandFunction::~DebuggerSendCommandFunction() = default;

ExtensionFunction::ResponseAction DebuggerSendCommandFunction::Run() {
  std::optional<SendCommand::Params> params =
      SendCommand::Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  return RespondNow(Error("Not Implement"));
}

void DebuggerSendCommandFunction::SendResponseBody(base::Value response) {
  if (base::Value* error_body = response.GetDict().Find("error")) {
    std::string error;
    base::JSONWriter::Write(*error_body, &error);
    Respond(Error(std::move(error)));
    return;
  }

  SendCommand::Results::Result result;
  if (base::Value::Dict* result_body = response.GetDict().FindDict("result")) {
    result.additional_properties = std::move(*result_body);
  }

  Respond(ArgumentList(SendCommand::Results::Create(result)));
}

void DebuggerSendCommandFunction::SendDetachedError() {
  // Respond(Error(debugger_api_constants::kDetachedWhileHandlingError));
}

DebuggerGetTargetsFunction::DebuggerGetTargetsFunction() = default;

DebuggerGetTargetsFunction::~DebuggerGetTargetsFunction() = default;

ExtensionFunction::ResponseAction DebuggerGetTargetsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
