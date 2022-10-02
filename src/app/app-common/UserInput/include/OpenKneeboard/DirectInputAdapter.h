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

#include <OpenKneeboard/DirectInputSettings.h>
#include <OpenKneeboard/Events.h>
#include <OpenKneeboard/UserAction.h>
#include <OpenKneeboard/json_fwd.h>
#include <shims/winrt/base.h>
#include <winrt/Windows.Foundation.h>

#include <memory>
#include <vector>

struct IDirectInput8W;

namespace OpenKneeboard {

class DirectInputDevice;
class UserInputButtonBinding;
class UserInputDevice;

class DirectInputAdapter final : private OpenKneeboard::EventReceiver {
 public:
  DirectInputAdapter() = delete;
  DirectInputAdapter(HWND mainWindow, const DirectInputSettings& settings);
  ~DirectInputAdapter();

  DirectInputSettings GetSettings() const;
  void LoadSettings(const DirectInputSettings& settings);
  std::vector<std::shared_ptr<UserInputDevice>> GetDevices() const;

  Event<UserAction> evUserActionEvent;
  Event<> evSettingsChangedEvent;
  Event<> evAttachedControllersChangedEvent;

 private:
  void Reload();

  HWND mWindow;
  WNDPROC mPreviousWindowProc;

  winrt::com_ptr<IDirectInput8W> mDI8;

  struct DeviceState {
    std::shared_ptr<DirectInputDevice> mDevice;
    winrt::Windows::Foundation::IAsyncAction mListener;
  };
  std::unordered_map<std::string, DeviceState> mDevices;

  mutable DirectInputSettings mSettings;

  static LRESULT CALLBACK WindowProc(
    _In_ HWND hwnd,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam);

  static DirectInputAdapter* gInstance;
};

}// namespace OpenKneeboard
