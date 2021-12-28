#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/image.h>

#include <json.hpp>

class okPageController : public wxEvtHandler {
  public:
    okPageController();
    virtual ~okPageController();

    virtual wxString GetTitle() const = 0;

    virtual wxWindow* GetSettingsUI(wxWindow* parent);
    virtual nlohmann::json GetSettings() const;
};
