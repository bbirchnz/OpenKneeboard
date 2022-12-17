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
// clang-format off
#include "pch.h"
#include "AdvancedSettingsPage.xaml.h"
#include "AdvancedSettingsPage.g.cpp"
// clang-format on

#include <OpenKneeboard/AppSettings.h>
#include <OpenKneeboard/Elevation.h>
#include <OpenKneeboard/Filesystem.h>
#include <OpenKneeboard/KneeboardState.h>
#include <OpenKneeboard/LaunchURI.h>
#include <OpenKneeboard/RunSubprocessAsync.h>
#include <OpenKneeboard/RuntimeFiles.h>
#include <OpenKneeboard/TroubleshootingStore.h>
#include <OpenKneeboard/VRConfig.h>
#include <OpenKneeboard/config.h>
#include <OpenKneeboard/scope_guard.h>
#include <OpenKneeboard/utf8.h>
#include <OpenKneeboard/weak_wrap.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "Globals.h"

using namespace OpenKneeboard;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;

namespace winrt::OpenKneeboardApp::implementation {

AdvancedSettingsPage::AdvancedSettingsPage() {
  InitializeComponent();
}

winrt::event_token AdvancedSettingsPage::PropertyChanged(
  winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const&
    handler) {
  return mPropertyChangedEvent.add(handler);
}

void AdvancedSettingsPage::PropertyChanged(
  winrt::event_token const& token) noexcept {
  mPropertyChangedEvent.remove(token);
}

bool AdvancedSettingsPage::DualKneeboards() const noexcept {
  return gKneeboard->GetAppSettings().mDualKneeboards.mEnabled;
}

void AdvancedSettingsPage::DualKneeboards(bool value) noexcept {
  auto s = gKneeboard->GetAppSettings();
  s.mDualKneeboards.mEnabled = value;
  gKneeboard->SetAppSettings(s);
}

bool AdvancedSettingsPage::MultipleProfiles() const noexcept {
  return gKneeboard->GetProfileSettings().mEnabled;
}

void AdvancedSettingsPage::MultipleProfiles(bool value) noexcept {
  static bool sHaveShownTip = false;
  auto s = gKneeboard->GetProfileSettings();
  if (value && (!s.mEnabled) && !sHaveShownTip) {
    OpenKneeboard::LaunchURI("openkneeboard:///TeachingTips/ProfileSwitcher");
    sHaveShownTip = true;
  }
  s.mEnabled = value;
  gKneeboard->SetProfileSettings(s);
}

bool AdvancedSettingsPage::GazeInputFocus() const noexcept {
  return gKneeboard->GetVRSettings().mEnableGazeInputFocus;
}

void AdvancedSettingsPage::GazeInputFocus(bool enabled) noexcept {
  auto vrc = gKneeboard->GetVRSettings();
  vrc.mEnableGazeInputFocus = enabled;
  gKneeboard->SetVRSettings(vrc);
}

bool AdvancedSettingsPage::LoopPages() const noexcept {
  return gKneeboard->GetAppSettings().mLoopPages;
}

void AdvancedSettingsPage::LoopPages(bool value) noexcept {
  auto s = gKneeboard->GetAppSettings();
  s.mLoopPages = value;
  gKneeboard->SetAppSettings(s);
}

bool AdvancedSettingsPage::LoopTabs() const noexcept {
  return gKneeboard->GetAppSettings().mLoopTabs;
}

void AdvancedSettingsPage::LoopTabs(bool value) noexcept {
  auto s = gKneeboard->GetAppSettings();
  s.mLoopTabs = value;
  gKneeboard->SetAppSettings(s);
}

uint32_t AdvancedSettingsPage::MinimumPenRadius() {
  return gKneeboard->GetDoodlesSettings().mPen.mMinimumRadius;
}

void AdvancedSettingsPage::MinimumPenRadius(uint32_t value) {
  auto ds = gKneeboard->GetDoodlesSettings();
  ds.mPen.mMinimumRadius = value;
  gKneeboard->SetDoodlesSettings(ds);
}

uint32_t AdvancedSettingsPage::PenSensitivity() {
  return gKneeboard->GetDoodlesSettings().mPen.mSensitivity;
}

void AdvancedSettingsPage::PenSensitivity(uint32_t value) {
  auto ds = gKneeboard->GetDoodlesSettings();
  ds.mPen.mSensitivity = value;
  gKneeboard->SetDoodlesSettings(ds);
}

uint32_t AdvancedSettingsPage::MinimumEraseRadius() {
  return gKneeboard->GetDoodlesSettings().mEraser.mMinimumRadius;
}

void AdvancedSettingsPage::MinimumEraseRadius(uint32_t value) {
  auto ds = gKneeboard->GetDoodlesSettings();
  ds.mEraser.mMinimumRadius = value;
  gKneeboard->SetDoodlesSettings(ds);
}

uint32_t AdvancedSettingsPage::EraseSensitivity() {
  return gKneeboard->GetDoodlesSettings().mEraser.mSensitivity;
}

void AdvancedSettingsPage::EraseSensitivity(uint32_t value) {
  auto ds = gKneeboard->GetDoodlesSettings();
  ds.mEraser.mSensitivity = value;
  gKneeboard->SetDoodlesSettings(ds);
}

void AdvancedSettingsPage::RestoreDoodleDefaults(
  const IInspectable&,
  const IInspectable&) noexcept {
  gKneeboard->ResetDoodlesSettings();

  for (auto prop: {
         L"MinimumPenRadius",
         L"PenSensitivity",
         L"MinimumEraseRadius",
         L"EraseSensitivity",
       }) {
    mPropertyChangedEvent(*this, PropertyChangedEventArgs(prop));
  }
}

void AdvancedSettingsPage::RestoreQuirkDefaults(
  const IInspectable&,
  const IInspectable&) noexcept {
  auto vr = gKneeboard->GetVRSettings();
  vr.mQuirks = {};
  gKneeboard->SetVRSettings(vr);
  for (auto prop: {
         L"Quirk_OculusSDK_DiscardDepthInformation",
         L"Quirk_Varjo_OpenXR_InvertYPosition",
         L"Quirk_Varjo_OpenXR_D3D12_DoubleBuffer",
       }) {
    mPropertyChangedEvent(*this, PropertyChangedEventArgs(prop));
  }
}

bool AdvancedSettingsPage::Quirk_OculusSDK_DiscardDepthInformation()
  const noexcept {
  return gKneeboard->GetVRSettings().mQuirks.mOculusSDK_DiscardDepthInformation;
}

void AdvancedSettingsPage::Quirk_OculusSDK_DiscardDepthInformation(
  bool value) noexcept {
  auto vrc = gKneeboard->GetVRSettings();
  vrc.mQuirks.mOculusSDK_DiscardDepthInformation = value;
  gKneeboard->SetVRSettings(vrc);
}

bool AdvancedSettingsPage::Quirk_Varjo_OpenXR_InvertYPosition() const noexcept {
  return gKneeboard->GetVRSettings().mQuirks.mVarjo_OpenXR_InvertYPosition;
}

void AdvancedSettingsPage::Quirk_Varjo_OpenXR_InvertYPosition(
  bool value) noexcept {
  auto vrc = gKneeboard->GetVRSettings();
  vrc.mQuirks.mVarjo_OpenXR_InvertYPosition = value;
  gKneeboard->SetVRSettings(vrc);
}

bool AdvancedSettingsPage::Quirk_Varjo_OpenXR_D3D12_DoubleBuffer()
  const noexcept {
  return gKneeboard->GetVRSettings().mQuirks.mVarjo_OpenXR_D3D12_DoubleBuffer;
}

void AdvancedSettingsPage::Quirk_Varjo_OpenXR_D3D12_DoubleBuffer(
  bool value) noexcept {
  auto vrc = gKneeboard->GetVRSettings();
  vrc.mQuirks.mVarjo_OpenXR_D3D12_DoubleBuffer;
  gKneeboard->SetVRSettings(vrc);
}

bool AdvancedSettingsPage::CanChangeElevation() const noexcept {
  return !IsShellElevated();
}

int32_t AdvancedSettingsPage::DesiredElevation() const noexcept {
  return static_cast<int32_t>(GetDesiredElevation());
}

winrt::fire_and_forget AdvancedSettingsPage::DesiredElevation(
  int32_t value) noexcept {
  if (value == DesiredElevation()) {
    co_return;
  }

  const scope_guard propertyChanged(weak_wrap(
    [](const auto& self) -> winrt::fire_and_forget {
      co_await self->mUIThread;
      self->mPropertyChangedEvent(
        *self, PropertyChangedEventArgs(L"DesiredElevation"));
    },
    this));

  // Always use the helper; while it's not needed, it never hurts, and gives us
  // a single path to act

  const auto path = (Filesystem::GetRuntimeDirectory()
                     / RuntimeFiles::SET_DESIRED_ELEVATION_HELPER)
                      .wstring();

  const auto cmdline = std::format(L"{}", value);
  if (
    co_await RunSubprocessAsync(path, cmdline, RunAs::Administrator)
    != SubprocessResult::Success) {
    co_return;
  }
  if (IsShellElevated()) {
    // The UI should have been disabled
    OPENKNEEBOARD_BREAK;
    co_return;
  }

  const auto desired = GetDesiredElevation();

  const auto isElevated = IsElevated();
  const bool elevate = (!isElevated) && desired == DesiredElevation::Elevated;
  const bool deElevate
    = (isElevated) && desired == DesiredElevation::NotElevated;
  if (!(elevate || deElevate)) {
    co_return;
  }

  co_await mUIThread;
  ContentDialog dialog;
  dialog.XamlRoot(this->XamlRoot());
  dialog.Title(box_value(to_hstring(_(L"Restart OpenKneeboard?"))));
  dialog.Content(
    box_value(to_hstring(_(L"OpenKneeboard needs to be restarted to change "
                           L"elevation. Would you like to restart it now?"))));
  dialog.PrimaryButtonText(_(L"Restart Now"));
  dialog.CloseButtonText(_(L"Later"));
  dialog.DefaultButton(ContentDialogButton::Primary);

  if (co_await dialog.ShowAsync() != ContentDialogResult::Primary) {
    co_return;
  }

  dprint("Tearing down exclusive resources for relaunch");
  gMutex = {};
  gTroubleshootingStore = {};
  gKneeboard->ReleaseExclusiveResources();

  const auto restarted = RelaunchWithDesiredElevation(desired, SW_SHOWNORMAL);
  if (restarted) {
    Application::Current().Exit();
    co_return;
  }

  // Failed to spawn, e.g. UAC deny
  gTroubleshootingStore = TroubleshootingStore::Get();
  dprint("Relaunch failed, coming back up!");
  gMutex = {CreateMutexW(nullptr, TRUE, OpenKneeboard::ProjectNameW)};
  gKneeboard->AcquireExclusiveResources();
}

}// namespace winrt::OpenKneeboardApp::implementation
