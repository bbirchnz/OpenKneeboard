#include <OpenKneeboard/ButtonTestTab.h>
#include <OpenKneeboard/IconButton.h>

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
    L'\xF57E');
  mTestButton = button;
}

utf8_string ButtonTestTab::GetTitle() const {
  return _("BaseButton Test Tab");
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
  mTestButton->Render(ctx);
}
};// namespace OpenKneeboard