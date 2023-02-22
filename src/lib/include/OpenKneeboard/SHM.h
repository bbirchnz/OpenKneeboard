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

#include <OpenKneeboard/config.h>
#include <Windows.h>
#include <d3d11.h>
#include <shims/winrt/base.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numbers>
#include <optional>
#include <string>
#include <vector>

#include "FlatConfig.h"
#include "VRConfig.h"

namespace OpenKneeboard::SHM {

struct Header;

static constexpr DXGI_FORMAT SHARED_TEXTURE_PIXEL_FORMAT
  = DXGI_FORMAT_B8G8R8A8_UNORM;
static constexpr bool SHARED_TEXTURE_IS_PREMULTIPLIED_B8G8R8A8 = true;
static constexpr bool SHARED_TEXTURE_IS_PREMULTIPLIED = true;

std::wstring SharedTextureName(
  uint64_t sessionID,
  uint8_t layerIndex,
  uint32_t sequenceNumber);

constexpr UINT DEFAULT_D3D11_BIND_FLAGS
  = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
constexpr UINT DEFAULT_D3D11_MISC_FLAGS = 0;

winrt::com_ptr<ID3D11Texture2D> CreateCompatibleTexture(
  ID3D11Device*,
  UINT bindFlags = DEFAULT_D3D11_BIND_FLAGS,
  UINT miscFlags = DEFAULT_D3D11_MISC_FLAGS);

enum class ConsumerKind : uint32_t {
  SteamVR = 1 << 0,
  OpenXR = 1 << 1,
  OculusD3D11 = 1 << 2,
  OculusD3D12 = 1 << 3,
  NonVRD3D11 = 1 << 4,
  Test = ~(0ui32),
};

class ConsumerPattern final {
 public:
  ConsumerPattern();
  ConsumerPattern(std::underlying_type_t<ConsumerKind>(consumerKindMask));
  ConsumerPattern(ConsumerKind kind)
    : mKindMask(std::underlying_type_t<ConsumerKind>(kind)) {
  }

  bool Matches(ConsumerKind) const;

 private:
  std::underlying_type_t<ConsumerKind> mKindMask {0};
};

struct Config final {
  uint64_t mGlobalInputLayerID {};
  VRRenderConfig mVR {};
  FlatConfig mFlat {};
  ConsumerPattern mTarget {};
};
static_assert(std::is_standard_layout_v<Config>);
struct LayerConfig final {
  uint64_t mLayerID;
  uint16_t mImageWidth, mImageHeight;// Pixels
  VRLayerConfig mVR;

  bool IsValid() const;
};
static_assert(std::is_standard_layout_v<LayerConfig>);

class Impl;

class Writer final {
 public:
  Writer();
  ~Writer();

  operator bool() const;
  void Update(const Config& config, const std::vector<LayerConfig>& layers);

  UINT GetNextTextureIndex() const;

  uint64_t GetSessionID() const;
  uint32_t GetNextSequenceNumber() const;

  // "Lockable" C++ named concept: supports std::unique_lock
  void lock();
  bool try_lock();
  void unlock();

 private:
  class Impl;
  std::shared_ptr<Impl> p;
};

struct TextureReadResources;
struct LayerTextureReadResources;

class Snapshot final {
 private:
  std::shared_ptr<Header> mHeader;
  std::array<winrt::com_ptr<ID3D11Texture2D>, MaxLayers> mLayerTextures;

  using LayerSRVArray
    = std::array<winrt::com_ptr<ID3D11ShaderResourceView>, MaxLayers>;
  std::shared_ptr<LayerSRVArray> mLayerSRVs;

 public:
  Snapshot();
  Snapshot(const Header& header, ID3D11Device*, TextureReadResources*);
  ~Snapshot();

  /// Changes even if the feeder restarts with frame ID 0
  size_t GetRenderCacheKey() const;
  Config GetConfig() const;
  uint8_t GetLayerCount() const;
  const LayerConfig* GetLayerConfig(uint8_t layerIndex) const;
  winrt::com_ptr<ID3D11Texture2D> GetLayerTexture(
    ID3D11Device*,
    uint8_t layerIndex) const;
  winrt::com_ptr<ID3D11ShaderResourceView> GetLayerShaderResourceView(
    ID3D11Device*,
    uint8_t layerIndex) const;

  bool IsValid() const;

  // Use GetRenderCacheKey() instead for almost all purposes
  uint64_t GetSequenceNumberForDebuggingOnly() const;
};

class Reader final {
 public:
  Reader();
  ~Reader();

  operator bool() const;
  // Fetch a (possibly-cached) snapshot
  Snapshot MaybeGet(ID3D11Device*, ConsumerKind);
  /// Do not use for caching - use GetRenderCacheKey instead
  uint32_t GetFrameCountForMetricsOnly() const;

  /// Changes even if the feeder restarts with frame ID 0
  size_t GetRenderCacheKey() const;

 private:
  Snapshot MaybeGetUncached(ID3D11Device*, ConsumerKind) const;

  class Impl;
  std::shared_ptr<Impl> p;

  Snapshot mCache;
  ConsumerKind mCachedConsumerKind;
  uint64_t mCachedSequenceNumber {};
};

}// namespace OpenKneeboard::SHM
