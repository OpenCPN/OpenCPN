#ifndef _WMSFRAME__
#define _WMSFRAME__

#include <wx/frame.h>
#include <wx/string.h>
#include <wx/log.h>
#include "WmsEvent.h"

#include "viewport.h"
#include "chcanv.h"

#include "model/logger.h"

wxDECLARE_EVENT(WXWMSREQUESTEVENT, wxWMSRequestEvent);

class WmsFrame : public wxFrame {
private:
   ChartCanvas* m_pChartCanvas;

   wxStaticText* pText;

   unsigned int lastSize_W;
   unsigned int lastSize_H;

   void* jpegdatabuffer;

protected:

  // Virtual event handlers, overide them in your derived class
  //virtual void AboutFrameOnActivate(wxActivateEvent& event) { event.Skip(); }

  void OnWmsRequestEvent(wxWMSRequestEvent& event);
  
  

public:
  WmsFrame(wxWindow* parent, wxWindowID id = wxID_ANY,
             const wxString& title = _("WMS Rendering Frame"),
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxSize(500, 300),
             long style = wxCAPTION | wxCLOSE_BOX | wxFRAME_FLOAT_ON_PARENT |
                          wxFRAME_NO_TASKBAR | wxRESIZE_BORDER |
                          wxTAB_TRAVERSAL);

  ~WmsFrame();
};

#endif
