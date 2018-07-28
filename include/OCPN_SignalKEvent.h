//
// Created by balp on 2018-07-28.
//

#ifndef OPENCPN_OCPN_SIGNALKEVENT_H
#define OPENCPN_OCPN_SIGNALKEVENT_H


#include <wx/event.h>
#include "wx/jsonval.h"

class OCPN_SignalKEvent : public wxEvent {
public:
    OCPN_SignalKEvent(int id,
                      wxEventType commandType,
                      wxJSONValue value)
            : wxEvent(id, commandType),
            m_value(value) {}
    virtual ~OCPN_SignalKEvent() {};

    const wxJSONValue& GetValue() { return m_value; };

    wxEvent *Clone() const {
        return new OCPN_SignalKEvent(GetId(), GetEventType(), m_value);
    };
private:
    wxJSONValue m_value;
};


#endif //OPENCPN_OCPN_SIGNALKEVENT_H
