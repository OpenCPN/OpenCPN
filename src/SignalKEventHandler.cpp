//
// Created by balp on 2018-07-31.
//

#include <cstddef>

#include "wx/jsonval.h"
#include "wx/jsonwriter.h"

#include "geodesic.h"
#include "SignalKEventHandler.h"
#include "OCPN_SignalKEvent.h"
#include "chart1.h"

void SignalKEventHandler::OnEvtOCPN_SignalK(OCPN_SignalKEvent &event)
{
    auto root = event.GetValue();
#if 1
    wxString dbg;
    wxJSONWriter writer;
    writer.Write(root, dbg);

    wxString msg( _T("SignalK Event received: ") );
    msg.append(dbg);
    wxLogMessage(msg);
#endif

    if(root.HasMember("self")) {
        if(root["self"].AsString().StartsWith(_T("vessels.")))
            m_self = (root["self"].AsString());                                 // for java server
        else
            m_self = _T("vessels.") + (root["self"].AsString());                // for Node.js server
    }
    
    if(root.HasMember("context")
       && root["context"].IsString()) {
        auto context = root["context"].AsString();
        if (context != m_self) {
#if 0
            wxLogMessage(_T("** Ignore context of other ships.."));
#endif
            return;
        }
    }

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
        } else if(update_path == _T("navigation.courseOverGroundMagnetic"))
        {  // Ignore magnetic COG as OpenCPN don't handle yet.
        } else if(update_path == _T("navigation.gnss.satellites"))
        {
            updateGnssSatellites(value, sfixtime);
        } else if(update_path == _T("navigation.headingTrue"))
        {
            updateHeadingTrue(value, sfixtime);
        } else if(update_path == _T("navigation.headingMagnetic"))
        {
            updateHeadingMagnetic(value, sfixtime);
        } else if(update_path == _T("navigation.magneticVariation"))
        {
            updateMagneticVariance(value, sfixtime);
        } else {
            wxLogMessage(wxString::Format(_T("** Signal K unhandled update: %s"), update_path));
#if 0
            wxString dbg;
            wxJSONWriter writer;
            writer.Write(item, dbg);
            wxString msg( _T("update: ") );
            msg.append(dbg);
            wxLogMessage(msg);
#endif
        }

    }
}

void SignalKEventHandler::updateNavigationPosition(wxJSONValue &value, const wxString &sfixtime) const {
    if(value.HasMember("latitude")
       && value.HasMember("longitude")) {
        wxLogMessage(_T(" ***** Position Update"));
        m_frame->setPosition(value["latitude"].AsDouble(),
                             value["longitude"].AsDouble());
        m_frame->PostProcessNMEA(true, false, sfixtime);
    }
}

void SignalKEventHandler::updateNavigationSpeedOverGround(wxJSONValue &value,
                                                          const wxString &sfixtime) const {
    double sog_ms = value.AsDouble();
    double sog_knot = sog_ms * ms_to_knot_factor;
    wxLogMessage(wxString::Format(_T(" ***** SOG: %f, %f"), sog_ms, sog_knot));
    m_frame->setSpeedOverGround(sog_knot);
    m_frame->PostProcessNMEA(false, true, sfixtime);
}

void SignalKEventHandler::updateNavigationCourseOverGround(wxJSONValue &value,
                                                           const wxString &sfixtime) const {
    double cog_rad = value.AsDouble();
    double cog_deg = GEODESIC_RAD2DEG(cog_rad);
    wxLogMessage(wxString::Format(_T(" ***** COG: %f, %f"), cog_rad, cog_deg));
    m_frame->setCourseOverGround(cog_deg);
    m_frame->PostProcessNMEA(false, true, sfixtime);
}

void SignalKEventHandler::updateGnssSatellites(wxJSONValue &value,
                                               const wxString &sfixtime) const
{
    m_frame->setSatelitesInView(value.AsInt());
}

void SignalKEventHandler::updateHeadingTrue(wxJSONValue &value,
                                            const wxString &sfixtime) const
{
    m_frame->setHeadingTrue(GEODESIC_RAD2DEG(value.AsDouble()));
}

void SignalKEventHandler::updateHeadingMagnetic(wxJSONValue &value,
                                            const wxString &sfixtime) const
{
    m_frame->setHeadingMagnetic(GEODESIC_RAD2DEG(value.AsDouble()));
}

void SignalKEventHandler::updateMagneticVariance(wxJSONValue &value,
                                                 const wxString &sfixtime) const
{
    m_frame->setMagneticVariation(GEODESIC_RAD2DEG(value.AsDouble()));
}