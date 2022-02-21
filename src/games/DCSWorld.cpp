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
#include <OpenKneeboard/Games/DCSWorld.h>
#include <OpenKneeboard/dprint.h>
#include <OpenKneeboard/utf8.h>
#include <ShlObj.h>
#include <Windows.h>
#include <fmt/format.h>
#include <shims/winrt.h>

namespace OpenKneeboard::Games {

static std::filesystem::path GetDCSPath(const char* lastSubKey) {
  const auto subkey = fmt::format("SOFTWARE\\Eagle Dynamics\\{}", lastSubKey);
  const auto wSubKey = winrt::to_hstring(subkey);

  wchar_t buffer[MAX_PATH];
  DWORD length = sizeof(buffer) * sizeof(buffer[0]);

  if (
    RegGetValueW(
      HKEY_CURRENT_USER,
      wSubKey.c_str(),
      L"Path",
      RRF_RT_REG_SZ,
      nullptr,
      reinterpret_cast<void*>(buffer),
      &length)
    != ERROR_SUCCESS) {
    return {};
  }

  const auto path = std::filesystem::path(std::wstring_view(buffer, length / sizeof(buffer[0])));
  if (!std::filesystem::is_directory(path)) {
    return {};
  }

  return std::filesystem::canonical(path);
}

static std::filesystem::path GetSavedGamesPath() {
  static std::filesystem::path sPath;
  if (!sPath.empty()) {
    return sPath;
  }

  wchar_t* buffer = nullptr;
  if (
    SHGetKnownFolderPath(FOLDERID_SavedGames, NULL, NULL, &buffer) == S_OK
    && buffer) {
    sPath = std::filesystem::canonical(std::wstring_view(buffer));
  }
  return sPath;
}

std::filesystem::path DCSWorld::GetInstalledPath(Version version) {
  switch (version) {
    case Version::OPEN_BETA:
      return GetDCSPath("DCS World OpenBeta");
    case Version::STABLE:
      return GetDCSPath("DCS World");
  }
  return {};
}

std::filesystem::path DCSWorld::GetSavedGamesPath(Version version) {
  switch (version) {
    case Version::OPEN_BETA:
      return Games::GetSavedGamesPath() / "DCS.openbeta";
    case Version::STABLE:
      return Games::GetSavedGamesPath() / "DCS";
  }
  return {};
}

std::string DCSWorld::GetUserFriendlyName(
  const std::filesystem::path& _path) const {
  const auto path = std::filesystem::canonical(_path);
  if (path == GetInstalledPath(Version::OPEN_BETA) / "bin" / "DCS.exe") {
    return _("DCS World - Open Beta");
  }
  if (path == GetInstalledPath(Version::STABLE) / "bin" / "DCS.exe") {
    return _("DCS World - Stable");
  }
  return _("DCS World");
}

std::vector<std::filesystem::path> DCSWorld::GetInstalledPaths() const {
  std::vector<std::filesystem::path> ret;
  for (const auto& path: {
         GetInstalledPath(Version::OPEN_BETA),
         GetInstalledPath(Version::STABLE),
       }) {
    if (!std::filesystem::is_directory(path)) {
      continue;
    }
    auto exe = path / "bin" / "DCS.exe";
    if (std::filesystem::is_regular_file(exe)) {
      ret.push_back(exe);
    }
  }
  return ret;
}

bool DCSWorld::DiscardOculusDepthInformationDefault() const {
  return true;
}

const char* DCSWorld::GetNameForConfigFile() const {
  return "DCSWorld";
}

}// namespace OpenKneeboard::Games
