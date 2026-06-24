// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WOLVIC_BROWSER_VR_WVR_GRAPHICS_DELEGATE_H_
#define WOLVIC_BROWSER_VR_WVR_GRAPHICS_DELEGATE_H_

#include <memory>

#include "base/cancelable_callback.h"
#include "base/memory/weak_ptr.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gl/android/scoped_a_native_window.h"
#include "ui/gl/gl_bindings.h"

namespace device {
class MailboxToSurfaceBridge;
struct WebXrSharedBuffer;
}  // namespace device

namespace gfx {
class GpuFence;
}

namespace gl {
class GLContext;
class GLSurface;
class SurfaceTexture;
}  // namespace gl

namespace wolvic {

class WvrGraphicsDelegate {
 public:
  WvrGraphicsDelegate();

  WvrGraphicsDelegate(const WvrGraphicsDelegate&) = delete;
  WvrGraphicsDelegate& operator=(const WvrGraphicsDelegate&) = delete;

  ~WvrGraphicsDelegate();

  void InitializeGl(base::OnceClosure callback);

  base::WeakPtr<WvrGraphicsDelegate> GetWeakPtr();

  bool CreateOrResizeWebXrSurface(
      const gfx::Size& size,
      base::RepeatingClosure on_webxr_frame_available);
  gl::SurfaceTexture* webxr_surface_texture() {
    return webxr_surface_texture_.get();
  }
  gfx::Size webxr_surface_size() const { return webxr_surface_size_; }
  int32_t webxr_texture_handle() const { return texture_handle_id_; }

  // Shared buffer management for DRAW_INTO_TEXTURE_MAILBOX transport.
  std::unique_ptr<device::WebXrSharedBuffer> CreateSharedBuffer();
  void ResizeSharedBuffer(device::WebXrSharedBuffer* buffer,
                          const gfx::Size& size,
                          device::MailboxToSurfaceBridge* mailbox_bridge);
  void ServerWaitForGpuFence(std::unique_ptr<gfx::GpuFence> gpu_fence);
  // Creates a GPU fence on the WVR GL context capturing all work submitted so
  // far (including the blit). Handed to the renderer via OnSubmitFrameGpuFence
  // so it knows when the shared buffer is safe to reuse for the next frame.
  std::unique_ptr<gfx::GpuFence> CreateGpuFence();
  // Blits the shared buffer onto the SurfaceTexture via an EGL window surface.
  // Returns true if SwapBuffers was called (frame queued), false on failure.
  bool BlitSharedBufferToSurfaceTexture(device::WebXrSharedBuffer* buffer);

 private:
  void CreateBlitProgram();

  // samplerExternalOES texture data for WebVR content image.
  int webvr_texture_id_ = 0;
  int32_t texture_handle_id_;

  // Java WVRSurfaceTexture instance.
  base::android::ScopedJavaGlobalRef<jobject> j_surface_texture_;

  scoped_refptr<gl::GLContext> context_;
  scoped_refptr<gl::GLSurface> surface_;
  scoped_refptr<gl::SurfaceTexture> webxr_surface_texture_;

  // EGL window surface backed by the SurfaceTexture; used for blitting.
  gl::ScopedANativeWindow webxr_surface_native_window_;
  scoped_refptr<gl::GLSurface> webxr_surface_texture_surface_;

  gfx::Size webxr_surface_size_;

  // Fullscreen quad shader for blitting shared buffer → SurfaceTexture.
  GLuint blit_program_ = 0;

  base::WeakPtrFactory<WvrGraphicsDelegate> weak_ptr_factory_{this};
};

}  // namespace wolvic

#endif  // WOLVIC_BROWSER_VR_WVR_GRAPHICS_DELEGATE_H_
