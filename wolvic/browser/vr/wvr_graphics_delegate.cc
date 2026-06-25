// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/browser/vr/wvr_graphics_delegate.h"

#include <memory>

#include "base/android/jni_android.h"
#include "base/android/scoped_hardware_buffer_handle.h"
#include "base/functional/callback_helpers.h"
#include "base/logging.h"
#include "device/vr/android/mailbox_to_surface_bridge.h"
#include "device/vr/android/web_xr_presentation_state.h"
#include "gpu/command_buffer/client/client_shared_image.h"
#include "gpu/command_buffer/common/shared_image_usage.h"
#include "gpu/command_buffer/service/ahardwarebuffer_utils.h"
#include "gpu/ipc/common/android/android_hardware_buffer_utils.h"
#include "ui/gfx/buffer_types.h"
#include "ui/gfx/color_space.h"
#include "ui/gfx/frame_data.h"
#include "ui/gfx/gpu_fence.h"
#include "ui/gfx/gpu_memory_buffer_handle.h"
#include "ui/gl/android/surface_texture.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_fence.h"
#include "ui/gl/gl_surface.h"
#include "ui/gl/gl_utils.h"
#include "ui/gl/gpu_preference.h"
#include "ui/gl/init/gl_factory.h"
#include "wolvic/jni_headers/WVRSurfaceTexture_jni.h"

namespace wolvic {

namespace {

int32_t GetNextTextureHandleId() {
  static int32_t s_next_texture_handle_id = 0;
  if (s_next_texture_handle_id == std::numeric_limits<int32_t>::max())
    s_next_texture_handle_id = 0;
  return ++s_next_texture_handle_id;
}

GLuint CompileShader(GLenum type, const char* source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (!status) {
    GLint len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    std::string log(len, '\0');
    glGetShaderInfoLog(shader, len, nullptr, log.data());
    LOG(ERROR) << "Shader compile error: " << log;
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

}  // namespace

WvrGraphicsDelegate::WvrGraphicsDelegate()
    : texture_handle_id_(GetNextTextureHandleId()) {}

WvrGraphicsDelegate::~WvrGraphicsDelegate() {
  if (blit_program_) {
    // Deleting a GL program requires a current context. This destructor must
    // run on the WVR thread where context_ is current; make it current to be
    // safe.
    if (context_ && surface_ && context_->MakeCurrent(surface_.get())) {
      glDeleteProgram(blit_program_);
    }
  }
  if (j_surface_texture_) {
    JNIEnv* env = base::android::AttachCurrentThread();
    Java_WVRSurfaceTexture_release(env, j_surface_texture_);
  }
}

base::WeakPtr<WvrGraphicsDelegate> WvrGraphicsDelegate::GetWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

void WvrGraphicsDelegate::InitializeGl(base::OnceClosure callback) {
  gl::DisableANGLE();

  gl::GLDisplay* display = nullptr;
  if (gl::GetGLImplementation() == gl::kGLImplementationNone) {
    display = gl::init::InitializeGLOneOff(gl::GpuPreference::kDefault);
    if (!display) {
      LOG(ERROR) << "gl::init::InitializeGLOneOff failed";
      return;
    }
  } else {
    display = gl::GetDefaultDisplayEGL();
  }

  surface_ = gl::init::CreateOffscreenGLSurface(display, gfx::Size());

  if (!surface_.get()) {
    LOG(ERROR) << "gl::init::CreateOffscreenGLSurface failed";
    return;
  }

  context_ = gl::init::CreateGLContext(nullptr, surface_.get(),
                                       gl::GLContextAttribs());
  if (!context_.get()) {
    LOG(ERROR) << "gl::init::CreateGLContext failed";
    return;
  }
  if (!context_->MakeCurrent(surface_.get())) {
    LOG(ERROR) << "gl::GLContext::MakeCurrent() failed";
    return;
  }

  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  unsigned int texture[1];
  glGenTextures(1, texture);
  webvr_texture_id_ = texture[0];

  std::move(callback).Run();
}

bool WvrGraphicsDelegate::CreateOrResizeWebXrSurface(
    const gfx::Size& size,
    base::RepeatingClosure on_webxr_frame_available) {
  DVLOG(2) << __func__ << ": size=" << size.width() << "x" << size.height();
  if (!webxr_surface_texture_) {
    DCHECK(on_webxr_frame_available)
        << "A callback must be provided to create the surface texture";
    webxr_surface_texture_ = gl::SurfaceTexture::Create(webvr_texture_id_);
    webxr_surface_texture_->SetFrameAvailableCallback(
        std::move(on_webxr_frame_available));

    DCHECK(!j_surface_texture_);
    JNIEnv* env = base::android::AttachCurrentThread();
    j_surface_texture_ = Java_WVRSurfaceTexture_create(
        env,
        texture_handle_id_,
        webxr_surface_texture_.get()->j_surface_texture());
  }

  if (size.IsEmpty()) {
    DVLOG(1) << "Ignore resize, invalid size";
    return false;
  }

  webxr_surface_texture_->SetDefaultBufferSize(size.width(), size.height());
  webxr_surface_size_ = size;

  // Create the EGL window surface backed by the SurfaceTexture only after the
  // default buffer size has been set, so the window surface binds at the
  // correct dimensions. BlitSharedBufferToSurfaceTexture swaps frames into it.
  if (!webxr_surface_texture_surface_) {
    webxr_surface_native_window_ = webxr_surface_texture_->CreateSurface();
    if (webxr_surface_native_window_) {
      gl::GLDisplay* display = gl::GetDefaultDisplayEGL();
      webxr_surface_texture_surface_ = gl::init::CreateViewGLSurface(
          display, webxr_surface_native_window_.a_native_window());
      if (!webxr_surface_texture_surface_) {
        LOG(ERROR) << "Failed to create SurfaceTexture window surface";
      }
    } else {
      LOG(ERROR) << "SurfaceTexture::CreateSurface() returned null";
    }
  }

  return true;
}

std::unique_ptr<device::WebXrSharedBuffer>
WvrGraphicsDelegate::CreateSharedBuffer() {
  auto buffer = std::make_unique<device::WebXrSharedBuffer>();
  glGenTextures(1, &buffer->local_texture.id);
  buffer->local_texture.target = GL_TEXTURE_EXTERNAL_OES;
  return buffer;
}

void WvrGraphicsDelegate::ResizeSharedBuffer(
    device::WebXrSharedBuffer* buffer,
    const gfx::Size& size,
    device::MailboxToSurfaceBridge* mailbox_bridge) {
  if (buffer->shared_image && buffer->shared_image->size() == size)
    return;

  if (buffer->shared_image) {
    mailbox_bridge->DestroySharedImage(buffer->sync_token,
                                       std::move(buffer->shared_image));
  }
  buffer->local_eglimage.reset();

  static constexpr gfx::BufferFormat kFormat = gfx::BufferFormat::RGBA_8888;
  static constexpr gfx::BufferUsage kUsage = gfx::BufferUsage::SCANOUT;

  gpu::SharedImageUsageSet shared_image_usage =
      gpu::SHARED_IMAGE_USAGE_SCANOUT | gpu::SHARED_IMAGE_USAGE_DISPLAY_READ |
      gpu::SHARED_IMAGE_USAGE_GLES2_READ | gpu::SHARED_IMAGE_USAGE_GLES2_WRITE;

  buffer->scoped_ahb_handle =
      gpu::CreateScopedHardwareBufferHandle(size, kFormat, kUsage);

  gfx::GpuMemoryBufferHandle gmb_handle;
  gmb_handle.type = gfx::ANDROID_HARDWARE_BUFFER;
  gmb_handle.android_hardware_buffer = buffer->scoped_ahb_handle.Clone();

  buffer->shared_image = mailbox_bridge->CreateSharedImage(
      std::move(gmb_handle), kFormat, size, gfx::ColorSpace(),
      shared_image_usage, buffer->sync_token);
  CHECK(buffer->shared_image);

  auto egl_image =
      gpu::CreateEGLImageFromAHardwareBuffer(buffer->scoped_ahb_handle.get());
  if (!egl_image.is_valid()) {
    LOG(ERROR) << "Failed to create EGLImage from AHardwareBuffer";
    return;
  }

  // AHardwareBuffer-backed EGLImages must be bound as GL_TEXTURE_EXTERNAL_OES.
  buffer->local_texture.target = GL_TEXTURE_EXTERNAL_OES;
  glBindTexture(buffer->local_texture.target, buffer->local_texture.id);
  glTexParameteri(buffer->local_texture.target, GL_TEXTURE_WRAP_S,
                  GL_CLAMP_TO_EDGE);
  glTexParameteri(buffer->local_texture.target, GL_TEXTURE_WRAP_T,
                  GL_CLAMP_TO_EDGE);
  glTexParameteri(buffer->local_texture.target, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR);
  glTexParameteri(buffer->local_texture.target, GL_TEXTURE_MAG_FILTER,
                  GL_LINEAR);
  glEGLImageTargetTexture2DOES(buffer->local_texture.target, egl_image.get());
  buffer->local_eglimage = std::move(egl_image);
}

void WvrGraphicsDelegate::ServerWaitForGpuFence(
    std::unique_ptr<gfx::GpuFence> gpu_fence) {
  auto local_fence = gl::GLFence::CreateFromGpuFence(*gpu_fence);
  local_fence->ServerWait();
}

std::unique_ptr<gfx::GpuFence> WvrGraphicsDelegate::CreateGpuFence() {
  if (!context_->IsCurrent(nullptr)) {
    context_->MakeCurrent(surface_.get());
  }
  std::unique_ptr<gl::GLFence> gl_fence = gl::GLFence::CreateForGpuFence();
  if (!gl_fence) {
    LOG(ERROR) << "Failed to create GL fence for GPU fence";
    return nullptr;
  }
  return gl_fence->GetGpuFence();
}

void WvrGraphicsDelegate::CreateBlitProgram() {
  static const char* kVertexShader = R"(
    attribute vec2 a_Position;
    varying vec2 v_TexCoord;
    void main() {
      v_TexCoord = (a_Position + vec2(1.0, 1.0)) * 0.5;
      gl_Position = vec4(a_Position, 0.0, 1.0);
    }
  )";

  static const char* kFragmentShader = R"(
    #extension GL_OES_EGL_image_external : require
    precision mediump float;
    uniform samplerExternalOES u_Texture;
    varying vec2 v_TexCoord;
    void main() {
      gl_FragColor = texture2D(u_Texture, v_TexCoord);
    }
  )";

  GLuint vert = CompileShader(GL_VERTEX_SHADER, kVertexShader);
  GLuint frag = CompileShader(GL_FRAGMENT_SHADER, kFragmentShader);
  if (!vert || !frag) {
    glDeleteShader(vert);
    glDeleteShader(frag);
    return;
  }

  blit_program_ = glCreateProgram();
  glAttachShader(blit_program_, vert);
  glAttachShader(blit_program_, frag);
  glLinkProgram(blit_program_);
  glDeleteShader(vert);
  glDeleteShader(frag);

  GLint status;
  glGetProgramiv(blit_program_, GL_LINK_STATUS, &status);
  if (!status) {
    GLint len;
    glGetProgramiv(blit_program_, GL_INFO_LOG_LENGTH, &len);
    std::string log(len, '\0');
    glGetProgramInfoLog(blit_program_, len, nullptr, log.data());
    LOG(ERROR) << "Blit program link error: " << log;
    glDeleteProgram(blit_program_);
    blit_program_ = 0;
  }
}

bool WvrGraphicsDelegate::BlitSharedBufferToSurfaceTexture(
    device::WebXrSharedBuffer* buffer) {
  DCHECK(buffer);
  DCHECK(buffer->local_eglimage.is_valid());
  if (!webxr_surface_texture_surface_) {
    LOG(ERROR) << "No SurfaceTexture window surface for blit";
    return false;
  }

  if (!blit_program_)
    CreateBlitProgram();
  if (!blit_program_)
    return false;

  // Switch to the SurfaceTexture window surface for output.
  if (!context_->MakeCurrent(webxr_surface_texture_surface_.get())) {
    LOG(ERROR) << "Failed to make SurfaceTexture surface current for blit";
    context_->MakeCurrent(surface_.get());
    return false;
  }

  glViewport(0, 0, webxr_surface_size_.width(), webxr_surface_size_.height());
  glUseProgram(blit_program_);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(buffer->local_texture.target, buffer->local_texture.id);
  glUniform1i(glGetUniformLocation(blit_program_, "u_Texture"), 0);

  static const GLfloat kQuadVertices[] = {
    -1.f, -1.f,
     1.f, -1.f,
    -1.f,  1.f,
    -1.f,  1.f,
     1.f, -1.f,
     1.f,  1.f,
  };
  GLint pos_loc = glGetAttribLocation(blit_program_, "a_Position");
  glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 0, kQuadVertices);
  glEnableVertexAttribArray(pos_loc);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDisableVertexAttribArray(pos_loc);

  glBindTexture(buffer->local_texture.target, 0);

  webxr_surface_texture_surface_->SwapBuffers(base::DoNothing(),
                                              gfx::FrameData());

  // Restore offscreen surface.
  context_->MakeCurrent(surface_.get());
  return true;
}

}  // namespace wolvic
