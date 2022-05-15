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
#include <OpenKneeboard/bitflags.h>
#include <OpenKneeboard/config.h>
#include <OpenKneeboard/shm.h>
#include <Windows.h>
#include <d3d11_2.h>
#include <dxgi1_2.h>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/xchar.h>

#include <bit>
#include <random>

namespace OpenKneeboard::SHM {

static uint64_t CreateSessionID() {
  std::random_device randDevice;
  std::uniform_int_distribution<uint32_t> randDist;
  return (static_cast<uint64_t>(GetCurrentProcessId()) << 32)
    | randDist(randDevice);
}

enum class HeaderFlags : ULONG {
  LOCKED = 1 << 0,
  FEEDER_ATTACHED = 1 << 1,
};

#pragma pack(push)
struct Header final {
  static constexpr uint16_t VERSION = 2;

  uint32_t sequenceNumber = 0;
  uint64_t sessionID = CreateSessionID();
  HeaderFlags flags;
  Config config;
};
#pragma pack(pop)

}// namespace OpenKneeboard::SHM

namespace OpenKneeboard {
template <>
constexpr bool is_bitflags_v<SHM::HeaderFlags> = true;
}// namespace OpenKneeboard

namespace OpenKneeboard::SHM {

namespace {

class Spinlock final {
 private:
  Header* mHeader = nullptr;

 public:
  enum FAILURE_BEHAVIOR {
    ON_FAILURE_FORCE_UNLOCK,
    ON_FAILURE_CREATE_FALSEY,
    ON_FAILURE_THROW_EXCEPTION
  };

  Spinlock(Header* header, FAILURE_BEHAVIOR behavior) : mHeader(header) {
    const auto bit = std::countr_zero(static_cast<ULONG>(HeaderFlags::LOCKED));
    static_assert(static_cast<ULONG>(HeaderFlags::LOCKED) == 1 << bit);

    for (int count = 1; count <= 1000; ++count) {
      auto alreadyLocked
        = _interlockedbittestandset(std::bit_cast<LONG*>(&header->flags), bit);
      if (!alreadyLocked) {
        return;
      }
      if (count % 10 == 0) {
        YieldProcessor();
      }
      if (count % 100 == 0) {
        Sleep(10 /* ms */);
      }
    }

    if (behavior == ON_FAILURE_FORCE_UNLOCK) {
      return;
    }

    if (behavior == ON_FAILURE_CREATE_FALSEY) {
      mHeader = nullptr;
      return;
    }

    throw new std::runtime_error("Failed to acquire spinlock");
  }

  operator bool() {
    return (bool)mHeader;
  }

  ~Spinlock() {
    if (!mHeader) {
      return;
    }

    mHeader->flags &= ~HeaderFlags::LOCKED;
  }
};

constexpr DWORD MAX_IMAGE_PX(1024 * 1024 * 8);
constexpr DWORD SHM_SIZE = sizeof(Header);

constexpr auto SHMPath() {
  char buf[255];
  auto end = fmt::format_to(
    buf,
    FMT_COMPILE("{}/h{}-c{}-vrc{}-fc{}-s{:x}"),
    ProjectNameA,
    Header::VERSION,
    Config::VERSION,
    VRRenderConfig::VERSION,
    FlatConfig::VERSION,
    SHM_SIZE);
  return std::string(buf, end);
}

UINT GetTextureKeyFromSequenceNumber(uint32_t sequenceNumber) {
  return (sequenceNumber % (std::numeric_limits<UINT>::max() - 1)) + 1;
}

}// namespace

struct TextureReadResources {
  winrt::com_ptr<ID3D11Texture2D> mTexture;
  winrt::com_ptr<IDXGISurface> mSurface;
  winrt::com_ptr<ID3D11ShaderResourceView> mShaderResourceView;
  winrt::com_ptr<IDXGIKeyedMutex> mMutex;
  uint64_t mSessionID = 0;

  void Populate(ID3D11Device* d3d, uint64_t sessionID, uint32_t sequenceNumber);
};

void TextureReadResources::Populate(
  ID3D11Device* d3d,
  uint64_t sessionID,
  uint32_t sequenceNumber) {
  if (sessionID != mSessionID) {
    *this = {};
  }

  if (mTexture) {
    return;
  }

  auto textureName = SHM::SharedTextureName(sessionID, sequenceNumber);

  ID3D11Device1* d1 = nullptr;
  d3d->QueryInterface(&d1);
  if (!d1) {
    return;
  }

  d1->OpenSharedResourceByName(
    textureName.c_str(),
    DXGI_SHARED_RESOURCE_READ,
    IID_PPV_ARGS(mTexture.put()));
  if (!mTexture) {
    return;
  }

  mSurface = mTexture.as<IDXGISurface>();
  mMutex = mTexture.as<IDXGIKeyedMutex>();

  d1->CreateShaderResourceView(
    mTexture.get(), nullptr, mShaderResourceView.put());
}

std::wstring SharedTextureName(uint64_t sessionID, uint32_t sequenceNumber) {
  return fmt::format(
    L"Local\\{}-texture-h{}-c{}-{:x}-{}",
    ProjectNameW,
    Header::VERSION,
    Config::VERSION,
    sessionID,
    sequenceNumber % TextureCount);
}

SharedTexture::SharedTexture() {
}

SharedTexture::SharedTexture(
  const Header& header,
  ID3D11Device* d3d,
  TextureReadResources* r)
  : mResources(r) {
  r->Populate(d3d, header.sessionID, header.sequenceNumber);
  auto key = GetTextureKeyFromSequenceNumber(header.sequenceNumber);
  if (r->mMutex->AcquireSync(key, 10) != S_OK) {
    mResources = nullptr;
    return;
  }
  mKey = key;
}

SharedTexture::~SharedTexture() {
  if (mKey == 0) {
    return;
  }
  if (!mResources) {
    return;
  }
  mResources->mMutex->ReleaseSync(mKey);
}

SharedTexture::operator bool() const {
  return mResources && mKey != 0;
}

ID3D11Texture2D* SharedTexture::GetTexture() const {
  return mResources->mTexture.get();
}

IDXGISurface* SharedTexture::GetSurface() const {
  return mResources->mSurface.get();
}

ID3D11ShaderResourceView* SharedTexture::GetShaderResourceView() const {
  return mResources->mShaderResourceView.get();
}

Snapshot::Snapshot() {
}

Snapshot::Snapshot(const Header& header, TextureReadResources* r)
  : mResources(r) {
  mHeader = std::make_unique<Header>(header);
}

Snapshot::~Snapshot() {
}

uint32_t Snapshot::GetSequenceNumber() const {
  return mHeader->sequenceNumber;
}

SharedTexture Snapshot::GetSharedTexture(ID3D11Device* d3d) const {
  if (mHeader) {
    return SharedTexture(*mHeader, d3d, mResources);
  }
  return {};
}

Config Snapshot::GetConfig() const {
  if (!*this) {
    return {};
  }
  return mHeader->config;
}

Snapshot::operator bool() const {
  return mHeader && (mHeader->flags & HeaderFlags::FEEDER_ATTACHED)
    && mHeader->config.mImageWidth > 0 && mHeader->config.mImageHeight > 0;
}

class Impl {
 public:
  HANDLE mHandle;
  std::byte* mMapping = nullptr;
  Header* mHeader = nullptr;

  ~Impl() {
    UnmapViewOfFile(mMapping);
    CloseHandle(mHandle);
  }
};

class Writer::Impl : public SHM::Impl {
 public:
  using SHM::Impl::Impl;
  bool mHaveFed = false;

  ~Impl() {
    mHeader->flags &= ~HeaderFlags::FEEDER_ATTACHED;
    FlushViewOfFile(mMapping, NULL);
  }
};

Writer::Writer() {
  const auto path = SHMPath();

  HANDLE handle;
  handle = CreateFileMappingA(
    INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHM_SIZE, path.c_str());
  if (!handle) {
    return;
  }
  auto mapping = reinterpret_cast<std::byte*>(
    MapViewOfFile(handle, FILE_MAP_WRITE, 0, 0, SHM_SIZE));
  if (!mapping) {
    CloseHandle(handle);
    return;
  }

  p = std::make_shared<Impl>();
  p->mHandle = handle;
  p->mMapping = mapping;
  p->mHeader = reinterpret_cast<Header*>(mapping);
  *p->mHeader = {};
}

Writer::~Writer() {
}

UINT Writer::GetPreviousTextureKey() const {
  return GetTextureKeyFromSequenceNumber(p->mHeader->sequenceNumber);
}

UINT Writer::GetNextTextureKey() const {
  return GetTextureKeyFromSequenceNumber(p->mHeader->sequenceNumber + 1);
}

UINT Writer::GetNextTextureIndex() const {
  return (p->mHeader->sequenceNumber + 1) % TextureCount;
}

uint64_t Writer::GetSessionID() const {
  return p->mHeader->sessionID;
}

uint32_t Writer::GetNextSequenceNumber() const {
  return p->mHeader->sequenceNumber + 1;
}

class Reader::Impl : public SHM::Impl {
 public:
  std::vector<TextureReadResources> mResources;
};

Reader::Reader() {
  const auto path = SHMPath();
  HANDLE handle;
  handle = CreateFileMappingA(
    INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHM_SIZE, path.c_str());
  if (!handle) {
    return;
  }
  auto mapping = reinterpret_cast<std::byte*>(
    MapViewOfFile(handle, FILE_MAP_WRITE, 0, 0, SHM_SIZE));
  if (!mapping) {
    CloseHandle(handle);
    return;
  }

  this->p = std::make_shared<Impl>();
  p->mHandle = handle;
  p->mMapping = mapping;
  p->mHeader = reinterpret_cast<Header*>(mapping);
  p->mResources = {TextureCount};
}

Reader::~Reader() {
}

Reader::operator bool() const {
  return p && (p->mHeader->flags & HeaderFlags::FEEDER_ATTACHED);
}

Writer::operator bool() const {
  return (bool)p;
}

uint32_t Reader::GetSequenceNumber() const {
  if (!(p && p->mHeader)) {
    return 0;
  }
  return p->mHeader->sequenceNumber;
}

Snapshot Reader::MaybeGet() const {
  Spinlock lock(p->mHeader, Spinlock::ON_FAILURE_CREATE_FALSEY);
  if (!lock) {
    return {};
  }

  return Snapshot(
    *p->mHeader, &p->mResources.at(p->mHeader->sequenceNumber % TextureCount));
}

void Writer::Update(const Config& config) {
  if (!p) {
    throw std::logic_error("Attempted to update invalid SHM");
  }

  if (config.mImageWidth == 0 || config.mImageHeight == 0) {
    throw std::logic_error("Not feeding a 0-size image");
  }

  Spinlock lock(p->mHeader, Spinlock::ON_FAILURE_FORCE_UNLOCK);
  p->mHeader->config = config;
  p->mHeader->sequenceNumber++;
  p->mHeader->flags |= HeaderFlags::FEEDER_ATTACHED;
}

}// namespace OpenKneeboard::SHM
