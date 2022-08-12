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

#include <OpenKneeboard/DirectInputDevice.h>
#include <OpenKneeboard/DirectInputKeyboardListener.h>
#include <OpenKneeboard/scope_guard.h>

namespace OpenKneeboard {

DirectInputKeyboardListener::DirectInputKeyboardListener(
  const winrt::com_ptr<IDirectInput8>& di,
  const std::shared_ptr<DirectInputDevice>& device)
  : DirectInputListener(di, device) {
  this->GetState(mState.size(), mState.data());
}

DirectInputKeyboardListener::~DirectInputKeyboardListener() = default;

void DirectInputKeyboardListener::Poll() {
  decltype(mState) newState {};
  this->GetState(newState.size(), &newState);
  scope_guard updateState([&]() { mState = newState; });

  auto device = this->GetDevice();
  for (uint8_t i = 0; i < newState.size(); ++i) {
    if (mState[i] != newState[i]) {
      device->PostButtonStateChange(
        i, static_cast<bool>(newState[i] & (1 << 7)));
    }
  }
}

}// namespace OpenKneeboard
