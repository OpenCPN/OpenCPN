//
// Created by balp on 2018-07-28.
//

#ifndef OPENCPN_OCPN_SIGNALKEVENT_H
#define OPENCPN_OCPN_SIGNALKEVENT_H

#include <wx/event.h>
#include "wx/jsonval.h"
#include "wx/jsonreader.h"
#include "wx/jsonwriter.h"

class OCPN_SignalKEvent : public wxEvent {
public:
  OCPN_SignalKEvent(int id, wxEventType commandType, std::string rawString)
      : wxEvent(id, commandType), m_string(rawString) {}
  virtual ~OCPN_SignalKEvent(){};

  const std::string& GetString() { return m_string; }

  wxEvent* Clone() const {
    return new OCPN_SignalKEvent(GetId(), GetEventType(), m_string);
  };

private:
  std::string m_string;
};

DECLARE_EVENT_TYPE(EVT_OCPN_SIGNALKSTREAM_TMP, wxID_ANY)
wxDECLARE_EVENT(EVT_OCPN_SIGNALKSTREAM, OCPN_SignalKEvent);

#endif  // OPENCPN_OCPN_SIGNALKEVENT_H
