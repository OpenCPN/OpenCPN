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

    wxString GetUpdatePath() {
        if(m_value.HasMember("updates")) {
            wxJSONValue &updates = m_value["updates"];
            if(updates.IsArray()) {
                for (int i = 0; i < updates.Size(); ++i) {
                    if(updates[i].HasMember("values")) {
                        auto values = updates[i]["values"];
                        if(values.IsArray()) {
                            for (int j = 0; j < values.Size(); ++j) {
                                if(values[i].HasMember("path")) {
                                    return values[i]["path"].AsString();
                                }
                            }
                        }
                    }
                }
            }
        }
        return _T("");
    }
    
    wxString GetTimeStampForPath(wxString path) {
        if(m_value.HasMember("updates")) {
            wxJSONValue &updates = m_value["updates"];
            if(updates.IsArray()) {
                for (int i = 0; i < updates.Size(); ++i) {
                    if(updates[i].HasMember("values")) {
                        auto values = updates[i]["values"];
                        if(values.IsArray()) {
                            for (int j = 0; j < values.Size(); ++j) {
                                if(values[i].HasMember("path")) {
                                    auto update_Path = values[i]["path"].AsString();
                                    if(update_Path == path) {
                                        return values[i]["timestamp"].AsString();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return _T("");
    }

    wxJSONValue GetValueForPath(wxString path) {
        if(m_value.HasMember("updates")) {
            wxJSONValue &updates = m_value["updates"];
            if(updates.IsArray()) {
                for (int i = 0; i < updates.Size(); ++i) {
                    if(updates[i].HasMember("values")) {
                        auto values = updates[i]["values"];
                        if(values.IsArray()) {
                            for (int j = 0; j < values.Size(); ++j) {
                                if(values[i].HasMember("path")) {
                                    auto update_Path = values[i]["path"].AsString();
                                    if(update_Path == path) {
                                        return values[i]["value"];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        const wxJSONValue retVal = wxJSONValue(wxJSONTYPE_NULL);
        return retVal;
    }



private:
    wxJSONValue m_value;
};


DECLARE_EVENT_TYPE(EVT_OCPN_SIGNALKSTREAM_TMP, wxID_ANY)
wxDECLARE_EVENT(EVT_OCPN_SIGNALKSTREAM, OCPN_SignalKEvent);

#endif //OPENCPN_OCPN_SIGNALKEVENT_H
