/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA Data Multiplexer Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 ***************************************************************************/
#ifndef __MULTIPLEXER_H__
#define __MULTIPLEXER_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "pluginmanager.h"  // for PlugInManager
#include "datastream.h"

WX_DEFINE_ARRAY(DataStream *, wxArrayOfDataStreams);

class Multiplexer : public wxEvtHandler
{
    public:
        Multiplexer();
        ~Multiplexer();
        void AddStream(DataStream *stream);
        void StopAllStreams();
        void ClearStreams();
        DataStream *FindStream( wxString port );
        void StopAndRemoveStream( DataStream *stream );
        void SaveStreamProperties( DataStream *stream );
        bool CreateAndRestoreSavedStreamProperties();
        
        void SendNMEAMessage( wxString &msg );
        void SetAISHandler(wxEvtHandler *handler);
        void SetGPSHandler(wxEvtHandler *handler);

        bool SendRouteToGPS(Route *pr, wxString &com_name, bool bsend_waypoints, wxGauge *pProgress);
        bool SendWaypointToGPS(RoutePoint *prp, wxString &com_name, wxGauge *pProgress);

        void OnEvtStream(OCPN_DataStreamEvent& event);
        void LogOutputMessage( wxString &msg, DataStream *stream, bool b_filter );
        
    private:
        wxArrayOfDataStreams *m_pdatastreams;

        wxEvtHandler        *m_aisconsumer;
        wxEvtHandler        *m_gpsconsumer;
        
        //      A set of temporarily saved parameters for a DataStream
        wxString port_save;
        wxString baud_rate_save;
        dsPortType port_type_save;
        int priority_save;
        wxArrayString input_sentence_list_save;
        ListType input_sentence_list_type_save;
        wxArrayString output_sentence_list_save;
        ListType output_sentence_list_type_save;
        bool bchecksum_check_save;
        bool bGarmin_GRM_upload_save;
        bool bGarmin_GRMN_mode_save;
        
};
#endif // __MULTIPLEXER_H__

