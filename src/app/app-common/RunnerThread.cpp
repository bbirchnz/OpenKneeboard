/*
 * OpenKneeboard
 *
 * Copyright (C) 2022 Fred Emmott <fred@fredemmott.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#include <OpenKneeboard/ProcessShutdownBlock.h>
#include <OpenKneeboard/RunnerThread.h>
#include <OpenKneeboard/ThreadGuard.h>

#include <OpenKneeboard/dprint.h>
#include <OpenKneeboard/scope_guard.h>

#include <source_location>

namespace OpenKneeboard {
RunnerThread::RunnerThread() = default;

RunnerThread::RunnerThread(
  std::string_view name,
  std::function<winrt::Windows::Foundation::IAsyncAction(std::stop_token)> impl,
  const std::source_location& loc)
  : mName(std::string {name}) {
  mDQC = winrt::Microsoft::UI::Dispatching::DispatcherQueueController::
    CreateOnDedicatedThread();
  mCompletionEvent
    = winrt::handle {CreateEventW(nullptr, FALSE, FALSE, nullptr)};

  mDQC.DispatcherQueue().TryEnqueue(
    [name = winrt::to_hstring(name),
     impl,
     stop = mStopSource.get_token(),
     loc = loc,
     event = mCompletionEvent.get()]() -> winrt::fire_and_forget {
      const scope_guard setEventOnExit([event, name]() {
        TraceLoggingWrite(
          gTraceProvider,
          "RunnerThread/SetEvent",
          TraceLoggingString(winrt::to_string(name).c_str(), "Name"));
        SetEvent(event);
      });
      ProcessShutdownBlock block(loc);
      SetThreadDescription(GetCurrentThread(), name.c_str());
      ThreadGuard threadGuard;
      co_await impl(stop);
    });
}

RunnerThread::~RunnerThread() {
  OPENKNEEBOARD_TraceLoggingScope("RunnerThread::~RunnerThread()");
  this->Stop();
}

void RunnerThread::Stop() {
  mThreadGuard.CheckThread();
  if (!mDQC) {
    return;
  }

  TraceLoggingWrite(
    gTraceProvider,
    "RunnerThread::Stop()/request_stop()",
    TraceLoggingString(mName.c_str(), "Name"));
  mStopSource.request_stop();

  ([](auto event, auto dqc) -> winrt::fire_and_forget {
    dprint("Waiting for completion event");
    co_await winrt::resume_on_signal(event.get());
    dprint("Shutting down runner thread DQC");
    co_await dqc.ShutdownQueueAsync();
    dprint("Runner thread DQC shut down");
  })(std::move(mCompletionEvent), std::move(mDQC));
}

RunnerThread& RunnerThread::operator=(RunnerThread&& other) {
  this->Stop();
  mDQC = std::move(other.mDQC);
  mStopSource = std::move(other.mStopSource);
  mCompletionEvent = std::move(other.mCompletionEvent);

  other.mDQC = {nullptr};
  other.mStopSource = {};

  return *this;
}

}// namespace OpenKneeboard