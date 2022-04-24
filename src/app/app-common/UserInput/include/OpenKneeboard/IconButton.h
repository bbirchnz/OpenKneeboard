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

  // selection of useful icons, see
  // https://docs.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font
  static constexpr wchar_t I_NAV_GLOBAL_NAVIGATION_BUTTON = L'\xE700';
  static constexpr wchar_t I_CANCEL = L'\xE711';
  static constexpr wchar_t I_MAP_PIN = L'\xE707';
  static constexpr wchar_t I_CHEVRON_DOWN = L'\xE70D';
  static constexpr wchar_t I_CHEVRON_UP = L'\xE70E';

 private:
  void RenderButton(ID2D1DeviceContext* ctx, ID2D1Brush* brush);
  WCHAR mIconChar;
  winrt::com_ptr<IDWriteTextFormat> mTextFormat;
};
}// namespace OpenKneeboard