#include <OpenKneeboard/Button.h>
#include <OpenKneeboard/CursorEvent.h>
#include <OpenKneeboard/TabState.h>

namespace OpenKneeboard {

BaseButton::BaseButton(
  winrt::com_ptr<ID2D1Brush> baseBrush,
  winrt::com_ptr<ID2D1Brush> hoverBrush,
  winrt::com_ptr<ID2D1Brush> activeBrush,
  D2D1_RECT_F buttonBounds,
  KneeboardState* kneeboard,
  Tab* tab) {
  mBaseBrush = baseBrush;
  mHoverBrush = hoverBrush;
  mActiveBrush = activeBrush;
  mButtonBounds = buttonBounds;
  mKneeboard = kneeboard;
  mTab = tab;

  mState = INACTIVE;

  // register for events:
  AddEventListener(kneeboard->evCursorEvent, &BaseButton::OnCursorEvent, this);
}

void BaseButton::SetEnabledState(bool isEnabled) {
  if (!isEnabled) {
    mState = DISABLED;
    return;
  }
  // if its now enabled, return it to inactive state
  if (mState == DISABLED) {
    mState = INACTIVE;
  }
}

// setup state based on what the cursor is up to, and whether this tab is
// visible
void BaseButton::OnCursorEvent(const CursorEvent& ev) {
  if (mState == DISABLED) {
    return;
  }

  const auto point = mKneeboard->GetCursorCanvasPoint({ev.mX, ev.mY});
  const bool pointInButton = point.x >= mButtonBounds.left
    && point.x <= mButtonBounds.right && point.y >= mButtonBounds.left
    && point.y <= mButtonBounds.right;

  bool touchingAnySurface
    = ev.mTouchState == CursorTouchState::TOUCHING_SURFACE;

  // if not in our button:
  if (!pointInButton) {
    mState = INACTIVE;
    ev return;
  }

  if (pointInButton && !touchingAnySurface) {
    // if it was previously in active state, then a release is a click:
    if (mState == ACTIVE) {
      evButtonClicked.Emit();
    }
    mState = HOVER;
    return;
  }

  if (pointInButton && touchingAnySurface) {
    mState = ACTIVE;
    return;
  }
}

void BaseButton::Render(ID2D1DeviceContext* ctx) {
  if (mState == DISABLED) {
    // don't draw, return
    return;
  }

  winrt::com_ptr<ID2D1Brush> brush;

  switch (mState) {
    case INACTIVE:
      brush = mBaseBrush;
      break;
    case HOVER:
      brush = mHoverBrush;
      break;
    case ACTIVE:
      brush = mActiveBrush;
      break;
    default:
      brush = mBaseBrush;
  }

  RenderButton(ctx, brush.get());
}
}// namespace OpenKneeboard