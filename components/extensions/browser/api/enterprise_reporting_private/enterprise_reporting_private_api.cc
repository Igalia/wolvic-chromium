// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/enterprise_reporting_private/enterprise_reporting_private_api.h"

#include <memory>
#include <utility>

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/strings/stringprintf.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/single_thread_task_runner.h"
#include "base/task/thread_pool.h"
#include "base/values.h"
#include "components/content_settings/core/common/pref_names.h"
#include "net/cert/x509_util.h"

namespace extensions {

namespace enterprise_reporting {
const char kDeviceIdNotFound[] = "Failed to retrieve the device id.";
}  // namespace enterprise_reporting

// GetDeviceId

EnterpriseReportingPrivateGetDeviceIdFunction::
    EnterpriseReportingPrivateGetDeviceIdFunction() {}

ExtensionFunction::ResponseAction
EnterpriseReportingPrivateGetDeviceIdFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

EnterpriseReportingPrivateGetDeviceIdFunction::
    ~EnterpriseReportingPrivateGetDeviceIdFunction() = default;

// getPersistentSecret

#if !BUILDFLAG(IS_LINUX)

EnterpriseReportingPrivateGetPersistentSecretFunction::
    EnterpriseReportingPrivateGetPersistentSecretFunction() = default;
EnterpriseReportingPrivateGetPersistentSecretFunction::
    ~EnterpriseReportingPrivateGetPersistentSecretFunction() = default;

ExtensionFunction::ResponseAction
EnterpriseReportingPrivateGetPersistentSecretFunction::Run() {
  return RespondNow(Error("Not Implement"));
}
#endif  // !BUILDFLAG(IS_LINUX)

// getDeviceData

EnterpriseReportingPrivateGetDeviceDataFunction::
    EnterpriseReportingPrivateGetDeviceDataFunction() = default;
EnterpriseReportingPrivateGetDeviceDataFunction::
    ~EnterpriseReportingPrivateGetDeviceDataFunction() = default;

ExtensionFunction::ResponseAction
EnterpriseReportingPrivateGetDeviceDataFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// setDeviceData

EnterpriseReportingPrivateSetDeviceDataFunction::
    EnterpriseReportingPrivateSetDeviceDataFunction() = default;
EnterpriseReportingPrivateSetDeviceDataFunction::
    ~EnterpriseReportingPrivateSetDeviceDataFunction() = default;

ExtensionFunction::ResponseAction
EnterpriseReportingPrivateSetDeviceDataFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// getDeviceInfo

EnterpriseReportingPrivateGetDeviceInfoFunction::
    EnterpriseReportingPrivateGetDeviceInfoFunction() = default;
EnterpriseReportingPrivateGetDeviceInfoFunction::
    ~EnterpriseReportingPrivateGetDeviceInfoFunction() = default;

ExtensionFunction::ResponseAction
EnterpriseReportingPrivateGetDeviceInfoFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

// getContextInfo

EnterpriseReportingPrivateGetContextInfoFunction::
    EnterpriseReportingPrivateGetContextInfoFunction() = default;
EnterpriseReportingPrivateGetContextInfoFunction::
    ~EnterpriseReportingPrivateGetContextInfoFunction() = default;

ExtensionFunction::ResponseAction
EnterpriseReportingPrivateGetContextInfoFunction::Run() {

  return RespondLater();
}


// getCertificate

EnterpriseReportingPrivateGetCertificateFunction::
    EnterpriseReportingPrivateGetCertificateFunction() = default;
EnterpriseReportingPrivateGetCertificateFunction::
    ~EnterpriseReportingPrivateGetCertificateFunction() = default;

ExtensionFunction::ResponseAction
EnterpriseReportingPrivateGetCertificateFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
