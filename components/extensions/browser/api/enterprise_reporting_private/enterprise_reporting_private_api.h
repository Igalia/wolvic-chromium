// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_ENTERPRISE_REPORTING_PRIVATE_ENTERPRISE_REPORTING_PRIVATE_API_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_ENTERPRISE_REPORTING_PRIVATE_ENTERPRISE_REPORTING_PRIVATE_API_H_

#include <memory>
#include <string>

#include "base/task/sequenced_task_runner.h"
#include "base/time/time.h"
#include "components/extensions/common/api/enterprise_reporting_private.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

namespace enterprise_reporting {

extern const char kDeviceIdNotFound[];

}  // namespace enterprise_reporting


class EnterpriseReportingPrivateGetDeviceIdFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("enterprise.reportingPrivate.getDeviceId",
                             ENTERPRISEREPORTINGPRIVATE_GETDEVICEID)

  EnterpriseReportingPrivateGetDeviceIdFunction();

  EnterpriseReportingPrivateGetDeviceIdFunction(
      const EnterpriseReportingPrivateGetDeviceIdFunction&) = delete;
  EnterpriseReportingPrivateGetDeviceIdFunction& operator=(
      const EnterpriseReportingPrivateGetDeviceIdFunction&) = delete;

  // ExtensionFunction
  ExtensionFunction::ResponseAction Run() override;

 private:
  ~EnterpriseReportingPrivateGetDeviceIdFunction() override;
};

#if !BUILDFLAG(IS_LINUX)

class EnterpriseReportingPrivateGetPersistentSecretFunction
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("enterprise.reportingPrivate.getPersistentSecret",
                             ENTERPRISEREPORTINGPRIVATE_GETPERSISTENTSECRET)

  EnterpriseReportingPrivateGetPersistentSecretFunction();
  EnterpriseReportingPrivateGetPersistentSecretFunction(
      const EnterpriseReportingPrivateGetPersistentSecretFunction&) = delete;
  EnterpriseReportingPrivateGetPersistentSecretFunction& operator=(
      const EnterpriseReportingPrivateGetPersistentSecretFunction&) = delete;

 private:
  ~EnterpriseReportingPrivateGetPersistentSecretFunction() override;

  // ExtensionFunction
  ExtensionFunction::ResponseAction Run() override;
};

#endif  // !BUILDFLAG(IS_LINUX)

class EnterpriseReportingPrivateGetDeviceDataFunction
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("enterprise.reportingPrivate.getDeviceData",
                             ENTERPRISEREPORTINGPRIVATE_GETDEVICEDATA)

  EnterpriseReportingPrivateGetDeviceDataFunction();
  EnterpriseReportingPrivateGetDeviceDataFunction(
      const EnterpriseReportingPrivateGetDeviceDataFunction&) = delete;
  EnterpriseReportingPrivateGetDeviceDataFunction& operator=(
      const EnterpriseReportingPrivateGetDeviceDataFunction&) = delete;

 private:
  ~EnterpriseReportingPrivateGetDeviceDataFunction() override;

  // ExtensionFunction
  ExtensionFunction::ResponseAction Run() override;
};

class EnterpriseReportingPrivateSetDeviceDataFunction
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("enterprise.reportingPrivate.setDeviceData",
                             ENTERPRISEREPORTINGPRIVATE_SETDEVICEDATA)

  EnterpriseReportingPrivateSetDeviceDataFunction();
  EnterpriseReportingPrivateSetDeviceDataFunction(
      const EnterpriseReportingPrivateSetDeviceDataFunction&) = delete;
  EnterpriseReportingPrivateSetDeviceDataFunction& operator=(
      const EnterpriseReportingPrivateSetDeviceDataFunction&) = delete;

 private:
  ~EnterpriseReportingPrivateSetDeviceDataFunction() override;

  // ExtensionFunction
  ExtensionFunction::ResponseAction Run() override;

};

class EnterpriseReportingPrivateGetDeviceInfoFunction
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("enterprise.reportingPrivate.getDeviceInfo",
                             ENTERPRISEREPORTINGPRIVATE_GETDEVICEINFO)

  EnterpriseReportingPrivateGetDeviceInfoFunction();
  EnterpriseReportingPrivateGetDeviceInfoFunction(
      const EnterpriseReportingPrivateGetDeviceInfoFunction&) = delete;
  EnterpriseReportingPrivateGetDeviceInfoFunction& operator=(
      const EnterpriseReportingPrivateGetDeviceInfoFunction&) = delete;

 private:
  ~EnterpriseReportingPrivateGetDeviceInfoFunction() override;

  // ExtensionFunction
  ExtensionFunction::ResponseAction Run() override;
};

class EnterpriseReportingPrivateGetContextInfoFunction
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("enterprise.reportingPrivate.getContextInfo",
                             ENTERPRISEREPORTINGPRIVATE_GETCONTEXTINFO)

  EnterpriseReportingPrivateGetContextInfoFunction();
  EnterpriseReportingPrivateGetContextInfoFunction(
      const EnterpriseReportingPrivateGetContextInfoFunction&) = delete;
  EnterpriseReportingPrivateGetContextInfoFunction& operator=(
      const EnterpriseReportingPrivateGetContextInfoFunction&) = delete;

 private:
  ~EnterpriseReportingPrivateGetContextInfoFunction() override;

  // ExtensionFunction
  ExtensionFunction::ResponseAction Run() override;
};

class EnterpriseReportingPrivateGetCertificateFunction
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("enterprise.reportingPrivate.getCertificate",
                             ENTERPRISEREPORTINGPRIVATE_GETCERTIFICATE)

  EnterpriseReportingPrivateGetCertificateFunction();
  EnterpriseReportingPrivateGetCertificateFunction(
      const EnterpriseReportingPrivateGetCertificateFunction&) = delete;
  EnterpriseReportingPrivateGetCertificateFunction& operator=(
      const EnterpriseReportingPrivateGetCertificateFunction&) = delete;

 private:
  ~EnterpriseReportingPrivateGetCertificateFunction() override;

  // ExtensionFunction:
  ExtensionFunction::ResponseAction Run() override;
};

#if BUILDFLAG(IS_WIN) || BUILDFLAG(IS_MAC) || BUILDFLAG(IS_LINUX)

class EnterpriseReportingPrivateGetFileSystemInfoFunction
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("enterprise.reportingPrivate.getFileSystemInfo",
                             ENTERPRISEREPORTINGPRIVATE_GETFILESYSTEMINFO)

  EnterpriseReportingPrivateGetFileSystemInfoFunction();
  EnterpriseReportingPrivateGetFileSystemInfoFunction(
      const EnterpriseReportingPrivateGetFileSystemInfoFunction&) = delete;
  EnterpriseReportingPrivateGetFileSystemInfoFunction& operator=(
      const EnterpriseReportingPrivateGetFileSystemInfoFunction&) = delete;

 private:
  ~EnterpriseReportingPrivateGetFileSystemInfoFunction() override;

  // ExtensionFunction
  ExtensionFunction::ResponseAction Run() override;
};

#endif  // BUILDFLAG(IS_WIN) || BUILDFLAG(IS_MAC) || BUILDFLAG(IS_LINUX)

}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_ENTERPRISE_REPORTING_PRIVATE_ENTERPRISE_REPORTING_PRIVATE_API_H_
