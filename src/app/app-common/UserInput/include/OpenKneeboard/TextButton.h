#pragma once

#include <OpenKneeboard/Button.h>

namespace OpenKneeboard {

// A text button displays a simple button with the provided text and font.
// Font size is taken from bounds height, width is responsibility of the user
class TextButton : public BaseButton {
 public:
  TextButton(
    winrt::com_ptr<ID2D1Brush> baseBrush,
    winrt::com_ptr<ID2D1Brush> hoverBrush,
    winrt::com_ptr<ID2D1Brush> activeBrush,
    D2D1_RECT_F buttonBounds,
    KneeboardState* kneeboard,
    Tab* tab,
    winrt::com_ptr<IDWriteFactory> dWrite,
    const wchar_t* font,
    const wchar_t* text);

 protected:
  void RenderButton(ID2D1DeviceContext* ctx, ID2D1Brush* brush);
  const wchar_t* mText;
  winrt::com_ptr<IDWriteTextFormat> mTextFormat;
};
}// namespace OpenKneeboard