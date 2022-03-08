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

#include <cstdint>
#include <numbers>

#include "bitflags.h"

namespace OpenKneeboard {

#pragma pack(push)
struct VRConfig {
  static constexpr uint16_t VERSION = 1;

  enum class Flags : uint32_t {
    HEADLOCKED = 1 << 0,
    DISCARD_DEPTH_INFORMATION = 1 << 1,
    PREFER_ROOMSCALE_POSITION = 1 << 2,
  };

  // Distances in meters, rotations in radians
  float x = 0.15f, floorY = 0.6f, eyeY = -0.7f, z = -0.4f;
  float rx = -2 * std::numbers::pi_v<float> / 5,
        ry = -std::numbers::pi_v<float> / 32, rz = 0.0f;
  float height = 0.25f;
  float zoomScale = 2.0f;

  Flags flags = Flags::DISCARD_DEPTH_INFORMATION;
};
#pragma pack(pop)

template <>
constexpr bool is_bitflags_v<VRConfig::Flags> = true;

}// namespace OpenKneeboard
