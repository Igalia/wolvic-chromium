// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/extensions/browser/api/image_writer_private/image_writer_private_api.h"

#include <utility>

#include "base/functional/bind.h"
#include "components/extensions/browser/api/image_writer_private/error_constants.h"
#include "components/prefs/pref_service.h"
#include "extensions/browser/api/extensions_api_client.h"
#include "extensions/browser/api/file_handlers/app_file_handler_util.h"

namespace image_writer_api = extensions::api::image_writer_private;

namespace extensions {

ImageWriterPrivateBaseFunction::ImageWriterPrivateBaseFunction() = default;

ImageWriterPrivateBaseFunction::~ImageWriterPrivateBaseFunction() = default;

void ImageWriterPrivateBaseFunction::OnComplete(bool success,
                                                const std::string& error) {
  if (success)
    Respond(NoArguments());
  else
    Respond(Error(error));
}

ImageWriterPrivateWriteFromUrlFunction::
    ImageWriterPrivateWriteFromUrlFunction() = default;

ImageWriterPrivateWriteFromUrlFunction::
    ~ImageWriterPrivateWriteFromUrlFunction() = default;

ExtensionFunction::ResponseAction
ImageWriterPrivateWriteFromUrlFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ImageWriterPrivateWriteFromFileFunction::
    ImageWriterPrivateWriteFromFileFunction() = default;

ImageWriterPrivateWriteFromFileFunction::
    ~ImageWriterPrivateWriteFromFileFunction() = default;

ExtensionFunction::ResponseAction
ImageWriterPrivateWriteFromFileFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ImageWriterPrivateCancelWriteFunction::ImageWriterPrivateCancelWriteFunction() =
    default;

ImageWriterPrivateCancelWriteFunction::
    ~ImageWriterPrivateCancelWriteFunction() = default;

ExtensionFunction::ResponseAction ImageWriterPrivateCancelWriteFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ImageWriterPrivateDestroyPartitionsFunction::
    ImageWriterPrivateDestroyPartitionsFunction() = default;

ImageWriterPrivateDestroyPartitionsFunction::
    ~ImageWriterPrivateDestroyPartitionsFunction() = default;

ExtensionFunction::ResponseAction
ImageWriterPrivateDestroyPartitionsFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

ImageWriterPrivateListRemovableStorageDevicesFunction::
    ImageWriterPrivateListRemovableStorageDevicesFunction() = default;

ImageWriterPrivateListRemovableStorageDevicesFunction::
    ~ImageWriterPrivateListRemovableStorageDevicesFunction() = default;

ExtensionFunction::ResponseAction
ImageWriterPrivateListRemovableStorageDevicesFunction::Run() {
  return RespondNow(Error("Not Implement"));
}

}  // namespace extensions
