#include <OpenKneeboard/Button.h>
#include <OpenKneeboard/CursorEvent.h>

namespace OpenKneeboard {

Button::Button(
  winrt::com_ptr<ID2D1Brush> baseBrush,
  winrt::com_ptr<ID2D1Brush> hoverBrush,
  winrt::com_ptr<ID2D1Brush> activeBrush,
  D2D1_RECT_F buttonBounds,
  KneeboardState* kneeboard,
  std::shared_ptr<OpenKneeboard::TabState> tab) {
  mBaseBrush = baseBrush;
  mHoverBrush = hoverBrush;
  mActiveBrush = activeBrush;
  mButtonBounds = buttonBounds;
  mKneeboard = kneeboard;
  mTab = tab;

  mState = INACTIVE;

  // register for events:
  AddEventListener(kneeboard->evCursorEvent, &Button::OnCursorEvent, this);
}

// setup state based on what the cursor is up to, and whether this tab is
// visible
void Button::OnCursorEvent(const CursorEvent& ev) {
  const auto point = mKneeboard->GetCursorCanvasPoint({ev.mX, ev.mY});
  const bool pointInButton = point.x >= mButtonBounds.left
    && point.x <= mButtonBounds.right && point.y >= mButtonBounds.left
    && point.y <= mButtonBounds.right;

  bool touchingAnySurface
    = ev.mTouchState == CursorTouchState::TOUCHING_SURFACE;

  // if not in our button:
  if (!pointInButton) {
    mState = INACTIVE;
    return;
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

void Button::Render(winrt::com_ptr<ID2D1DeviceContext> ctx) {
  winrt::com_ptr<ID2D1Brush> brush;

  // will be drawn either If our tab visible, or tab==null == we're in the
  // header
  if (mTab != nullptr && mKneeboard->GetCurrentTab() != mTab) {
    return;
  }

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

  RenderButton(ctx, brush);
}
}// namespace OpenKneeboard