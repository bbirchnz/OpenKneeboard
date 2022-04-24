#pragma once

#include <OpenKneeboard/IconButton.h>
#include <OpenKneeboard/TextButton.h>

#include "OpenKneeboard/DXResources.h"
#include "OpenKneeboard/Events.h"
#include "OpenKneeboard/KneeboardState.h"
#include "Tab.h"

namespace OpenKneeboard {

class ButtonTestTab final : public Tab, EventReceiver {
 public:
  ButtonTestTab(const DXResources& dxr, KneeboardState* kbs);
  virtual utf8_string GetTitle() const override;
  void Reload();

  virtual uint16_t GetPageCount() const override;
  D2D1_SIZE_U GetNativeContentSize(uint16_t pageIndex);
  void
  RenderPage(ID2D1DeviceContext*, uint16_t pageIndex, const D2D1_RECT_F& rect);

 private:
  static constexpr int RENDER_SCALE = 1;
  DXResources mDXR;
  KneeboardState* mKneeboard;
  IconButton* mTestButton;
  IconButton* mTestButton2;
  TextButton* mTestButton3;
  winrt::com_ptr<IDWriteTextFormat> mTextFormat;
  bool mButton1Toggle;
  bool mButton2Toggle;
  void Button1Toggle();
  void Button2Toggle();
};

}// namespace OpenKneeboard
