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

#include <OpenKneeboard/json_fwd.h>
#include <OpenKneeboard/utf8.h>

#include <cstddef>
#include <string>
#include <vector>

namespace OpenKneeboard {
struct GameEvent final {
  utf8_string name;
  utf8_string value;

  template <class T>
  T ParsedValue() const {
    if (name != T::ID) {
      throw std::logic_error("Parse type does not match event name");
    }
    return nlohmann::json::parse(this->value);
  }

  template <class T>
  static GameEvent FromStruct(const T& v) {
    nlohmann::json j;
    j = v;
    return {T::ID, j.dump()};
  }

  operator bool() const;

  static GameEvent Unserialize(const std::vector<std::byte>& payload);
  std::vector<std::byte> Serialize() const;
  void Send() const;

  static const char* GetMailslotPath();

  static constexpr std::string_view EVT_REMOTE_USER_ACTION
    = "com.fredemmott.openkneeboard/RemoteUserAction";

  static constexpr std::string_view EVT_SET_INPUT_FOCUS
    = "com.fredemmott.openkneeboard/SetInputFocus";

  static constexpr std::string_view EVT_SET_TAB_BY_ID
    = "com.fredemmott.openkneeboard/SetTabByID";
  static constexpr std::string_view EVT_SET_TAB_BY_NAME
    = "com.fredemmott.openkneeboard/SetTabByName";
};

// Use a struct to allow specifying which kneeboard in the future
struct SetTabByIDEvent {
  static constexpr auto ID {GameEvent::EVT_SET_TAB_BY_ID};
  std::string mID;
  // 0 = no change
  uint64_t mPageNumber {0};
  // 0 = 'active', 1 = primary, 2 = secondary
  uint8_t mKneeboard {0};
};
OPENKNEEBOARD_DECLARE_JSON(SetTabByIDEvent);

// Use a struct to allow specifying which kneeboard in the future
struct SetTabByNameEvent {
  static constexpr auto ID {GameEvent::EVT_SET_TAB_BY_NAME};
  std::string mName;
  // 0 = no change
  uint64_t mPageNumber {0};
  // 0 = 'active', 1 = primary, 2 = secondary
  uint8_t mKneeboard {0};
};
OPENKNEEBOARD_DECLARE_JSON(SetTabByNameEvent);

}// namespace OpenKneeboard
