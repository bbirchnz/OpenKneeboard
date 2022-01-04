#include "OpenKneeboard/DCSRadioLogTab.h"

#include <fmt/format.h>

#include "OpenKneeboard/Games/DCSWorld.h"

using DCS = OpenKneeboard::Games::DCSWorld;

namespace OpenKneeboard {

DCSRadioLogTab::DCSRadioLogTab() : DCSTab(_("Radio Log")) {
}

DCSRadioLogTab::~DCSRadioLogTab() {
}

void DCSRadioLogTab::Reload() {
}

uint16_t DCSRadioLogTab::GetPageCount() const {
  if (mCompletePages.empty()) {
    return 1;
  }

  if (mCurrentPageLines.empty()) {
    return mCompletePages.size();
  }

  return mCompletePages.size() + 1;
}

wxImage DCSRadioLogTab::RenderPage(uint16_t index) {
  wxBitmap bitmap(768, 1024);
  wxMemoryDC dc(bitmap);

  dc.SetFont(
    wxFont(16, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
  dc.SetBackground(*wxWHITE_BRUSH);
  dc.Clear();

  auto metrics = dc.GetTextExtent("m");
  auto padding = metrics.GetHeight();
  const auto columns = (bitmap.GetWidth() - (2 * padding)) / metrics.GetWidth();
  mColumns = columns;
  const auto rows
    = (bitmap.GetHeight() - (2 * padding)) / metrics.GetHeight() - 2;
  mRows = rows;
  LayoutMessages();

  if (mCurrentPageLines.empty()) {
    dc.SetTextForeground(wxColor(0x55, 0x55, 0x55));
    dc.DrawText(_("[waiting for radio messages]"), wxPoint(padding, padding));
    return bitmap.ConvertToImage();
  }

  const auto& lines = index == mCompletePages.size() ? mCurrentPageLines
                                                     : mCompletePages.at(index);

  wxPoint point {padding, padding};
  auto y = metrics.GetHeight();
  for (const auto& line: lines) {
    dc.DrawText(wxString(line.data(), line.size()), point);
    point.y += metrics.GetHeight();
  }

  // Draw footer
  dc.SetTextForeground(wxColor(0x77, 0x77, 0x77, 0xff));
  y = bitmap.GetHeight() - (padding + metrics.GetHeight());
  auto text
    = fmt::format(_("Page {} of {}").ToStdString(), index + 1, GetPageCount());
  metrics = dc.GetTextExtent(text);

  dc.DrawText(text, (bitmap.GetWidth() - metrics.GetWidth()) / 2, y);

  if (index > 0) {
    text = "<<<<<";
    dc.DrawText(text, padding, y);
  }

  if (index + 1 < GetPageCount()) {
    text = ">>>>>";
    metrics = dc.GetTextExtent(text);
    dc.DrawText(text, bitmap.GetWidth() - (metrics.GetWidth() + padding), y);
  }

  return bitmap.ConvertToImage();
}

void DCSRadioLogTab::PushMessage(const std::string& message) {
  mMessages.push_back(message);
}

void DCSRadioLogTab::LayoutMessages() {
  if (mRows <= 1 || mColumns <= 1) {
    return;
  }

  auto& pageLines = mCurrentPageLines;
  for (const auto& message: mMessages) {
    std::vector<std::string_view> messageLines;
    std::string_view remaining(message);
    while (!remaining.empty()) {
      if (remaining.size() <= mColumns) {
        messageLines.push_back(remaining);
        break;
      }
      auto space = remaining.find_last_of(" ", mColumns);
      if (space == remaining.npos) {
        messageLines.push_back(remaining.substr(0, mColumns));
        if (remaining.size() > mColumns) {
          remaining = remaining.substr(mColumns);
          continue;
        }
        break;
      }

      messageLines.push_back(remaining.substr(0, space));
      if (remaining.size() > space + 1) {
        remaining = remaining.substr(space + 1);
        continue;
      }
      break;
    }

    if (messageLines.size() > mRows) {
      if (!pageLines.empty()) {
        pageLines.push_back("");
      }
      for (const auto& line: messageLines) {
        if (pageLines.size() >= mRows) {
          mCompletePages.push_back(pageLines);
          pageLines.clear();
        }
        pageLines.push_back(std::string(line));
      }
      continue;
    }

    // If we reach here, we can fit the full message on one page. Now figure
    // out if we want a new page first.
    if (!pageLines.empty()) {
      if (mRows - pageLines.size() >= 2) {
        pageLines.push_back({});
      } else {
        mCompletePages.push_back(pageLines);
        pageLines.clear();
      }
    }

    for (auto line: messageLines) {
      if (pageLines.size() >= mRows) {
        mCompletePages.push_back(pageLines);
        pageLines.clear();
      }
      pageLines.push_back(std::string(line));
    }
  }
  mMessages.clear();
}

const char* DCSRadioLogTab::GetGameEventName() const {
  return DCS::EVT_RADIO_MESSAGE;
}

void DCSRadioLogTab::Update(
  const std::filesystem::path& installPath,
  const std::filesystem::path& savedGamesPath,
  const std::string& value) {
  PushMessage(value);
}

void DCSRadioLogTab::OnSimulationStart() {
  if (mColumns <= 0 || mMessages.empty()) {
    return;
  }
  PushMessage(std::string(mColumns, '-'));
  wxQueueEvent(this, new wxCommandEvent(okEVT_TAB_UPDATED));
}

}// namespace OpenKneeboard
