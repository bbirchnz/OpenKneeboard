#include <OpenKneeboard/TextButton.h>

namespace OpenKneeboard {

TextButton::TextButton(
  winrt::com_ptr<ID2D1Brush> baseBrush,
  winrt::com_ptr<ID2D1Brush> hoverBrush,
  winrt::com_ptr<ID2D1Brush> activeBrush,
  D2D1_RECT_F buttonBounds,
  KneeboardState* kneeboard,
  Tab* tab,
  winrt::com_ptr<IDWriteFactory> dWrite,
  const wchar_t* font,
  const wchar_t* text)
  : BaseButton {
    baseBrush,
    hoverBrush,
    activeBrush,
    buttonBounds,
    kneeboard,
    tab} {
  mText = text;

  // setup text format:
  dWrite->CreateTextFormat(
    font,
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

void TextButton::RenderButton(ID2D1DeviceContext* ctx, ID2D1Brush* brush) {
  ctx->DrawTextW(
    mText,
    static_cast<UINT32>(wcslen(mText)),
    mTextFormat.get(),
    &mButtonBounds,
    brush);
}
}// namespace OpenKneeboard