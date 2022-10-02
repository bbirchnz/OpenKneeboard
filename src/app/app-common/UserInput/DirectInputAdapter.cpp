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
#include <OpenKneeboard/DirectInputAdapter.h>
#include <OpenKneeboard/DirectInputDevice.h>
#include <OpenKneeboard/DirectInputListener.h>
#include <OpenKneeboard/GetDirectInputDevices.h>
#include <OpenKneeboard/UserInputButtonBinding.h>
#include <OpenKneeboard/dprint.h>
#include <shims/winrt/base.h>

#include <thread>
#include <unordered_map>
#include <unordered_set>

// clang-format off
#include <Windows.h>
#include <Dbt.h>
#include <Rpc.h>
#include <dinput.h>
// clang-format on

using namespace OpenKneeboard;

namespace OpenKneeboard {

DirectInputAdapter* DirectInputAdapter::gInstance = nullptr;

DirectInputAdapter::DirectInputAdapter(
  HWND hwnd,
  const DirectInputSettings& settings)
  : mWindow(hwnd), mSettings(settings) {
  if (gInstance) {
    throw std::logic_error("Only one DirectInputAdapter at a time");
  }
  gInstance = this;

  mPreviousWindowProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(
    hwnd,
    GWLP_WNDPROC,
    reinterpret_cast<LONG_PTR>(&DirectInputAdapter::WindowProc)));
  winrt::check_pointer(mPreviousWindowProc);
  winrt::check_hresult(DirectInput8Create(
    GetModuleHandle(nullptr),
    DIRECTINPUT_VERSION,
    IID_IDirectInput8,
    mDI8.put_void(),
    NULL));

  this->Reload();
}

void DirectInputAdapter::LoadSettings(const DirectInputSettings& settings) {
  mSettings = settings;
  this->Reload();
}

void DirectInputAdapter::Reload() {
  this->RemoveAllEventListeners();

  for (auto& [id, device]: mDevices) {
    device.mListener.Cancel();
  }
  mDevices.clear();

  for (auto diDeviceInstance: GetDirectInputDevices(mDI8.get())) {
    auto device = DirectInputDevice::Create(diDeviceInstance);
    if (mSettings.mDevices.contains(device->GetID())) {
      std::vector<UserInputButtonBinding> bindings;
      for (const auto& binding:
           mSettings.mDevices.at(device->GetID()).mButtonBindings) {
        bindings.push_back({
          device,
          binding.mButtons,
          binding.mAction,
        });
      }
      device->SetButtonBindings(bindings);
    }

    mDevices.insert_or_assign(
      device->GetID(),
      DeviceState {
        .mDevice = device,
        .mListener = DirectInputListener::Run(mDI8, device),
      });

    AddEventListener(device->evUserActionEvent, this->evUserActionEvent);
    AddEventListener(
      device->evBindingsChangedEvent, this->evSettingsChangedEvent);
  }

  this->evAttachedControllersChangedEvent.Emit();
}

DirectInputAdapter::~DirectInputAdapter() {
  this->RemoveAllEventListeners();
  if (mPreviousWindowProc) {
    SetWindowLongPtrW(
      mWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(mPreviousWindowProc));
  }
  for (auto& [id, device]: mDevices) {
    device.mListener.Cancel();
  }
  gInstance = nullptr;
}

std::vector<std::shared_ptr<UserInputDevice>> DirectInputAdapter::GetDevices()
  const {
  std::vector<std::shared_ptr<UserInputDevice>> devices;
  for (const auto& [id, device]: mDevices) {
    devices.push_back(
      std::static_pointer_cast<UserInputDevice>(device.mDevice));
  }
  return devices;
}

DirectInputSettings DirectInputAdapter::GetSettings() const {
  for (const auto& [deviceID, state]: mDevices) {
    const auto& device = state.mDevice;
    const auto kind
      = (device->GetDIDeviceInstance().dwDevType & 0xff) == DI8DEVTYPE_KEYBOARD
      ? "Keyboard"
      : "GameController";

    auto& deviceSettings = mSettings.mDevices[deviceID];
    deviceSettings = {
      .mID = deviceID,
      .mName = device->GetName(),
      .mKind = kind,
      .mButtonBindings = {},
    };

    if (device->GetButtonBindings().empty()) {
      continue;
    }

    for (const auto& binding: device->GetButtonBindings()) {
      deviceSettings.mButtonBindings.push_back({
        .mButtons = binding.GetButtonIDs(),
        .mAction = binding.GetAction(),
      });
    }
  }

  return mSettings;
}

LRESULT CALLBACK DirectInputAdapter::WindowProc(
  _In_ HWND hwnd,
  _In_ UINT uMsg,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam) {
  if (uMsg == WM_DEVICECHANGE && wParam == DBT_DEVNODES_CHANGED) {
    dprint("Devices changed, reloading DirectInput device list");
    gInstance->Reload();
  }

  return CallWindowProc(
    gInstance->mPreviousWindowProc, hwnd, uMsg, wParam, lParam);
}

}// namespace OpenKneeboard
