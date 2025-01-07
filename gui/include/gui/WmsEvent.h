#ifndef _WXWMSREQEVENT__
#define _WXWMSREQEVENT__

#include <wx/event.h>
#include <model/WmsReqParams.h>

class wxWMSRequestEvent : public wxEvent {
public:
  wxWMSRequestEvent(wxEventType eventType, int id, WmsReqParams P)
                    : wxEvent(id, eventType)  {
    p = P;
  }
  wxEvent* Clone() const { return new wxWMSRequestEvent(*this); }

  
  WmsReqParams p;
};

#endif
