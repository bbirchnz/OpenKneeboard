#include <OpenKneeboard/ButtonTestTab.h>
#include <OpenKneeboard/IconButton.h>
#include <dwrite.h>

namespace OpenKneeboard {
ButtonTestTab::ButtonTestTab(const DXResources& dxr, KneeboardState* kbs) {
  mDXR = dxr;
  mKneeboard = kbs;

  winrt::com_ptr<ID2D1Brush> mButtonBrush;
  winrt::com_ptr<ID2D1Brush> mHoverButtonBrush;
  winrt::com_ptr<ID2D1Brush> mActiveButtonBrush;

  dxr.mD2DDeviceContext->CreateSolidColorBrush(
    {0.4f, 0.4f, 0.4f, 1.0f},
    D2D1::BrushProperties(),
    reinterpret_cast<ID2D1SolidColorBrush**>(mButtonBrush.put()));
  dxr.mD2DDeviceContext->CreateSolidColorBrush(
    {0.0f, 0.8f, 1.0f, 1.0f},
    D2D1::BrushProperties(),
    reinterpret_cast<ID2D1SolidColorBrush**>(mHoverButtonBrush.put()));
  dxr.mD2DDeviceContext->CreateSolidColorBrush(
    {0.0f, 0.0f, 0.0f, 1.0f},
    D2D1::BrushProperties(),
    reinterpret_cast<ID2D1SolidColorBrush**>(mActiveButtonBrush.put()));

  D2D1_RECT_F bounds = {.left = 50, .top = 50, .right = 100, .bottom = 100};
  auto button = new IconButton(
    mButtonBrush,
    mHoverButtonBrush,
    mActiveButtonBrush,
    bounds,
    mKneeboard,
    this,
    mDXR.mDWriteFactory,
    IconButton::I_MAP_PIN);
  mTestButton = button;

  bounds = {.left = 150, .top = 50, .right = 250, .bottom = 150};
  button = new IconButton(
    mButtonBrush,
    mHoverButtonBrush,
    mActiveButtonBrush,
    bounds,
    mKneeboard,
    this,
    mDXR.mDWriteFactory,
    IconButton::I_CANCEL);
  mTestButton2 = button;

  bounds = {.left = 350, .top = 50, .right = 450, .bottom = 150};
  mTestButton3 = new TextButton(
    mButtonBrush,
    mHoverButtonBrush,
    mActiveButtonBrush,
    bounds,
    mKneeboard,
    this,
    mDXR.mDWriteFactory,
    L"Consolas",
    L"1");

  // setup text format for debugging purposes only:
  mDXR.mDWriteFactory->CreateTextFormat(
    L"Consolas",
    nullptr,
    DWRITE_FONT_WEIGHT_NORMAL,
    DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STRETCH_NORMAL,
    20.0f,
    L"",
    mTextFormat.put());

  mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

  AddEventListener(
    mTestButton->evButtonClicked, &ButtonTestTab::Button1Toggle, this);
  AddEventListener(
    mTestButton2->evButtonClicked, &ButtonTestTab::Button2Toggle, this);
}

utf8_string ButtonTestTab::GetTitle() const {
  return _("BaseButton Test Tab");
}

void ButtonTestTab::Button1Toggle() {
  mButton1Toggle = !mButton1Toggle;
}

void ButtonTestTab::Button2Toggle() {
  mButton2Toggle = !mButton2Toggle;
}

void ButtonTestTab::Reload() {
}

uint16_t ButtonTestTab::GetPageCount() const {
  return 1;
}
D2D1_SIZE_U ButtonTestTab::GetNativeContentSize(uint16_t pageIndex) {
  return {768 * RENDER_SCALE, 1024 * RENDER_SCALE};
}

void ButtonTestTab::RenderPage(
  ID2D1DeviceContext* ctx,
  uint16_t pageIndex,
  const D2D1_RECT_F& rect) {
  // Start boilerplate for setting up the tab background:
  // taken from tab with plain text content
  const auto virtualSize = GetNativeContentSize(0);
  const D2D1_SIZE_F canvasSize {rect.right - rect.left, rect.bottom - rect.top};

  const auto scaleX = canvasSize.width / virtualSize.width;
  const auto scaleY = canvasSize.height / virtualSize.height;
  const auto scale = std::min(scaleX, scaleY);
  const D2D1_SIZE_F renderSize {
    scale * virtualSize.width, scale * virtualSize.height};

  ctx->SetTransform(
    D2D1::Matrix3x2F::Scale(scale, scale)
    * D2D1::Matrix3x2F::Translation(
      rect.left + ((canvasSize.width - renderSize.width) / 2),
      rect.top + ((canvasSize.height - renderSize.height) / 2)));

  winrt::com_ptr<ID2D1SolidColorBrush> background;
  ctx->CreateSolidColorBrush({1.0f, 1.0f, 1.0f, 1.0f}, background.put());

  ctx->FillRectangle(
    {0.0f,
     0.0f,
     static_cast<float>(virtualSize.width),
     static_cast<float>(virtualSize.height)},
    background.get());

  // end boilerplate with a white background and correct translation

  mTestButton->Render(ctx);
  mTestButton2->Render(ctx);
  mTestButton3->Render(ctx);

  wchar_t buffer[200];
  auto point = mKneeboard->GetCursorPoint();
  auto canvasPoint = mKneeboard->GetCursorCanvasPoint(point);

  int count = swprintf(
    buffer,
    200,
    L"Original Point: %.0f, %.0f. \n Canvas Point: %.0f, %.0f\n Button1 State: "
    L"%s\n Button2 State: %s",
    point.x,
    point.y,
    canvasPoint.x,
    canvasPoint.y,
    mButton1Toggle ? L"True" : L"False",
    mButton2Toggle ? L"True" : L"False");

  winrt::com_ptr<ID2D1SolidColorBrush> textBrush;
  ctx->CreateSolidColorBrush({0.0f, 0.0f, 1.0f, 1.0f}, textBrush.put());

  ctx->DrawTextW(
    buffer,
    count,
    mTextFormat.get(),
    {50.0f, 500.0f, 700.0f, 600.0f},
    textBrush.get());
}
};// namespace OpenKneeboard