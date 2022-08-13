/*
 * OpenKneeboard
 *
 * Copyright (C) 2022 Fred Emmott <fred@fredemmott.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */
#pragma once

#include <OpenKneeboard/D3D11.h>
#include <OpenKneeboard/config.h>
#include <d3d12.h>

#include "OpenXRKneeboard.h"

struct XrGraphicsBindingD3D12KHR;

namespace OpenKneeboard {

class OpenXRD3D12Kneeboard final : public OpenXRKneeboard {
 public:
  OpenXRD3D12Kneeboard() = delete;
  OpenXRD3D12Kneeboard(
    XrSession,
    const std::shared_ptr<OpenXRNext>&,
    const XrGraphicsBindingD3D12KHR&);
  ~OpenXRD3D12Kneeboard();

 protected:
  virtual XrSwapchain CreateSwapChain(XrSession, uint8_t layerIndex) override;
  virtual bool Render(
    XrSwapchain swapchain,
    const SHM::Snapshot& snapshot,
    uint8_t layerIndex,
    const VRKneeboard::RenderParameters&) override;

 private:
  ID3D12Device* mDevice12 = nullptr;
  winrt::com_ptr<ID3D11Device> mDevice11;
  winrt::com_ptr<ID3D11DeviceContext> mContext11;

  std::array<
    std::vector<std::shared_ptr<D3D11::RenderTargetViewFactory>>,
    MaxLayers>
    mRenderTargetViews;
};

}// namespace OpenKneeboard
