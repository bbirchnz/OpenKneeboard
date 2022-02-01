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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <OpenKneeboard/GameEvent.h>
#include <OpenKneeboard/WintabTablet.h>
#include <d3d11.h>
#include <wx/frame.h>

#include <memory>

#include "Settings.h"
#include "okOpenVRThread.h"
#include "okSHMRenderer.h"
#include "okTab.h"
#include "okTabsList.h"

class wxBookCtrlEvent;
class wxNotebook;

class okMainWindow final : public wxFrame {
 public:
  okMainWindow();
  virtual ~okMainWindow();

 protected:
  virtual WXLRESULT
  MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) override;

 private:
  void OnExit(wxCommandEvent&);
  void OnGameEvent(wxThreadEvent&);
  void OnShowSettings(wxCommandEvent&);

  void OnPreviousTab(wxCommandEvent&);
  void OnNextTab(wxCommandEvent&);
  void OnPreviousPage(wxCommandEvent&);
  void OnNextPage(wxCommandEvent&);
  void OnToggleVisibility(wxCommandEvent&);

  void OnTabChanged(wxBookCtrlEvent&);

  void UpdateTabs();
  void UpdateSHM();

  winrt::com_ptr<ID3D11Device> mD3D;
  std::vector<okConfigurableComponent*> mConfigurables;
  std::vector<okTab*> mTabUIs;
  wxNotebook* mNotebook = nullptr;
  okTabsList* mTabsList = nullptr;
  int mCurrentTab = -1;
  Settings mSettings = Settings::Load();

  std::unique_ptr<okSHMRenderer> mSHMRenderer;
  std::unique_ptr<OpenKneeboard::WintabTablet> mTablet;
};
