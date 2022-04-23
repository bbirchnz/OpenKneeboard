#pragma once

#include <OpenKneeboard/Button.h>

namespace OpenKneeboard {
class IconButton : public BaseButton {
 public:
  IconButton(
    winrt::com_ptr<ID2D1Brush> baseBrush,
    winrt::com_ptr<ID2D1Brush> hoverBrush,
    winrt::com_ptr<ID2D1Brush> activeBrush,
    D2D1_RECT_F buttonBounds,
    KneeboardState* kneeboard,
    Tab* tab,
    winrt::com_ptr<IDWriteFactory> dWrite,
    WCHAR iconChar);

 private:
  void RenderButton(ID2D1DeviceContext* ctx, ID2D1Brush* brush);
  WCHAR mIconChar;
  winrt::com_ptr<IDWriteTextFormat> mTextFormat;
};
}// namespace OpenKneeboard