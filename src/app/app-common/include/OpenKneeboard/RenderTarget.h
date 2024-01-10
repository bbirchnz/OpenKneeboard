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

#include <OpenKneeboard/DXResources.h>
#include <OpenKneeboard/RenderTargetID.h>

#include <Windows.h>

#include <memory>

#include <d2d1_3.h>
#include <d3d11.h>

namespace OpenKneeboard {

struct DXesources;

/** Encapsulate a render target for use with either D3D11 or D2D.
 *
 * Only D2D or D3D can be active, and if acquired, the objects
 * must be released before passing the RenderTarget* on, or the
 * callee will not be able to use it.
 */
class RenderTarget final : public std::enable_shared_from_this<RenderTarget> {
 public:
  RenderTarget() = delete;
  ~RenderTarget();

  static std::shared_ptr<RenderTarget> Create(
    const DXResources& dxr,
    const winrt::com_ptr<ID3D11Texture2D>& texture);

  RenderTargetID GetID() const;

  class D2D;
  class D3D;
  friend class D2D;
  friend class D3D;

  D2D d2d();
  D3D d3d();

 private:
  RenderTarget(
    const DXResources& dxr,
    const winrt::com_ptr<ID3D11Texture2D>& texture);
  enum class Mode { Unattached, D2D, D3D };
  Mode mMode {Mode::Unattached};

  DXResources mDXR;

  RenderTargetID mID;

  winrt::com_ptr<ID2D1Bitmap1> mD2DBitmap;
  winrt::com_ptr<ID2D1Bitmap1> mD2DIntermediate;
  winrt::com_ptr<ID3D11Texture2D> mD2DIntermediateD3DTexture;
  winrt::com_ptr<ID3D11RenderTargetView> mD2DIntermediateD3DRenderTargetView;

  winrt::com_ptr<ID2D1Effect> mD2DWhiteLevel;
  winrt::com_ptr<ID2D1Effect> mD2DColorManagement;
  ID2D1Effect* mD2DLastEffect {nullptr};

  winrt::com_ptr<ID3D11Texture2D> mD3DTexture;
  winrt::com_ptr<ID3D11RenderTargetView> mD3DRenderTargetView;
};

class RenderTarget::D2D final {
 public:
  D2D() = delete;
  D2D(const D2D&) = delete;
  D2D(D2D&&) = delete;
  D2D& operator=(const D2D&) = delete;
  D2D& operator=(D2D&&) = delete;

  D2D(const std::shared_ptr<RenderTarget>&);
  ~D2D();

  ID2D1DeviceContext* operator->() const;
  operator ID2D1DeviceContext*() const;

  void Release();
  void Reacquire();

 private:
  std::shared_ptr<RenderTarget> mParent;
  RenderTarget* mUnsafeParent {nullptr};
  bool mReleased {false};
  bool mHDR {false};

  void Acquire();
};

class RenderTarget::D3D final {
 public:
  D3D() = delete;
  D3D(const D3D&) = delete;
  D3D(D3D&&) = delete;
  D3D& operator=(const D3D&) = delete;
  D3D& operator=(D3D&&) = delete;

  D3D(const std::shared_ptr<RenderTarget>&);
  ~D3D();

  ID3D11Texture2D* texture() const;
  ID3D11RenderTargetView* rtv() const;

 private:
  std::shared_ptr<RenderTarget> mParent;
  RenderTarget* mUnsafeParent {nullptr};
};

}// namespace OpenKneeboard