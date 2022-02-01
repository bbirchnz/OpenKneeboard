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

#define PACKETDATA (PK_X | PK_Y | PK_BUTTONS)
#define PACKETMODE 0

#include <OpenKneeboard/WintabTablet.h>
#include <OpenKneeboard/dprint.h>

// clang-format off
#include <wintab/WINTAB.h>
#include <wintab/PKTDEF.h>
// clang-format on

#define WINTAB_FUNCTIONS \
  IT(WTInfoW) \
  IT(WTOpenW) \
  IT(WTGetW) \
  IT(WTClose) \
  IT(WTPacket)

namespace {
class LibWintab {
 public:
#define IT(x) decltype(&x) x = nullptr;
  WINTAB_FUNCTIONS
#undef IT

  LibWintab() {
    mWintab = LoadLibraryA("Wintab32.dll");
    if (!mWintab) {
      return;
    }
#define IT(x) \
  this->x = reinterpret_cast<decltype(&::x)>(GetProcAddress(mWintab, #x));
    WINTAB_FUNCTIONS
#undef IT
  }

  ~LibWintab() {
    if (mWintab) {
      FreeLibrary(mWintab);
    }
  }

  operator bool() const {
    return mWintab != 0;
  }

  LibWintab(const LibWintab&) = delete;
  LibWintab& operator=(const LibWintab&) = delete;

 private:
  HMODULE mWintab = 0;
};
}// namespace

namespace OpenKneeboard {

struct WintabTablet::Impl {
  Impl(HWND window);
  ~Impl();

  State mState;
  LibWintab mWintab;
  HCTX mCtx;
};

WintabTablet::WintabTablet(HWND window) : p(std::make_unique<Impl>(window)) {
}

WintabTablet::~WintabTablet() {
}

WintabTablet::State WintabTablet::GetState() {
  if (!p) {
    return {};
  }
  return p->mState;
}

WintabTablet::operator bool() const {
  return p && p->mCtx;
}

WintabTablet::Impl::Impl(HWND window) {
  if (!mWintab) {
    return;
  }

  LOGCONTEXT logicalContext;
  mWintab.WTInfoW(WTI_DEFSYSCTX, 0, &logicalContext);
  logicalContext.lcPktData = PACKETDATA;
  logicalContext.lcMoveMask = PACKETDATA;
  logicalContext.lcPktMode = PACKETMODE;
  logicalContext.lcOptions = CXO_MESSAGES | CXO_CSRMESSAGES;

  AXIS axis;

  mWintab.WTInfoW(WTI_DEVICES, DVC_X, &axis);
  logicalContext.lcInOrgX = axis.axMin;
  logicalContext.lcInExtX = axis.axMax - axis.axMin;
  logicalContext.lcOutOrgX = 0;
  logicalContext.lcOutExtX = logicalContext.lcInExtX;

  mWintab.WTInfoW(WTI_DEVICES, DVC_Y, &axis);
  logicalContext.lcInOrgY = axis.axMin;
  logicalContext.lcInExtY = axis.axMax - axis.axMin;
  logicalContext.lcOutOrgY = 0;
  logicalContext.lcOutExtY = -logicalContext.lcInExtY;

  for (UINT i = 0, tag; mWintab.WTInfo(WTI_EXTENSIONS + i, EXT_TAG, &tag);
       ++i) {
    if (tag == WTX_EXPKEYS2 || tag == WTX_OBT) {
      UINT mask = 0;
      mWintab.WTInfo(WTI_EXTENSIONS + i, EXT_MASK, &mask);
      logicalContext.lcPktData |= mask;
      break;
    }
  }

  mCtx = mWintab.WTOpen(window, &logicalContext, true);
  if (!mCtx) {
    dprint("Failed to open wintab tablet");
    return;
  }
  dprint("Opened wintab tablet");
}

WintabTablet::Impl::~Impl() {
  if (!mCtx) {
    return;
  }
  mWintab.WTClose(mCtx);
}

bool WintabTablet::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == WT_PROXIMITY) {
    // high word indicates hardware events, low word indicates
    // context enter/leave
    p->mState.active = lParam & 0xffff;
    return true;
  }

  if (message == WT_PACKET) {
    PACKET packet;
    if (!p->mWintab.WTPacket(p->mCtx, static_cast<UINT>(wParam), &packet)) {
      return false;
    }
    p->mState.x = packet.pkX;
    p->mState.y = packet.pkY;
    return true;
  }

  return false;
}

}// namespace OpenKneeboard
