// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wolvic/renderer/wolvic_render_frame_observer.h"

#include "content/public/renderer/render_frame.h"

namespace wolvic {

WolvicRenderFrameObserver::WolvicRenderFrameObserver(
    content::RenderFrame* render_frame)
    : content::RenderFrameObserver(render_frame) {}

WolvicRenderFrameObserver::~WolvicRenderFrameObserver() = default;

void WolvicRenderFrameObserver::OnInterfaceRequestForFrame(
  const std::string& interface_name,
  mojo::ScopedMessagePipeHandle* interface_pipe) {
  registry_.TryBindInterface(interface_name, interface_pipe);
}

bool WolvicRenderFrameObserver::OnAssociatedInterfaceRequestForFrame(
    const std::string& interface_name,
    mojo::ScopedInterfaceEndpointHandle* handle) {
  return associated_interfaces_.TryBindInterface(interface_name, handle);
}

void WolvicRenderFrameObserver::OnDestruct() {
  delete this;
}

}  // namespace wolvic
