#include <OpenKneeboard/IconButton.h>

namespace OpenKneeboard {

IconButton::IconButton(
  winrt::com_ptr<ID2D1Brush> baseBrush,
  winrt::com_ptr<ID2D1Brush> hoverBrush,
  winrt::com_ptr<ID2D1Brush> activeBrush,
  D2D1_RECT_F buttonBounds,
  KneeboardState* kneeboard,
  Tab* tab,
  winrt::com_ptr<IDWriteFactory> dWrite,
  const wchar_t* icon)
  : TextButton {
    baseBrush,
    hoverBrush,
    activeBrush,
    buttonBounds,
    kneeboard,
    tab,
    dWrite,
    (const wchar_t*)L"Segoe MDL2 Assets",
    icon} {
}
}// namespace OpenKneeboard