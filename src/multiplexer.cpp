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

#include "wx/wx.h"

#include "multiplexer.h"
#include "navutil.h"                    // for NMEALogWindow

extern PlugInManager    *g_pi_manager;
extern wxString         g_GPS_Ident;
extern TTYWindow        *g_NMEALogWindow;

Multiplexer::Multiplexer()
{
    m_aisconsumer = NULL;
    m_gpsconsumer = NULL;
    Connect(wxEVT_OCPN_DATASTREAM, (wxObjectEventFunction)(wxEventFunction)&Multiplexer::OnEvtStream);
    m_pdatastreams = new wxArrayOfDataStreams();
}

Multiplexer::~Multiplexer()
{
    ClearStreams();
    delete m_pdatastreams;
}

void Multiplexer::AddStream(DataStream *stream)
{
    m_pdatastreams->Add(stream);
}

void Multiplexer::StopAllStreams()
{
    for (size_t i = 0; i < m_pdatastreams->Count(); i++)
    {
        m_pdatastreams->Item(i)->Close();
    }
}

void Multiplexer::ClearStreams()
{
    for (size_t i = 0; i < m_pdatastreams->Count(); i++)
    {
        m_pdatastreams->Item(i)->Close();
    }
    m_pdatastreams->Clear();
}

void Multiplexer::SendNMEAMessage( wxString &msg )
{
    //Send to all the outputs
    for (size_t i = 0; i < m_pdatastreams->Count(); i++)
    {
        DataStream* s = m_pdatastreams->Item(i);
        if ( s->IsOk() && (s->GetIoSelect() == DS_TYPE_INPUT_OUTPUT || s->GetIoSelect() == DS_TYPE_OUTPUT) ) {
            bool bout_filter = true;
            
            if(s->SentencePassesFilter( msg, FILTER_OUTPUT ) ) {
                s->SendSentence(msg);
                bout_filter = false;
            }    
            //Send to the Debug Window, if open
            if( g_NMEALogWindow) {
                wxDateTime now = wxDateTime::Now();
                wxString ss = now.FormatISOTime();
                ss.Prepend(_T("--> "));
                ss.Append( _T(" (") );
                ss.Append( s->GetPort() );
                ss.Append( _T(") ") );
                ss.Append( msg );
                if(bout_filter)
                    ss.Prepend( _T("<AMBER>") );
                else
                    ss.Prepend( _T("<BLUE>") );
                
                g_NMEALogWindow->Add( ss );
                g_NMEALogWindow->Refresh( false );
            }
            
        }
        
    }
    //Send to plugins
    if ( g_pi_manager )
        g_pi_manager->SendNMEASentenceToAllPlugIns( msg );
}

void Multiplexer::SetAISHandler(wxEvtHandler *handler)
{
    m_aisconsumer = handler;
}

void Multiplexer::SetGPSHandler(wxEvtHandler *handler)
{
    m_gpsconsumer = handler;
}

void Multiplexer::OnEvtStream(OCPN_DataStreamEvent& event)
{
    wxString message = event.GetNMEAString();
    wxString ds = event.GetDataSource();
    DataStream *stream = event.GetDataStream();
 
    
    if( !message.IsEmpty() )
    {
        //Send to all the other outputs
        for (size_t i = 0; i < m_pdatastreams->Count(); i++)
        {
            DataStream* s = m_pdatastreams->Item(i);
            if ( ds != s->GetPort() ) {
                if ( s->IsOk() )
                    if ( s->GetIoSelect() == DS_TYPE_INPUT_OUTPUT || s->GetIoSelect() == DS_TYPE_OUTPUT ) {
                        bool bout_filter = true;
                        
                        if(s->SentencePassesFilter( message, FILTER_OUTPUT ) ) {
                            s->SendSentence(message);
                            bout_filter = false;
                        }    
                            //Send to the Debug Window, if open
                        if( g_NMEALogWindow) {
                            wxDateTime now = wxDateTime::Now();
                            wxString ss = now.FormatISOTime();
                            ss.Prepend(_T("--> "));
                            ss.Append( _T(" (") );
                            ss.Append( s->GetPort() );
                            ss.Append( _T(") ") );
                            ss.Append( message );
                            if(bout_filter)
                                ss.Prepend( _T("<AMBER>") );
                            else
                                ss.Prepend( _T("<BLUE>") );
                                
                            g_NMEALogWindow->Add( ss );
                            g_NMEALogWindow->Refresh( false );
                        }
                            
                        
                    }
            }
        }
        //Send to core consumers
        //if it passes the source's input filter
        //  If there is no datastream, as for PlugIns, then pass everything
        bool bpass = true;
        if( stream )
            bpass = stream->SentencePassesFilter( message, FILTER_INPUT );
            
        if( bpass ) {
            if( message.Mid(3,3).IsSameAs(_T("VDM")) ||
                message.Mid(3,3).IsSameAs(_T("VDO")) ||
                message.Mid(1,5).IsSameAs(_T("FRPOS")) ||
                message.Mid(1,2).IsSameAs(_T("CD")) )
            {
                if( m_aisconsumer )
                    m_aisconsumer->AddPendingEvent(event);
            }
            else
            {
                if( m_gpsconsumer )
                    m_gpsconsumer->AddPendingEvent(event);
            }
        //Send to plugins
            if ( g_pi_manager )
                g_pi_manager->SendNMEASentenceToAllPlugIns( message );
        }
        
        //Send to the Debug Window, if open
        if( g_NMEALogWindow) {
            wxDateTime now = wxDateTime::Now();
            wxString ss = now.FormatISOTime();
            ss.Append( _T(" (") );
            ss.Append( event.GetDataSource() );
            ss.Append( _T(") ") );
            ss.Append( message );
            if( !bpass )
                ss.Prepend( _T("<AMBER>") );
            else
                ss.Prepend( _T("<GREEN>") );
            
            g_NMEALogWindow->Add( ss );
            g_NMEALogWindow->Refresh( false );
        }
        
    }
}

bool Multiplexer::SendRouteToGPS(Route *pr, wxString &com_name, bool bsend_waypoints, wxGauge *pProgress)
{
    bool ret_bool = false;
/*TODO: RE-enable use of Garmin host
#ifdef USE_GARMINHOST
#ifdef __WXMSW__
    if(com_name.Upper().Matches(_T("*GARMIN*"))) // Garmin USB Mode
    {
        if(m_pdevmon)
            m_pdevmon->StopIOThread(true);

        int v_init = Garmin_GPS_Init(NULL, wxString(_T("usb:")));

        if(v_init < 0)
        {
            wxString msg(_T(" Garmin USB GPS could not be initialized"));
            wxLogMessage(msg);
            msg.Printf(_T(" Error Code is %d"), v_init);
            wxLogMessage(msg);
            msg = _T(" LastGarminError is: ");
            msg += GetLastGarminError();
            wxLogMessage(msg);

            ret_bool = false;
        }
        else
        {
            wxLogMessage(_T("Garmin USB Initialized"));

            wxString msg = _T("USB Unit identifies as: ");
            wxString GPS_Unit = Garmin_GPS_GetSaveString();
            msg += GPS_Unit;
            wxLogMessage(msg);

            wxLogMessage(_T("Sending Routes..."));
            int ret1 = Garmin_GPS_SendRoute(NULL, wxString(_T("usb:")), pr, pProgress);

            if(ret1 != 1)
            {
                wxLogMessage(_T(" Error Sending Routes"));
                wxString msg;
                msg = _T(" LastGarminError is: ");
                msg += GetLastGarminError();
                wxLogMessage(msg);

                ret_bool = false;
            }
            else
                ret_bool = true;
        }

        if(m_pdevmon)
            m_pdevmon->RestartIOThread();

        return ret_bool;
    }
#endif

    if(m_bGarmin_host)
    {
        int ret_val;

        // Request that the thread should pause
        m_brequest_thread_pause = true;

        ::wxSleep(1);
        bool b_gotPort = false;
        // Poll, waiting for the Mutex. Abort after 5 seconds
        long time = ::wxGetLocalTime();
        while(::wxGetLocalTime() < time + 5)
        {
            if(wxMUTEX_BUSY != m_pPortMutex->TryLock())
            {
                b_gotPort = true;
                break;
            }
        }

        if(!b_gotPort)
        {
            wxString msg(_T("Error Sending Route...could not lock Mutex on port: "));
            msg +=com_name;
            wxLogMessage(msg);

            return false;
        }

        if ( pProgress )
        {
            pProgress->SetValue ( 20 );
            pProgress->Refresh();
            pProgress->Update();
        }

        // Initialize the Garmin receiver, build required Jeeps internal data structures
        int v_init = Garmin_GPS_Init(g_pCommMan, com_name);
        if(v_init < 0)
        {
            wxString msg(_T("Garmin GPS could not be initialized on port: "));
            msg +=com_name;
            wxString err;
            err.Printf(_T(" Error Code is %d"), v_init);
            msg += err;

            msg += _T("\n LastGarminError is: ");
            msg += GetLastGarminError();

            wxLogMessage(msg);

            ret_bool = false;
            goto ret_point;
        }
        else
        {
            wxString msg(_T("Sent Route to Garmin GPS on port: "));
            msg +=com_name;
            msg += _T("\n Unit identifies as: ");
            wxString GPS_Unit = Garmin_GPS_GetSaveString();
            msg += GPS_Unit;

            wxLogMessage(msg);
        }

        if ( pProgress )
        {
            pProgress->SetValue ( 40 );
            pProgress->Refresh();
            pProgress->Update();
        }

        ret_val = Garmin_GPS_SendRoute(g_pCommMan, com_name, pr, pProgress);
        if(ret_val != 1)
        {
            wxString msg(_T("Error Sending Route to Garmin GPS on port: "));
            msg +=com_name;
            wxString err;
            err.Printf(_T(" Error Code is %d"), ret_val);

            msg += _T("\n LastGarminError is: ");
            msg += GetLastGarminError();

            msg += err;
            wxLogMessage(msg);

            ret_bool = false;
            goto ret_point;
        }
        else
            ret_bool = true;

ret_point:
        // Release the Mutex
        m_brequest_thread_pause = false;
        m_pPortMutex->Unlock();

        if ( pProgress )
        {
            pProgress->SetValue ( 100 );
            pProgress->Refresh();
            pProgress->Update();
        }

        wxMilliSleep ( 500 );

        return ret_bool;
    }
    else
#endif //USE_GARMINHOST
*/
    {
        { // Standard NMEA mode
            SENTENCE snt;
            NMEA0183 oNMEA0183;
            oNMEA0183.TalkerID = _T ( "EC" );

            int nProg = pr->pRoutePointList->GetCount() + 1;
            if ( pProgress )
            pProgress->SetRange ( 100 );

            int progress_stall = 500;
            if(pr->pRoutePointList->GetCount() > 10)
                progress_stall = 500;

            // Send out the waypoints, in order
            if ( bsend_waypoints )
            {
                wxRoutePointListNode *node = pr->pRoutePointList->GetFirst();

                int ip = 1;
                while ( node )
                {
                    RoutePoint *prp = node->GetData();

                    if(g_GPS_Ident == _T("Generic"))
                    {
                        if ( prp->m_lat < 0. )
                            oNMEA0183.Wpl.Position.Latitude.Set ( -prp->m_lat, _T ( "S" ) );
                        else
                            oNMEA0183.Wpl.Position.Latitude.Set ( prp->m_lat, _T ( "N" ) );

                        if ( prp->m_lon < 0. )
                            oNMEA0183.Wpl.Position.Longitude.Set ( -prp->m_lon, _T ( "W" ) );
                        else
                            oNMEA0183.Wpl.Position.Longitude.Set ( prp->m_lon, _T ( "E" ) );

                        oNMEA0183.Wpl.To = prp->GetName().Truncate ( 6 );

                        oNMEA0183.Wpl.Write ( snt );

                    }
                    else if(g_GPS_Ident == _T("FurunoGP3X"))
                    {
                        oNMEA0183.TalkerID = _T ( "PFEC," );

                        if ( prp->m_lat < 0. )
                            oNMEA0183.GPwpl.Position.Latitude.Set ( -prp->m_lat, _T ( "S" ) );
                        else
                            oNMEA0183.GPwpl.Position.Latitude.Set ( prp->m_lat, _T ( "N" ) );

                        if ( prp->m_lon < 0. )
                            oNMEA0183.GPwpl.Position.Longitude.Set ( -prp->m_lon, _T ( "W" ) );
                        else
                            oNMEA0183.GPwpl.Position.Longitude.Set ( prp->m_lon, _T ( "E" ) );

                        oNMEA0183.GPwpl.To = prp->GetName().Truncate ( 8 );

                        oNMEA0183.GPwpl.Write ( snt );
                    }

                    SendNMEAMessage( snt.Sentence );

                    wxString msg(_T("-->GPS Port:"));
                    msg += com_name;
                    msg += _T(" Sentence: ");
                    msg += snt.Sentence;
                    msg.Trim();
                    wxLogMessage(msg);

                    if ( pProgress )
                    {
                        pProgress->SetValue ( ( ip * 100 ) / nProg );
                        pProgress->Refresh();
                        pProgress->Update();
                    }

                    wxMilliSleep ( progress_stall );

                    node = node->GetNext();

                    ip++;
                }
            }

            // Create the NMEA Rte sentence

            oNMEA0183.Rte.Empty();
            oNMEA0183.Rte.TypeOfRoute = CompleteRoute;

            if ( pr->m_RouteNameString.IsEmpty() )
                oNMEA0183.Rte.RouteName = _T ( "1" );
            else
                oNMEA0183.Rte.RouteName = pr->m_RouteNameString;

            if(g_GPS_Ident == _T("FurunoGP3X"))
            {
                oNMEA0183.Rte.RouteName = _T ( "1" );
                oNMEA0183.TalkerID = _T ( "GP" );
            }

            oNMEA0183.Rte.total_number_of_messages = 1;
            oNMEA0183.Rte.message_number = 1;

            // add the waypoints
            wxRoutePointListNode *node = pr->pRoutePointList->GetFirst();
            while ( node )
            {
                RoutePoint *prp = node->GetData();
                wxString name = prp->GetName().Truncate ( 6 );

                if(g_GPS_Ident == _T("FurunoGP3X"))
                {
                      name = prp->GetName().Truncate ( 7 );
                      name.Prepend(_T(" "));
                }

                oNMEA0183.Rte.AddWaypoint ( name );
                node = node->GetNext();
            }

            oNMEA0183.Rte.Write ( snt );

            unsigned int max_length = 70;

            if(snt.Sentence.Len() > max_length)
            {
                // Make a route with one waypoint to get tare load.
                NMEA0183 tNMEA0183;
                SENTENCE tsnt;
                tNMEA0183.TalkerID = _T ( "EC" );

                tNMEA0183.Rte.Empty();
                tNMEA0183.Rte.TypeOfRoute = CompleteRoute;

                if(g_GPS_Ident != _T("FurunoGP3X"))
                {
                    if ( pr->m_RouteNameString.IsEmpty() )
                        tNMEA0183.Rte.RouteName = _T ( "1" );
                    else
                        tNMEA0183.Rte.RouteName = pr->m_RouteNameString;

                    tNMEA0183.Rte.AddWaypoint ( _T("123456") );
                }
                else
                {
                    if (( pr->m_RouteNameString.IsNumber() ) &&
                        ( pr->m_RouteNameString.Len() <= 2 ) ) {
                            if( pr->m_RouteNameString.Len() == 2) {
                                tNMEA0183.Rte.RouteName = pr->m_RouteNameString;
                            }
                            else {
                                tNMEA0183.Rte.RouteName = _T("0");
                                tNMEA0183.Rte.RouteName += pr->m_RouteNameString;
                            }
                        }
                    else
                    {
                        tNMEA0183.Rte.RouteName = _T ( "01" );
                    }
                    tNMEA0183.Rte.AddWaypoint ( _T(" 1234567") );
                }


                tNMEA0183.Rte.Write ( tsnt );

                unsigned int tare_length = tsnt.Sentence.Len();

                wxArrayString sentence_array;

                // Trial balloon: add the waypoints, with length checking
                int n_total = 1;
                bool bnew_sentence = true;
                int sent_len=0;

                wxRoutePointListNode *node = pr->pRoutePointList->GetFirst();
                while ( node )
                {
                    RoutePoint *prp = node->GetData();
                    unsigned int name_len = prp->GetName().Truncate ( 6 ).Len();
                    if(g_GPS_Ident == _T("FurunoGP3X"))
                        name_len = 1 + prp->GetName().Truncate ( 7 ).Len();


                    if(bnew_sentence)
                    {
                        sent_len = tare_length;
                        sent_len += name_len;
                        bnew_sentence = false;
                        node = node->GetNext();

                    }
                    else
                    {
                        if(sent_len + name_len > max_length)
                        {
                            n_total ++;
                            bnew_sentence = true;
                        }
                        else
                        {
                            sent_len += name_len;
                            node = node->GetNext();
                        }
                    }
                }

                // Now we have the sentence count, so make the real sentences using the same counting logic
                int final_total = n_total;
                int n_run = 1;
                bnew_sentence = true;

                node = pr->pRoutePointList->GetFirst();
                while ( node )
                {
                    RoutePoint *prp = node->GetData();
                    wxString name = prp->GetName().Truncate ( 6 );
                    if(g_GPS_Ident == _T("FurunoGP3X"))
                    {
                        name = prp->GetName().Truncate ( 7 );
                        name.Prepend(_T(" "));
                    }

                    unsigned int name_len = name.Len();

                    if(bnew_sentence)
                    {
                        sent_len = tare_length;
                        sent_len += name_len;
                        bnew_sentence = false;

                        oNMEA0183.Rte.Empty();
                        oNMEA0183.Rte.TypeOfRoute = CompleteRoute;

                        if(g_GPS_Ident != _T("FurunoGP3X"))
                        {
                              if ( pr->m_RouteNameString.IsEmpty() )
                                    oNMEA0183.Rte.RouteName = _T ( "1" );
                              else
                                    oNMEA0183.Rte.RouteName = pr->m_RouteNameString;
                        }
                        else {
                            if (( pr->m_RouteNameString.IsNumber() ) &&
                                ( pr->m_RouteNameString.Len() <= 2 ) ) {
                                    if( pr->m_RouteNameString.Len() == 2) {
                                        oNMEA0183.Rte.RouteName = pr->m_RouteNameString;
                                    }
                                    else {
                                        oNMEA0183.Rte.RouteName = _T("0");
                                        oNMEA0183.Rte.RouteName += pr->m_RouteNameString;
                                    }
                                }
                                else {
                                    oNMEA0183.Rte.RouteName = _T ( "01" );
                                }
                        }


                        oNMEA0183.Rte.total_number_of_messages = final_total;
                        oNMEA0183.Rte.message_number = n_run;
                        snt.Sentence.Clear();

                        oNMEA0183.Rte.AddWaypoint ( name );
                        node = node->GetNext();
                    }
                    else
                    {
                        if(sent_len + name_len > max_length)
                        {
                            n_run ++;
                            bnew_sentence = true;

                            oNMEA0183.Rte.Write ( snt );
                    // printf("%s", snt.Sentence.mb_str());

                            sentence_array.Add(snt.Sentence);
                        }
                        else
                        {
                            sent_len += name_len;
                            oNMEA0183.Rte.AddWaypoint ( name );
                            node = node->GetNext();
                        }
                    }
                }

                oNMEA0183.Rte.Write ( snt ); //last one...
                if(snt.Sentence.Len() > tare_length)
                      sentence_array.Add(snt.Sentence);

                for(unsigned int ii=0 ; ii < sentence_array.GetCount(); ii++)
                {
                    SendNMEAMessage( sentence_array.Item(ii) );

                    wxString msg(_T("-->GPS Port:"));
                    msg += com_name;
                    msg += _T(" Sentence: ");
                    msg += sentence_array.Item(ii);
                    msg.Trim();
                    wxLogMessage(msg);

                    wxMilliSleep ( 500 );
                }

            }
            else
            {
                SendNMEAMessage( snt.Sentence );

                wxString msg(_T("-->GPS Port:"));
                msg += com_name;
                msg += _T(" Sentence: ");
                msg += snt.Sentence;
                msg.Trim();
                wxLogMessage(msg);
            }

            if(g_GPS_Ident == _T("FurunoGP3X"))
            {
                wxString term;
                term.Printf(_T("$PFEC,GPxfr,CTL,E%c%c"), 0x0d, 0x0a);

                SendNMEAMessage( term );

                wxString msg(_T("-->GPS Port:"));
                msg += com_name;
                msg += _T(" Sentence: ");
                msg += term;
                msg.Trim();
                wxLogMessage(msg);
            }

            if ( pProgress )
            {
                pProgress->SetValue ( 100 );
                pProgress->Refresh();
                pProgress->Update();
            }

            wxMilliSleep ( 500 );

            ret_bool = true;
            return ret_bool;
        }
    }

    return ret_bool;
}


bool Multiplexer::SendWaypointToGPS(RoutePoint *prp, wxString &com_name, wxGauge *pProgress)
{
    bool ret_bool = false;
/*TODO: RE-enable use of Garmin host
#ifdef USE_GARMINHOST
#ifdef __WXMSW__
    if(com_name.Upper().Matches(_T("*GARMIN*"))) // Garmin USB Mode
    {
        if(m_pdevmon)
            m_pdevmon->StopIOThread(true);

        int v_init = Garmin_GPS_Init(NULL, wxString(_T("usb:")));

        if(v_init < 0)
        {
            wxString msg(_T(" Garmin USB GPS could not be initialized"));
            wxLogMessage(msg);
            msg.Printf(_T(" Error Code is %d"), v_init);
            wxLogMessage(msg);
            msg = _T(" LastGarminError is: ");
            msg += GetLastGarminError();
            wxLogMessage(msg);

            ret_bool = false;
        }
        else
        {
            wxLogMessage(_T("Garmin USB Initialized"));

            wxString msg = _T("USB Unit identifies as: ");
            wxString GPS_Unit = Garmin_GPS_GetSaveString();
            msg += GPS_Unit;
            wxLogMessage(msg);

            wxLogMessage(_T("Sending Waypoint..."));

            // Create a RoutePointList with one item
            RoutePointList rplist;
            rplist.Append(prp);

            int ret1 = Garmin_GPS_SendWaypoints(NULL, wxString(_T("usb:")), &rplist);

            if(ret1 != 1)
            {
                wxLogMessage(_T(" Error Sending Waypoint to Garmin USB"));
                wxString msg;
                msg = _T(" LastGarminError is: ");
                msg += GetLastGarminError();
                wxLogMessage(msg);

                ret_bool = false;
            }
            else
                ret_bool = true;
        }

        if(m_pdevmon)
            m_pdevmon->RestartIOThread();

        return ret_bool;
    }
#endif

    // Are we using Garmin Host mode for uploads?
    if(m_bGarmin_host)
    {
        RoutePointList rplist;
        int ret_val;

        // Request that the thread should pause
        m_brequest_thread_pause = true;

        bool b_gotPort = false;
        // Poll, waiting for the Mutex. Abort after 5 seconds
        long time = ::wxGetLocalTime();
        while(::wxGetLocalTime() < time + 5)
        {
            if(wxMUTEX_BUSY != m_pPortMutex->TryLock())
            {
                b_gotPort = true;
                break;
            }
        }

        if(!b_gotPort)
        {
            wxString msg(_T("Error Sending waypoint(s)...could not lock Mutex on port: "));
            msg +=com_name;
            wxLogMessage(msg);

            return false;
        }


        // Initialize the Garmin receiver, build required Jeeps internal data structures
        int v_init = Garmin_GPS_Init(g_pCommMan, com_name);
        if(v_init < 0)
        {
            wxString msg(_T("Garmin GPS could not be initialized on port: "));
            msg +=com_name;
            wxString err;
            err.Printf(_T(" Error Code is %d"), v_init);
            msg += err;

            msg += _T("\n LastGarminError is: ");
            msg += GetLastGarminError();

            wxLogMessage(msg);

            ret_bool = false;
            goto ret_point;
        }
        else
        {
            wxString msg(_T("Sent waypoint(s) to Garmin GPS on port: "));
            msg +=com_name;
            msg += _T("\n Unit identifies as: ");
            wxString GPS_Unit = Garmin_GPS_GetSaveString();
            msg += GPS_Unit;
            wxLogMessage(msg);
        }

        // Create a RoutePointList with one item
        rplist.Append(prp);

        ret_val = Garmin_GPS_SendWaypoints(g_pCommMan, com_name, &rplist);
        if(ret_val != 1)
        {
            wxString msg(_T("Error Sending Waypoint(s) to Garmin GPS on port: "));
            msg +=com_name;
            wxString err;
            err.Printf(_T(" Error Code is %d"), ret_val);
            msg += err;

            msg += _T("\n LastGarminError is: ");
            msg += GetLastGarminError();

            wxLogMessage(msg);

            ret_bool = false;
            goto ret_point;
        }
        else
            ret_bool = true;

ret_point:
        // Release the Mutex
        m_brequest_thread_pause = false;
        m_pPortMutex->Unlock();

        return ret_bool;
    }
    else
#endif //USE_GARMINHOST
*/
    { // Standard NMEA mode
        SENTENCE snt;
        NMEA0183 oNMEA0183;
        oNMEA0183.TalkerID = _T ( "EC" );

        if ( pProgress )
            pProgress->SetRange ( 100 );

        if(g_GPS_Ident == _T("Generic"))
        {
            if ( prp->m_lat < 0. )
                oNMEA0183.Wpl.Position.Latitude.Set ( -prp->m_lat, _T ( "S" ) );
            else
                oNMEA0183.Wpl.Position.Latitude.Set ( prp->m_lat, _T ( "N" ) );

            if ( prp->m_lon < 0. )
                oNMEA0183.Wpl.Position.Longitude.Set ( -prp->m_lon, _T ( "W" ) );
            else
                oNMEA0183.Wpl.Position.Longitude.Set ( prp->m_lon, _T ( "E" ) );

            oNMEA0183.Wpl.To = prp->GetName().Truncate ( 6 );

            oNMEA0183.Wpl.Write ( snt );
        }
        else if(g_GPS_Ident == _T("FurunoGP3X"))
        {
            oNMEA0183.TalkerID = _T ( "PFEC," );

            if ( prp->m_lat < 0. )
                oNMEA0183.GPwpl.Position.Latitude.Set ( -prp->m_lat, _T ( "S" ) );
            else
                oNMEA0183.GPwpl.Position.Latitude.Set ( prp->m_lat, _T ( "N" ) );

            if ( prp->m_lon < 0. )
                oNMEA0183.GPwpl.Position.Longitude.Set ( -prp->m_lon, _T ( "W" ) );
            else
                oNMEA0183.GPwpl.Position.Longitude.Set ( prp->m_lon, _T ( "E" ) );


            oNMEA0183.GPwpl.To = prp->GetName().Truncate ( 8 );

            oNMEA0183.GPwpl.Write ( snt );
        }

        SendNMEAMessage( snt.Sentence );

        wxString msg(_T("-->GPS Port:"));
        msg += com_name;
        msg += _T(" Sentence: ");
        msg += snt.Sentence;
        msg.Trim();
        wxLogMessage(msg);

        if(g_GPS_Ident == _T("FurunoGP3X"))
        {
            wxString term;
            term.Printf(_T("$PFEC,GPxfr,CTL,E%c%c"), 0x0d, 0x0a);

            SendNMEAMessage( term );

            wxString msg(_T("-->GPS Port:"));
            msg += com_name;
            msg += _T(" Sentence: ");
            msg += term;
            msg.Trim();
            wxLogMessage(msg);
        }

        if ( pProgress )
        {
            pProgress->SetValue ( 100 );
            pProgress->Refresh();
            pProgress->Update();
        }

        wxMilliSleep ( 500 );

        ret_bool = true;
        return ret_bool;
    }

}

