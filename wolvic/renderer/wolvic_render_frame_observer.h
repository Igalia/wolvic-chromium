// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WOLVIC_RENDERER_WOLVIC_RENDER_FRAME_OBSERVER_H_
#define WOLVIC_RENDERER_WOLVIC_RENDER_FRAME_OBSERVER_H_

#include "content/public/renderer/render_frame_observer.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"

namespace wolvic {

class WolvicRenderFrameObserver : public content::RenderFrameObserver {
 public:
  explicit WolvicRenderFrameObserver(content::RenderFrame* render_frame);

  WolvicRenderFrameObserver(const WolvicRenderFrameObserver&) = delete;
  WolvicRenderFrameObserver& operator=(const WolvicRenderFrameObserver&) =
      delete;

  service_manager::BinderRegistry* registry() { return &registry_; }

  blink::AssociatedInterfaceRegistry* associated_interfaces() {
    return &associated_interfaces_;
  }

 private:
  ~WolvicRenderFrameObserver() override;

  // RenderFrameObserver:
  void OnInterfaceRequestForFrame(
    const std::string& interface_name,
    mojo::ScopedMessagePipeHandle* interface_pipe) override;
  bool OnAssociatedInterfaceRequestForFrame(
      const std::string& interface_name,
      mojo::ScopedInterfaceEndpointHandle* handle) override;
  void OnDestruct() override;

  service_manager::BinderRegistry registry_;
  blink::AssociatedInterfaceRegistry associated_interfaces_;
};

}  // namespace wolvic

#endif  // WOLVIC_RENDERER_WOLVIC_RENDER_FRAME_OBSERVER_H_
