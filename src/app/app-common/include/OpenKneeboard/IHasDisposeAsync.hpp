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

#include <OpenKneeboard/StateMachine.hpp>

#include <shims/winrt/base.h>

#include <winrt/Windows.Foundation.h>

namespace OpenKneeboard {
class IHasDisposeAsync {
 public:
  // This is an alternative to `final_release()`, where consumers (or
  // subclasses) may need to be able to wait for async cleanup
  [[nodiscard]]
  virtual winrt::Windows::Foundation::IAsyncAction DisposeAsync() noexcept
    = 0;
};

}// namespace OpenKneeboard