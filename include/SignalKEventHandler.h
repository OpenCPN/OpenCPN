//
// Created by balp on 2018-07-31.
//

#ifndef OPENCPN_SIGNALKEVENTHANDLER_H
#define OPENCPN_SIGNALKEVENTHANDLER_H


#include "../plugins/dashboard_pi/src/wxJSON/jsonval.h"

class MyFrame;
class OCPN_SignalKEvent;


class SignalKEventHandler {
public:
    explicit SignalKEventHandler(MyFrame* app) : m_frame(app) {}
    void OnEvtOCPN_SignalK(OCPN_SignalKEvent &event);
private:
    MyFrame* m_frame;


    void handleUpdate(wxJSONValue &update) const;

    void updateItem(wxJSONValue &item, wxString &sfixtime) const;

    void updateNavigationPosition(wxJSONValue &value, const wxString &sfixtime) const;
    void updateNavigationSpeedOverGround(wxJSONValue &value, const wxString &sfixtime) const;
    void updateNavigationCourseOverGround(wxJSONValue &value, const wxString &sfixtime) const;

};


#endif //OPENCPN_SIGNALKEVENTHANDLER_H
