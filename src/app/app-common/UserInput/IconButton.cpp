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
  WCHAR iconChar)
  : BaseButton {
    baseBrush,
    hoverBrush,
    activeBrush,
    buttonBounds,
    kneeboard,
    tab} {
  mIconChar = iconChar;

  // setup text format:
  dWrite->CreateTextFormat(
    L"Segoe MDL2 Assets",
    nullptr,
    DWRITE_FONT_WEIGHT_NORMAL,
    DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STRETCH_NORMAL,
    // this should be relative to button bounds:
    (buttonBounds.bottom - buttonBounds.top) * 0.9f,
    L"",
    mTextFormat.put());

  mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

void IconButton::RenderButton(ID2D1DeviceContext* ctx, ID2D1Brush* brush) {
  ctx->DrawTextW(&mIconChar, 1UL, mTextFormat.get(), &mButtonBounds, brush);
}
}// namespace OpenKneeboard