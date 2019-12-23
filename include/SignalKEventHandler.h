//
// Created by balp on 2018-07-31.
//

#ifndef OPENCPN_SIGNALKEVENTHANDLER_H
#define OPENCPN_SIGNALKEVENTHANDLER_H


static const double ms_to_knot_factor = 1.9438444924406;

#include <wx/jsonval.h>

class MyFrame;
class OCPN_SignalKEvent;


class SignalKEventHandler {
public:
    explicit SignalKEventHandler(MyFrame* frame) : m_frame(frame), m_self("") {}
    void OnEvtOCPN_SignalK(OCPN_SignalKEvent &event);
private:
    MyFrame* m_frame;
    wxString m_self;

    void handleUpdate(wxJSONValue &update) const;

    void updateItem(wxJSONValue &item, wxString &sfixtime) const;

    void updateNavigationPosition(wxJSONValue &value, const wxString &sfixtime) const;
    void updateNavigationSpeedOverGround(wxJSONValue &value, const wxString &sfixtime) const;
    void updateNavigationCourseOverGround(wxJSONValue &value, const wxString &sfixtime) const;
    void updateGnssSatellites(wxJSONValue &value, const wxString &sfixtime) const;
    void updateHeadingTrue(wxJSONValue &value, const wxString &sfixtime) const;
    void updateHeadingMagnetic(wxJSONValue &value, const wxString &sfixtime) const;
    void updateMagneticVariance(wxJSONValue &value, const wxString &sfixtime) const;
};


#endif //OPENCPN_SIGNALKEVENTHANDLER_H
