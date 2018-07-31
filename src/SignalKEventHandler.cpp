//
// Created by balp on 2018-07-31.
//

#include <cstddef>

#include "geodesic.h"
#include "SignalKEventHandler.h"
#include "OCPN_SignalKEvent.h"
#include "chart1.h"

void SignalKEventHandler::OnEvtOCPN_SignalK(OCPN_SignalKEvent &event)
{
    auto root = event.GetValue();
    if(root.HasMember("updates")
       && root["updates"].IsArray()) {
        wxJSONValue &updates = root["updates"];
        for (int i = 0; i < updates.Size(); ++i) {
            handleUpdate(updates[i]);
        }
    }
}

void SignalKEventHandler::handleUpdate(wxJSONValue &update) const {
    wxString sfixtime = "";
    if(update.HasMember("timestamp")) {
        sfixtime = update["timestamp"].AsString();
    }
    if(update.HasMember("values")
       && update["values"].IsArray())
    {
        for (int j = 0; j < update["values"].Size(); ++j) {
            wxJSONValue &item = update["values"][j];
            updateItem(item, sfixtime);
        }
    }
}

void SignalKEventHandler::updateItem(wxJSONValue &item, wxString &sfixtime) const {
    if(item.HasMember("path")
       && item.HasMember("value")) {
        const wxString &update_path = item["path"].AsString();
        wxJSONValue &value = item["value"];
        if(update_path == _T("navigation.position")) {
            updateNavigationPosition(value, sfixtime);
        } else if(update_path == _T("navigation.speedOverGround"))
        {
            updateNavigationSpeedOverGround(value, sfixtime);
        } else if(update_path == _T("navigation.courseOverGroundTrue"))
        {
            updateNavigationCourseOverGround(value, sfixtime);
        }
    }
}

void SignalKEventHandler::updateNavigationPosition(wxJSONValue &value, const wxString &sfixtime) const {
    if(value.HasMember("latitude")
       && value.HasMember("longitude")) {
        wxLogMessage(_T(" ***** Position Update"));
        m_frame->setPosition(value["latitude"].AsDouble(),
                             value["longitude"].AsDouble());
        m_frame->PostProcessNNEA(true, false, sfixtime);
    }
}

void SignalKEventHandler::updateNavigationSpeedOverGround(wxJSONValue &value,
        const wxString &sfixtime) const {
    wxLogMessage(wxString::Format(_T(" ***** SOG: %f"), value.AsDouble()));
    m_frame->setSpeedOverGround(value.AsDouble());
    m_frame->PostProcessNNEA(false, true, sfixtime);
}

void SignalKEventHandler::updateNavigationCourseOverGround(wxJSONValue &value,
        const wxString &sfixtime) const {
    double cog_rad = value.AsDouble();
    double cog_deg = GEODESIC_RAD2DEG(cog_rad);
    wxLogMessage(wxString::Format(_T(" ***** COG: %f, %f"), cog_rad, cog_deg));
    m_frame->setCourseOverGround(cog_deg);
    m_frame->PostProcessNNEA(false, true, sfixtime);
}