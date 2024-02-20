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
#include <OpenKneeboard/DebugPrivileges.h>

#include <OpenKneeboard/dprint.h>

namespace OpenKneeboard {
static bool sHaveInstance = false;

DebugPrivileges::DebugPrivileges() {
  if (sHaveInstance) {
    OPENKNEEBOARD_LOG_AND_FATAL("Can't nest DebugPrivileges");
  }
  sHaveInstance = true;
  if (!OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
        mToken.put())) {
    dprint("WARNING: Failed to open own process token");
    return;
  }

  LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &mLuid);

  TOKEN_PRIVILEGES privileges;
  privileges.PrivilegeCount = 1;
  privileges.Privileges[0].Luid = mLuid;
  privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  AdjustTokenPrivileges(
    mToken.get(), false, &privileges, sizeof(privileges), nullptr, nullptr);
}

DebugPrivileges::~DebugPrivileges() {
  sHaveInstance = false;
  if (!mToken) {
    return;
  }
  TOKEN_PRIVILEGES privileges;
  privileges.PrivilegeCount = 1;
  privileges.Privileges[0].Luid = mLuid;
  privileges.Privileges[0].Attributes = SE_PRIVILEGE_REMOVED;
  AdjustTokenPrivileges(
    mToken.get(), false, &privileges, sizeof(privileges), nullptr, nullptr);
}

}// namespace OpenKneeboard