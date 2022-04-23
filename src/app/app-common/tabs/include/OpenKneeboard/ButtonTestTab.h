#pragma once

#include <OpenKneeboard/IconButton.h>

#include "OpenKneeboard/DXResources.h"
#include "OpenKneeboard/KneeboardState.h"
#include "Tab.h"

namespace OpenKneeboard {

class ButtonTestTab final : public Tab {
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
};

}// namespace OpenKneeboard
