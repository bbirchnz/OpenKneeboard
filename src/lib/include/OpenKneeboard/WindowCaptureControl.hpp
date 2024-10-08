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

#include <OpenKneeboard/handles.hpp>

namespace OpenKneeboard::WindowCaptureControl {

constexpr const wchar_t WindowMessageName[] {
  L"OpenKneeboard_WindowCaptureControl"};

enum class WParam : unsigned int {
  Initialize = 1,// LPARAM: target top-level window
  StartInjection = 2,// LPARAM: target top-level window
  EndInjection = 3,
};

struct Handles {
  unique_hmodule mLibrary;
  unique_hhook mMessageHook;
  unique_hhook mWindowProcHook;

  bool IsValid() const;
};
Handles InstallHooks(HWND hwnd);

}// namespace OpenKneeboard::WindowCaptureControl
