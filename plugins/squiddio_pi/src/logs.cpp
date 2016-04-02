/******************************************************************************
 * Project:  OpenCPN
 * Purpose:  Squiddio plugin
 *
 ***************************************************************************
 *   Copyright (C) 2014 by Mauro Calvi                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "logs.h"

#define SECOND 1
#define MINUTE 60
#define HOUR 3600
#define DAY 86400

BEGIN_EVENT_TABLE(logsWindow, wxWindow)
EVT_TIMER(TIMER_ID,  logsWindow::OnTimerTimeout)
EVT_TIMER(TIMER_ID1, logsWindow::OnRefreshTimeout)
EVT_PAINT (logsWindow::OnPaint )
END_EVENT_TABLE();

logsWindow::logsWindow(squiddio_pi * plugin, wxWindow *pparent, wxWindowID id) :
        wxWindow(pparent, id, wxPoint(10, 200), wxSize(1000, 25),
                wxSIMPLE_BORDER, _T("OpenCPN PlugIn")) {

    this->SetTransparent(30);
    p_plugin = plugin;
    m_parent_window = pparent;
    m_pTimer = new wxTimer(this, TIMER_ID);
    m_pRefreshTimer = new wxTimer(this, TIMER_ID1);
    m_pRefreshTimer->Start(5000);

    m_LogsLayer = NULL;
    m_ErrorCondition = wxEmptyString;
    m_Notice = wxEmptyString;
    g_RetrieveSecs = period_secs(p_plugin->g_RetrievePeriod);
    m_LastLogSent = p_plugin->g_LastLogSent;
    m_LastLogsRcvd = p_plugin->g_LastLogsRcvd;

    m_LogsFilePath = p_plugin->layerdir;
    p_plugin->appendOSDirSlash(&m_LogsFilePath);
    m_LogsFilePath.Append(_T("logs.gpx"));

    DisplayLogsLayer();

    if (g_RetrieveSecs > 0)  // display friends' logs
            {
        if (wxDateTime::Now().GetTicks() > m_LastLogsRcvd.GetTicks() + g_RetrieveSecs) // overdue request at startup?
        {
            RequestRefresh(m_parent_window);
            if (p_plugin->CheckIsOnline())
                ShowFriendsLogs();
        }
        int nextEvent = g_RetrieveSecs - (wxDateTime::Now().GetTicks() - m_LastLogsRcvd.GetTicks());

        // if update is overdue, delay by a few seconds to prevent get request from interfering with opencpn launch, else schedule it for when it's due
        SetTimer(wxMax(wxMin(nextEvent, g_RetrieveSecs), 7));
    }
}

logsWindow::~logsWindow(){
    delete m_pTimer;
    delete m_pRefreshTimer;
}

wxString logsWindow::timeAgo(wxDateTime currTime) {
    int delta = wxDateTime::Now().GetTicks() - currTime.GetTicks();
    wxString timeString;

    if (delta == 0)
    {
        return _("Just now");
    } else if (delta == 1)
    {
        return _("One second ago");
    } else if (delta < MINUTE)
    {
        timeString.Printf(_("%i seconds ago"), delta);
        return timeString;
    } else if (delta < 2 * MINUTE)
    {
        return _("About a minute ago");
    } else if (delta < 45 * MINUTE)
    {
        timeString.Printf(_("%i minutes ago"), delta/MINUTE);
        return timeString;
    } else if (delta < 90 * MINUTE)
    {
        return _("About an hour ago");
    } else if (delta < DAY)
    {
        timeString.Printf(_("%i hours ago"), delta/HOUR);
        return timeString;
    } else if (delta < 48 * HOUR)
    {
        return _("Yesterday");
    } else if (delta < 365 * DAY)
    {
        timeString.Printf(_("%i days ago"), delta/DAY);
        return timeString;
    } else {
        return wxEmptyString;
    }
}

void logsWindow::SetTimer(int RetrieveSecs) {
    m_pTimer->Stop();
    if (RetrieveSecs > 0)
        m_pTimer->Start(RetrieveSecs * 1000);
    g_RetrieveSecs = RetrieveSecs;
    Refresh(false);
}

void logsWindow::OnTimerTimeout(wxTimerEvent& event) {
    if (p_plugin->CheckIsOnline()) {
        RequestRefresh(m_parent_window);
        ShowFriendsLogs();
        if (m_pTimer->GetInterval() / 1000 < g_RetrieveSecs) {
            // after initial friends update, reset the timer to the required interval
            SetTimer(0);
            SetTimer(g_RetrieveSecs * 1000);
        }
    }
    Refresh(false);
}

void logsWindow::OnRefreshTimeout(wxTimerEvent& event) {
    // if the last IsOnline() call returned negative (connection lost), check again every n seconds,
    // but only if there has been any mouse activity (to minimize data usage)
    if (!p_plugin->last_online &&
            (m_last_lat != p_plugin->m_cursor_lat || m_last_lon != p_plugin->m_cursor_lon)){
        p_plugin->CheckIsOnline();
        wxBell();
        m_last_lat =  p_plugin->m_cursor_lat;
        m_last_lon =  p_plugin->m_cursor_lon;
    }
    Refresh(false);
}

void logsWindow::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    wxColour cs;
    GetGlobalColor(_T("GREEN2"), &cs);
    wxColour cr;
    GetGlobalColor(_T("BLUE2"), &cr);
    wxColour ca;
    GetGlobalColor(_T("URED"), &ca);
    wxColour ci;
    GetGlobalColor(_T("DASHL"), &ci);
    wxColour cb;
    GetGlobalColor(_T("DASHB"), &cb);
    dc.SetBackground(cb);
    dc.SetTextBackground(cb);
    wxString lastRcvd, lastSent=wxEmptyString;

    wxFont *g_pFontSmall;
    g_pFontSmall = new wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(*g_pFontSmall);

    if (m_LastLogSent.IsValid() && m_LastLogSent.GetYear() > 1970 ) // can't figure out how to assess if it is NULL
        lastSent = m_LastLogSent.Format(_T(" %a-%d-%b-%Y %H:%M:%S  "), wxDateTime::Local);

    if (m_LastLogsRcvd.IsValid() && m_LastLogsRcvd.GetYear() > 1970 )
        lastRcvd = m_LastLogsRcvd.Format(_T(" %a-%d-%b-%Y %H:%M:%S  "),wxDateTime::Local);

    dc.Clear();
    wxString data;
    wxString lastSentAv = (lastSent.Length() > 0 ? lastSent : _T("(awaiting NMEA events)"));

    if (p_plugin->g_PostPeriod > 0 && p_plugin->last_online) {
        dc.SetTextForeground(cs);
    } else {
        dc.SetTextForeground(ci);
    }

    // own log postings
    dc.DrawText(_("Log sent:"), 5, 5);
    dc.DrawText(timeAgo(m_LastLogSent),100,5);
    dc.DrawText(_T("(")+lastSent+_T(")"),250,5);

    // friends logs
    if (g_RetrieveSecs > 0 && p_plugin->last_online) {
        dc.SetTextForeground(cr);
    } else {
        dc.SetTextForeground(ci);
    }

    dc.DrawText(_T("|"),450,5);

    wxString demo_msg = _T("");
    if (p_plugin->g_ApiKey == _T("squiddio_demo_api"))
        demo_msg = _T(" (demo)");
    dc.DrawText(_("Logs received")+demo_msg+_T(":"), 480, 5);
    dc.DrawText(timeAgo(m_LastLogsRcvd),610,5);
    dc.DrawText(_T("(")+lastRcvd+_T(")"),750,5);

    dc.SetTextForeground(ca);
    dc.DrawText(m_ErrorCondition ,950, 5);

    dc.SetTextForeground(cr);
    dc.DrawText(m_Notice ,950, 5);

    m_pRefreshTimer->Stop();
    m_pRefreshTimer->Start(5000);
}

void logsWindow::SetSentence(wxString &sentence) {
    wxString PostResponse;
    bool bGoodData = false;

    m_NMEA0183 << sentence;

    if (m_NMEA0183.PreParse()) {
        if (m_NMEA0183.LastSentenceIDReceived == _T("RMC")) {
            if (m_NMEA0183.Parse()) {
                if (m_NMEA0183.Rmc.IsDataValid == NTrue) {
                    float llt = m_NMEA0183.Rmc.Position.Latitude.Latitude;
                    int lat_deg_int = (int) (llt / 100);
                    float lat_deg = lat_deg_int;
                    float lat_min = llt - (lat_deg * 100);
                    mLat = lat_deg + (lat_min / 60.);
                    if (m_NMEA0183.Rmc.Position.Latitude.Northing == South)
                        mLat = -mLat;

                    float lln = m_NMEA0183.Rmc.Position.Longitude.Longitude;
                    int lon_deg_int = (int) (lln / 100);
                    float lon_deg = lon_deg_int;
                    float lon_min = lln - (lon_deg * 100);
                    mLon = lon_deg + (lon_min / 60.);
                    if (m_NMEA0183.Rmc.Position.Longitude.Easting == West)
                        mLon = -mLon;

                    mSog = m_NMEA0183.Rmc.SpeedOverGroundKnots;
                    mCog = m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue;

                    if (m_NMEA0183.Rmc.MagneticVariationDirection == East)
                        mVar = m_NMEA0183.Rmc.MagneticVariation;
                    else if (m_NMEA0183.Rmc.MagneticVariationDirection == West)
                        mVar = -m_NMEA0183.Rmc.MagneticVariation;
                    bGoodData = true;
                }
            }
        }
    }

    if (bGoodData) {
        PostResponse = PostPosition(mLat, mLon, mSog, mCog);
        wxJSONReader r;
        wxJSONValue v;
        r.Parse(PostResponse, &v);

        if (v[_T("error")].AsString() != _T("null") )
        {
            m_ErrorCondition = v[_T("error")].AsString();
            Refresh(false);
        }else{
            if (v[_T("notice")].AsString() != _T("null") )
                { m_Notice = v[_T("notice")].AsString();}
            else
                { m_Notice = wxEmptyString;}
            m_LastLogSent = wxDateTime::Now();
            p_plugin->g_LastLogSent = wxDateTime::GetTimeNow(); //to be saved in config file
            Refresh(false);
        }
    }

}

wxString logsWindow::PostPosition(double lat, double lon, double sog,
        double cog) {
    wxString reply = wxEmptyString;
    wxString parameters;

    parameters.Printf(
            _T("api_key=%s&email=%s&lat=%f&lon=%f&sog=%f&cog=%f&source=ocpn"),
            p_plugin->g_ApiKey.c_str(), p_plugin->g_Email.c_str(), lat, lon,
            sog, cog);

    _OCPN_DLStatus res = OCPN_postDataHttp(_T("http://squidd.io/positions.json"), parameters, reply, 5);

    if( res == OCPN_DL_NO_ERROR )
    {
        wxLogMessage(_("Created sQuiddio log update:") + reply);
    }

    return reply;
}

void logsWindow::ShowFriendsLogs() {
    wxString layerContents;
    wxString request_url;
    bool isLayerUpdate;

    request_url.Printf(_T("/connections.xml?api_key=%s&email=%s"),
            p_plugin->g_ApiKey.c_str(), p_plugin->g_Email.c_str());

    layerContents = p_plugin->DownloadLayer(request_url);

    if (layerContents.length() > 200) {
        isLayerUpdate = p_plugin->SaveLayer(layerContents, m_LogsFilePath);
        if (isLayerUpdate) {
            if (m_LogsLayer) {
                // hide and delete the current logs layer
                m_LogsLayer->SetVisibleOnChart(false);
                p_plugin->RenderLayerContentsOnChart(m_LogsLayer);
                RequestRefresh(m_parent_window);
                p_plugin->pLayerList->DeleteObject(m_LogsLayer);
            }

            DisplayLogsLayer();

            m_LastLogsRcvd = wxDateTime::Now();
            p_plugin->g_LastLogsRcvd = wxDateTime::GetTimeNow(); //to be saved in config file
            //wxBell();
        }
    } else {
        m_ErrorCondition = _T("Unable to retrieve friends logs: check your credentials and Follow List");
        Refresh(false);
        wxLogMessage(_T("sQuiddio: ")+m_ErrorCondition);
    }
}

void logsWindow::DisplayLogsLayer() {
    if (::wxFileExists(m_LogsFilePath) && g_RetrieveSecs > 0) {
        wxString null_region;
        m_LogsLayer = p_plugin->LoadLayer(m_LogsFilePath, null_region);
        m_LogsLayer->SetVisibleNames(false);
        p_plugin->RenderLayerContentsOnChart(m_LogsLayer);
    }
}



