#pragma once

#include <OpenKneeboard/Events.h>
#include <OpenKneeboard/KneeboardState.h>
#include <OpenKneeboard/Tab.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <shims/winrt.h>

#include <memory>

namespace OpenKneeboard {

enum ButtonState { DISABLED, INACTIVE, HOVER, ACTIVE };

// Abstract Base class for a button
class BaseButton : EventReceiver {
 public:
  BaseButton(
    winrt::com_ptr<ID2D1Brush> baseBrush,
    winrt::com_ptr<ID2D1Brush> hoverBrush,
    winrt::com_ptr<ID2D1Brush> activeBrush,
    D2D1_RECT_F buttonBounds,
    KneeboardState* kneeboard,
    Tab* tab);

  void Render(ID2D1DeviceContext* ctx);
  void SetEnabledState(bool isEnabled);

  Event<> evButtonClicked;

 protected:
  // implement this to actually draw your button with provided brush
  virtual void RenderButton(ID2D1DeviceContext* ctx, ID2D1Brush* brush) = 0;

  void OnCursorEvent(const CursorEvent& ev);

  winrt::com_ptr<ID2D1Brush> mBaseBrush;
  winrt::com_ptr<ID2D1Brush> mHoverBrush;
  winrt::com_ptr<ID2D1Brush> mActiveBrush;
  D2D1_RECT_F mButtonBounds;
  bool mIsActive;
  KneeboardState* mKneeboard;
  Tab* mTab;
  ButtonState mState;
};

}// namespace OpenKneeboard