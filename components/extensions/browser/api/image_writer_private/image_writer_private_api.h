// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_IMAGE_WRITER_PRIVATE_IMAGE_WRITER_PRIVATE_API_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_IMAGE_WRITER_PRIVATE_IMAGE_WRITER_PRIVATE_API_H_

#include "components/extensions/common/api/image_writer_private.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

class ImageWriterPrivateBaseFunction : public ExtensionFunction {
 public:
  ImageWriterPrivateBaseFunction();

  ImageWriterPrivateBaseFunction(const ImageWriterPrivateBaseFunction&) =
      delete;
  ImageWriterPrivateBaseFunction& operator=(
      const ImageWriterPrivateBaseFunction&) = delete;

  virtual void OnComplete(bool success, const std::string& error);

 protected:
  ~ImageWriterPrivateBaseFunction() override;
};

class ImageWriterPrivateWriteFromUrlFunction
    : public ImageWriterPrivateBaseFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("imageWriterPrivate.writeFromUrl",
                             IMAGEWRITER_WRITEFROMURL)
  ImageWriterPrivateWriteFromUrlFunction();

 private:
  ~ImageWriterPrivateWriteFromUrlFunction() override;
  ResponseAction Run() override;
};

class ImageWriterPrivateWriteFromFileFunction
    : public ImageWriterPrivateBaseFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("imageWriterPrivate.writeFromFile",
                             IMAGEWRITER_WRITEFROMFILE)
  ImageWriterPrivateWriteFromFileFunction();

 private:
  ~ImageWriterPrivateWriteFromFileFunction() override;
  ResponseAction Run() override;
};

class ImageWriterPrivateCancelWriteFunction
    : public ImageWriterPrivateBaseFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("imageWriterPrivate.cancelWrite",
                             IMAGEWRITER_CANCELWRITE)
  ImageWriterPrivateCancelWriteFunction();

 private:
  ~ImageWriterPrivateCancelWriteFunction() override;
  ResponseAction Run() override;
};

class ImageWriterPrivateDestroyPartitionsFunction
    : public ImageWriterPrivateBaseFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("imageWriterPrivate.destroyPartitions",
                             IMAGEWRITER_DESTROYPARTITIONS)
  ImageWriterPrivateDestroyPartitionsFunction();

 private:
  ~ImageWriterPrivateDestroyPartitionsFunction() override;
  ResponseAction Run() override;
};

class ImageWriterPrivateListRemovableStorageDevicesFunction
    : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("imageWriterPrivate.listRemovableStorageDevices",
                             IMAGEWRITER_LISTREMOVABLESTORAGEDEVICES)
  ImageWriterPrivateListRemovableStorageDevicesFunction();

 private:
  ~ImageWriterPrivateListRemovableStorageDevicesFunction() override;
  ResponseAction Run() override;
};

}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_IMAGE_WRITER_PRIVATE_IMAGE_WRITER_PRIVATE_API_H_
