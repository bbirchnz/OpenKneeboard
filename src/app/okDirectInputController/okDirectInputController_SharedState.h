#pragma once

#include "okDirectInputController.h"

#include <vector>
#include <winrt/base.h>

#include "okDirectInputController_DIBinding.h"

struct okDirectInputController::SharedState {
  winrt::com_ptr<IDirectInput8> di8;
  std::vector<DIBinding> bindings;
  wxEvtHandler* hook = nullptr;
};
