/***************************************************************************
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
 **************************************************************************/

#include "wx/wx.h"

#include "multiplexer.h"
#include "navutil.h"
#include "NMEALogWindow.h"
#include "garmin/jeeps/garmin_wrapper.h"
#include "OCPN_DataStreamEvent.h"

extern PlugInManager    *g_pi_manager;
extern wxString         g_GPS_Ident;
extern bool             g_bGarminHostUpload;
extern bool             g_bWplIsAprsPosition;
extern wxArrayOfConnPrm  *g_pConnectionParams;
extern bool             g_bserial_access_checked;
extern bool             g_b_legacy_input_filter_behaviour;

extern "C" bool CheckSerialAccess( void );

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
        delete m_pdatastreams->Item(i);         // Implicit Close(), see datastream dtor
    }
    m_pdatastreams->Clear();
}

DataStream *Multiplexer::FindStream(const wxString & port)
{
    for (size_t i = 0; i < m_pdatastreams->Count(); i++)
    {
        DataStream *stream = m_pdatastreams->Item(i);
        if( stream && stream->GetPort() == port )
            return stream;
    }
    return NULL;
}

void Multiplexer::StopAndRemoveStream( DataStream *stream )
{
    if( stream )
        stream->Close();

    if( m_pdatastreams ) {
        int index = m_pdatastreams->Index( stream );
        if( wxNOT_FOUND != index )
            m_pdatastreams->RemoveAt( index );
    }
}

void Multiplexer::StartAllStreams( void )
{
    for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
    {
        ConnectionParams *cp = g_pConnectionParams->Item(i);
        if( cp->bEnabled ) {

#ifdef __WXGTK__
            if( cp->GetDSPort().Contains(_T("Serial"))) {
                if( ! g_bserial_access_checked ){
                    if( !CheckSerialAccess() ){
                    }
                    g_bserial_access_checked = true;
                }
            }
#endif

            dsPortType port_type = cp->IOSelect;
            DataStream *dstr = new DataStream( this,
                                               cp->Type,
                                               cp->GetDSPort(),
                                               wxString::Format(wxT("%i"),cp->Baudrate),
                                               port_type,
                                               cp->Priority,
                                               cp->Garmin
                                               );
                                               dstr->SetInputFilter(cp->InputSentenceList);
                                               dstr->SetInputFilterType(cp->InputSentenceListType);
                                               dstr->SetOutputFilter(cp->OutputSentenceList);
                                               dstr->SetOutputFilterType(cp->OutputSentenceListType);
                                               dstr->SetChecksumCheck(cp->ChecksumCheck);

            cp->b_IsSetup = true;

            AddStream(dstr);
        }
    }

}



void Multiplexer::LogOutputMessageColor(const wxString &msg, const wxString & stream_name, const wxString & color)
{
    if (NMEALogWindow::Get().Active()) {
        wxDateTime now = wxDateTime::Now();
        wxString ss;
#ifndef __WXQT__        //  Date/Time on Qt are broken, at least for android
        ss = now.FormatISOTime();
#endif        
        ss.Prepend(_T("--> "));
        ss.Append( _T(" (") );
        ss.Append( stream_name );
        ss.Append( _T(") ") );
        ss.Append( msg );
        ss.Prepend( color );

        NMEALogWindow::Get().Add(ss);
    }
}



void Multiplexer::LogOutputMessage(const wxString &msg, wxString stream_name, bool b_filter)
{
    if(b_filter)
        LogOutputMessageColor( msg, stream_name, _T("<CORAL>") );
    else
        LogOutputMessageColor( msg, stream_name, _T("<BLUE>") );
}

void Multiplexer::LogInputMessage(const wxString &msg, const wxString & stream_name, bool b_filter, bool b_error)
{
    if (NMEALogWindow::Get().Active()) {
        wxDateTime now = wxDateTime::Now();
        wxString ss;
#ifndef __WXQT__        //  Date/Time on Qt are broken, at least for android
        ss = now.FormatISOTime();
#endif        
        ss.Append( _T(" (") );
        ss.Append( stream_name );
        ss.Append( _T(") ") );
        ss.Append( msg );
        if(b_error){
            ss.Prepend( _T("<RED>") );
        }
        else{
            if(b_filter)
                if (g_b_legacy_input_filter_behaviour)
                    ss.Prepend( _T("<CORAL>") );
                else
                    ss.Prepend( _T("<MAROON>") );
            else
                ss.Prepend( _T("<GREEN>") );
        }

        NMEALogWindow::Get().Add( ss );
    }
}


void Multiplexer::SendNMEAMessage(const wxString &msg)
{
    //Send to all the outputs
    for (size_t i = 0; i < m_pdatastreams->Count(); i++)
    {
        DataStream* s = m_pdatastreams->Item(i);

        if ( s->IsOk() && (s->GetIoSelect() == DS_TYPE_INPUT_OUTPUT || s->GetIoSelect() == DS_TYPE_OUTPUT) ) {
            bool bout_filter = true;

            bool bxmit_ok = true;
            if(s->SentencePassesFilter( msg, FILTER_OUTPUT ) ) {
                bxmit_ok = s->SendSentence(msg);
                bout_filter = false;
            }
            //Send to the Debug Window, if open
            if( !bout_filter ) {
                if( bxmit_ok )
                    LogOutputMessageColor( msg, s->GetPort(), _T("<BLUE>") );
                else
                    LogOutputMessageColor( msg, s->GetPort(), _T("<RED>") );
            }
            else
                LogOutputMessageColor( msg, s->GetPort(), _T("<CORAL>") );
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
    wxString message = wxString(event.GetNMEAString().c_str(), wxConvUTF8);

    DataStream *stream = event.GetStream();
    wxString port(_T("Virtual:"));
    if( stream )
        port = wxString(stream->GetPort());

    if( !message.IsEmpty() )
    {
        //Send to core consumers
        //if it passes the source's input filter
        //  If there is no datastream, as for PlugIns, then pass everything
        bool bpass = true;
        if( stream )
            bpass = stream->SentencePassesFilter( message, FILTER_INPUT );

        if( bpass ) {
            if( message.Mid(3,3).IsSameAs(_T("VDM")) ||
                message.Mid(1,5).IsSameAs(_T("FRPOS")) ||
                message.Mid(1,2).IsSameAs(_T("CD")) ||
                message.Mid(3,3).IsSameAs(_T("TLL")) ||
                message.Mid(3,3).IsSameAs(_T("TTM")) ||
                message.Mid(3,3).IsSameAs(_T("OSD")) ||
                ( g_bWplIsAprsPosition && message.Mid(3,3).IsSameAs(_T("WPL")) ) )
            {
                if( m_aisconsumer )
                    m_aisconsumer->AddPendingEvent(event);
            }
            else
            {
                if( m_gpsconsumer )
                    m_gpsconsumer->AddPendingEvent(event);
            }
        }

        if ((g_b_legacy_input_filter_behaviour && !bpass) || bpass) {

            //Send to plugins
            if ( g_pi_manager )
                g_pi_manager->SendNMEASentenceToAllPlugIns( message );

           //Send to all the other outputs
            for (size_t i = 0; i < m_pdatastreams->Count(); i++)
            {
                DataStream* s = m_pdatastreams->Item(i);
                if ( s->IsOk() ) {
                    if((s->GetConnectionType() == SERIAL)  || (s->GetPort() != port)) {
                        if ( s->GetIoSelect() == DS_TYPE_INPUT_OUTPUT || s->GetIoSelect() == DS_TYPE_OUTPUT ) {
                            bool bout_filter = true;

                            bool bxmit_ok = true;
                            if(s->SentencePassesFilter( message, FILTER_OUTPUT ) ) {
                                bxmit_ok = s->SendSentence(message);
                                bout_filter = false;
                            }

                            //Send to the Debug Window, if open
                            if( !bout_filter ) {
                                if( bxmit_ok )
                                    LogOutputMessageColor( message, s->GetPort(), _T("<BLUE>") );
                                else
                                    LogOutputMessageColor( message, s->GetPort(), _T("<RED>") );
                            }
                            else
                                LogOutputMessageColor( message, s->GetPort(), _T("<CORAL>") );
                        }
                    }
                }
            }
        }

            //Send to the Debug Window, if open
            //  Special formatting for non-printable characters helps debugging NMEA problems
        if (NMEALogWindow::Get().Active()) {
            std::string str= event.GetNMEAString();    
            wxString fmsg;
            
            bool b_error = false;
            for ( std::string::iterator it=str.begin(); it!=str.end(); ++it){
                if(isprint(*it))
                    fmsg += *it;
                else{
                    wxString bin_print;
                    bin_print.Printf(_T("<0x%02X>"), *it);
                    fmsg += bin_print;
                    if((*it != 0x0a) && (*it != 0x0d))
                        b_error = true;
                }
                
            }
            LogInputMessage( fmsg, port, !bpass, b_error );
        }
    }
}

void Multiplexer::SaveStreamProperties( DataStream *stream )
{
    if( stream ) {
        type_save = stream->GetConnectionType();
        port_save = stream->GetPort();
        baud_rate_save = stream->GetBaudRate();
        port_type_save = stream->GetPortType();
        priority_save = stream->GetPriority();
        input_sentence_list_save = stream->GetInputSentenceList();
        input_sentence_list_type_save = stream->GetInputSentenceListType();
        output_sentence_list_save = stream->GetOutputSentenceList();
        output_sentence_list_type_save = stream->GetOutputSentenceListType();
        bchecksum_check_save = stream->GetChecksumCheck();
        bGarmin_GRMN_mode_save = stream->GetGarminMode();
    }
}

bool Multiplexer::CreateAndRestoreSavedStreamProperties()
{
    DataStream *dstr = new DataStream( this,
                                       type_save,
                                       port_save,
                                       baud_rate_save,
                                       port_type_save,
                                       priority_save,
                                       bGarmin_GRMN_mode_save
                                     );
    dstr->SetInputFilter(input_sentence_list_save);
    dstr->SetInputFilterType(input_sentence_list_type_save);
    dstr->SetOutputFilter(output_sentence_list_save);
    dstr->SetOutputFilterType(output_sentence_list_type_save);
    dstr->SetChecksumCheck(bchecksum_check_save);

    AddStream(dstr);

    return true;
}


int Multiplexer::SendRouteToGPS(Route *pr, const wxString &com_name, bool bsend_waypoints, wxGauge *pProgress)
{
    int ret_val = 0;
    DataStream *old_stream = FindStream( com_name );
    if( old_stream ) {
        SaveStreamProperties( old_stream );
        StopAndRemoveStream( old_stream );
    }

#ifdef USE_GARMINHOST
#ifdef __WXMSW__
    if(com_name.Upper().Matches(_T("*GARMIN*"))) // Garmin USB Mode
    {
//        if(m_pdevmon)
//            m_pdevmon->StopIOThread(true);

        int v_init = Garmin_GPS_Init(wxString(_T("usb:")));

        if(v_init < 0)
        {
            wxString msg(_T(" Garmin USB GPS could not be initialized"));
            wxLogMessage(msg);
            msg.Printf(_T(" Error Code is %d"), v_init);
            wxLogMessage(msg);
            msg = _T(" LastGarminError is: ");
            msg += GetLastGarminError();
            wxLogMessage(msg);

            ret_val = ERR_GARMIN_INITIALIZE;
        }
        else
        {
            wxLogMessage(_T("Garmin USB Initialized"));

            wxString msg = _T("USB Unit identifies as: ");
            wxString GPS_Unit = Garmin_GPS_GetSaveString();
            msg += GPS_Unit;
            wxLogMessage(msg);

            wxLogMessage(_T("Sending Routes..."));
            int ret1 = Garmin_GPS_SendRoute(wxString(_T("usb:")), pr, pProgress);

            if(ret1 != 1)
            {
                wxLogMessage(_T(" Error Sending Routes"));
                wxString msg;
                msg = _T(" LastGarminError is: ");
                msg += GetLastGarminError();
                wxLogMessage(msg);

                ret_val = ERR_GARMIN_GENERAL;
            }
            else
                ret_val = 0;
        }

//        if(m_pdevmon)
//            m_pdevmon->RestartIOThread();

        goto ret_point_1;
    }
#endif

    if(g_bGarminHostUpload)
    {
        int lret_val;
        if ( pProgress )
        {
            pProgress->SetValue ( 20 );
            pProgress->Refresh();
            pProgress->Update();
        }

        wxString short_com = com_name.Mid(7);
        // Initialize the Garmin receiver, build required Jeeps internal data structures
        int v_init = Garmin_GPS_Init(short_com);
        if(v_init < 0)
        {
            wxString msg(_T("Garmin GPS could not be initialized on port: "));
            msg +=short_com;
            wxString err;
            err.Printf(_T(" Error Code is %d"), v_init);
            msg += err;

            msg += _T("\n LastGarminError is: ");
            msg += GetLastGarminError();

            wxLogMessage(msg);

            ret_val = ERR_GARMIN_INITIALIZE;
            goto ret_point;
        }
        else
        {
            wxString msg(_T("Sent Route to Garmin GPS on port: "));
            msg +=short_com;
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

        lret_val = Garmin_GPS_SendRoute(short_com, pr, pProgress);
        if(lret_val != 1)
        {
            wxString msg(_T("Error Sending Route to Garmin GPS on port: "));
            msg +=short_com;
            wxString err;
            err.Printf(_T(" Error Code is %d"), ret_val);

            msg += _T("\n LastGarminError is: ");
            msg += GetLastGarminError();

            msg += err;
            wxLogMessage(msg);

            ret_val = ERR_GARMIN_GENERAL;
            goto ret_point;
        }
        else
            ret_val = 0;

ret_point:

        if ( pProgress )
        {
            pProgress->SetValue ( 100 );
            pProgress->Refresh();
            pProgress->Update();
        }

        wxMilliSleep ( 500 );

        goto ret_point_1;
    }
    else
#endif //USE_GARMINHOST

    {
        { // Standard NMEA mode

            //  If the port was temporarily closed, reopen as I/O type
            //  Otherwise, open another port using default properties
            wxString baud;

            if( old_stream ) {
                baud = baud_rate_save;
            }
            else {
                baud = _T("4800");
            }

            DataStream *dstr = new DataStream( this,
                                               SERIAL,
                                               com_name,
                                               baud,
                                               DS_TYPE_INPUT_OUTPUT,
                                               0 );

            //  Wait up to 5 seconds for Datastream secondary thread to come up
            int timeout = 0;
            while( !dstr-> IsSecThreadActive()  && (timeout < 50)) {
                wxMilliSleep(100);
                timeout++;
            }

            if( !dstr-> IsSecThreadActive() ){
                wxString msg(_T("-->GPS Port:"));
                msg += com_name;
                msg += _T(" ...Could not be opened for writing");
                wxLogMessage(msg);

                dstr->Close();
                goto ret_point_1;
            }

            SENTENCE snt;
            NMEA0183 oNMEA0183;
            oNMEA0183.TalkerID = _T ( "EC" );

            int nProg = pr->pRoutePointList->GetCount() + 1;
            if ( pProgress )
                pProgress->SetRange ( 100 );

            int progress_stall = 500;
            if(pr->pRoutePointList->GetCount() > 10)
                progress_stall = 200;

            if(!pProgress)
                progress_stall = 200;   // 80 chars at 4800 baud is ~160 msec

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

                        wxString name = prp->GetName();
                        name += _T("000000");
                        name.Truncate( 6 );
                        oNMEA0183.GPwpl.To = name;

                        oNMEA0183.GPwpl.Write ( snt );
                    }

                    if( dstr->SendSentence( snt.Sentence ) )
                        LogOutputMessage( snt.Sentence, dstr->GetPort(), false );

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
            // Try to create a single sentence, and then check the length to see if too long
            unsigned int max_length = 76;
            unsigned int max_wp = 2;                     // seems to be required for garmin...

            //  Furuno GPS can only accept 5 (five) waypoint linkage sentences....
            //  So, we need to compact a few more points into each link sentence.
            if(g_GPS_Ident == _T("FurunoGP3X")){
                max_wp = 6;
            }

            oNMEA0183.Rte.Empty();
            oNMEA0183.Rte.TypeOfRoute = CompleteRoute;

            if ( pr->m_RouteNameString.IsEmpty() )
                oNMEA0183.Rte.RouteName = _T ( "1" );
            else
                oNMEA0183.Rte.RouteName = pr->m_RouteNameString;

            if(g_GPS_Ident == _T("FurunoGP3X"))
            {
                oNMEA0183.Rte.RouteName = _T ( "01" );
                oNMEA0183.TalkerID = _T ( "GP" );
                oNMEA0183.Rte.m_complete_char = 'C';   // override the default "c"
                oNMEA0183.Rte.m_skip_checksum = 1;     // no checksum needed
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
                    name = prp->GetName();
                    name += _T("000000");
                    name.Truncate( 6 );
                    name .Prepend( _T(" "));        // What Furuno calls "Skip Code", space means use the WP
                }

                oNMEA0183.Rte.AddWaypoint ( name );
                node = node->GetNext();
            }

            oNMEA0183.Rte.Write ( snt );

            if( (snt.Sentence.Len() > max_length)
                || (pr->pRoutePointList->GetCount() > max_wp) )        // Do we need split sentences?
            {
                // Make a route with zero waypoints to get tare load.
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

                }
                else
                {
                    tNMEA0183.Rte.RouteName = _T ( "01" );
                }


                tNMEA0183.Rte.Write ( tsnt );

                unsigned int tare_length = tsnt.Sentence.Len();

                wxArrayString sentence_array;

                // Trial balloon: add the waypoints, with length checking
                int n_total = 1;
                bool bnew_sentence = true;
                int sent_len=0;
                unsigned int wp_count = 0;

                wxRoutePointListNode *node = pr->pRoutePointList->GetFirst();
                while ( node )
                {
                    RoutePoint *prp = node->GetData();
                    unsigned int name_len = prp->GetName().Truncate ( 6 ).Len();
                    if(g_GPS_Ident == _T("FurunoGP3X"))
                        name_len = 7;           // six chars, with leading space for "Skip Code"


                    if(bnew_sentence)
                    {
                        sent_len = tare_length;
                        sent_len += name_len + 1;        // with comma
                        bnew_sentence = false;
                        node = node->GetNext();
                        wp_count = 1;

                    }
                    else
                    {
                        if( (sent_len + name_len > max_length) || (wp_count >= max_wp) )
                        {
                            n_total ++;
                            bnew_sentence = true;
                        }
                        else
                        {
                            sent_len += name_len + 1;   // with comma
                            wp_count++;
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
                        name = prp->GetName();
                        name += _T("000000");
                        name.Truncate( 6 );
                        name .Prepend( _T(" "));        // What Furuno calls "Skip Code", space means use the WP
                    }

                    unsigned int name_len = name.Len();

                    if(bnew_sentence)
                    {
                        sent_len = tare_length;
                        sent_len += name_len + 1;       // comma
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
                            oNMEA0183.Rte.RouteName = _T ( "01" );
                        }


                        oNMEA0183.Rte.total_number_of_messages = final_total;
                        oNMEA0183.Rte.message_number = n_run;
                        snt.Sentence.Clear();
                        wp_count = 1;

                        oNMEA0183.Rte.AddWaypoint ( name );
                        node = node->GetNext();
                    }
                    else
                    {
                        if( (sent_len + name_len > max_length) || (wp_count >= max_wp) )
                        {
                            n_run ++;
                            bnew_sentence = true;

                            oNMEA0183.Rte.Write ( snt );

                            sentence_array.Add(snt.Sentence);
                        }
                        else
                        {
                            sent_len += name_len + 1;   // comma
                            oNMEA0183.Rte.AddWaypoint ( name );
                            wp_count ++;
                            node = node->GetNext();
                        }
                    }
                }

                oNMEA0183.Rte.Write ( snt ); //last one...
                if(snt.Sentence.Len() > tare_length)
                      sentence_array.Add(snt.Sentence);

                for(unsigned int ii=0 ; ii < sentence_array.GetCount(); ii++)
                {
                    wxString sentence = sentence_array.Item(ii);
                    
                    if(dstr->SendSentence( sentence ) )
                        LogOutputMessage( sentence, dstr->GetPort(), false );

                    wxString msg(_T("-->GPS Port:"));
                    msg += com_name;
                    msg += _T(" Sentence: ");
                    msg += sentence;
                    msg.Trim();
                    wxLogMessage(msg);

                    wxMilliSleep ( progress_stall );
                }

            }
            else
            {
                if( dstr->SendSentence( snt.Sentence ) )
                    LogOutputMessage( snt.Sentence, dstr->GetPort(), false );

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

                if( dstr->SendSentence( term ) )
                    LogOutputMessage( term, dstr->GetPort(), false );

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

            wxMilliSleep ( progress_stall );

            ret_val = 0;

            //  All finished with the temp port
            dstr->Close();
        }
    }

ret_point_1:

    if( old_stream )
        CreateAndRestoreSavedStreamProperties();

    return ret_val;
}


int Multiplexer::SendWaypointToGPS(RoutePoint *prp, const wxString &com_name, wxGauge *pProgress)
{
    int ret_val = 0;
    DataStream *old_stream = FindStream( com_name );
    if( old_stream ) {
        SaveStreamProperties( old_stream );
        StopAndRemoveStream( old_stream );
    }

#ifdef USE_GARMINHOST
#ifdef __WXMSW__
    if(com_name.Upper().Matches(_T("*GARMIN*"))) // Garmin USB Mode
    {
//        if(m_pdevmon)
//            m_pdevmon->StopIOThread(true);

        int v_init = Garmin_GPS_Init(wxString(_T("usb:")));

        if(v_init < 0)
        {
            wxString msg(_T(" Garmin USB GPS could not be initialized"));
            wxLogMessage(msg);
            msg.Printf(_T(" Error Code is %d"), v_init);
            wxLogMessage(msg);
            msg = _T(" LastGarminError is: ");
            msg += GetLastGarminError();
            wxLogMessage(msg);

            ret_val = ERR_GARMIN_INITIALIZE;
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

            int ret1 = Garmin_GPS_SendWaypoints(wxString(_T("usb:")), &rplist);

            if(ret1 != 1)
            {
                wxLogMessage(_T(" Error Sending Waypoint to Garmin USB"));
                wxString msg;
                msg = _T(" LastGarminError is: ");
                msg += GetLastGarminError();
                wxLogMessage(msg);

                ret_val = ERR_GARMIN_GENERAL;
            }
            else
                ret_val = 0;
        }

//        if(m_pdevmon)
//            m_pdevmon->RestartIOThread();

        return ret_val;
    }
#endif

    // Are we using Garmin Host mode for uploads?
    if(g_bGarminHostUpload)
    {
        RoutePointList rplist;
        int ret_val;

        wxString short_com = com_name.Mid(7);
        // Initialize the Garmin receiver, build required Jeeps internal data structures
        int v_init = Garmin_GPS_Init(short_com);
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

            ret_val = ERR_GARMIN_INITIALIZE;
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

        ret_val = Garmin_GPS_SendWaypoints(short_com, &rplist);
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

            ret_val = ERR_GARMIN_GENERAL;
            goto ret_point;
        }
        else
            ret_val = 0;

        goto ret_point;
    }
    else
#endif //USE_GARMINHOST

    { // Standard NMEA mode

    //  If the port was temporarily closed, reopen as I/O type
    //  Otherwise, open another port using default properties
    wxString baud;

    if( old_stream ) {
        baud = baud_rate_save;
    }
    else {
        baud = _T("4800");
    }

    DataStream *dstr = new DataStream( this,
                                       SERIAL,
                                       com_name,
                                       baud,
                                       DS_TYPE_INPUT_OUTPUT,
                                       0 );


    //  Wait up to 1 seconds for Datastream secondary thread to come up
    int timeout = 0;
    while( !dstr-> IsSecThreadActive()  && (timeout < 50)) {
        wxMilliSleep(100);
        timeout++;
    }

    if( !dstr-> IsSecThreadActive() ){
        wxString msg(_T("-->GPS Port:"));
        msg += com_name;
        msg += _T(" ...Could not be opened for writing");
        wxLogMessage(msg);

        dstr->Close();
        goto ret_point;
    }



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


            wxString name = prp->GetName();
            name += _T("000000");
            name.Truncate( 6 );

            oNMEA0183.GPwpl.To = name;

            oNMEA0183.GPwpl.Write ( snt );
        }

        if( dstr->SendSentence( snt.Sentence ) )
            LogOutputMessage( snt.Sentence, dstr->GetPort(), false );

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

            if( dstr->SendSentence( term ) )
                LogOutputMessage( term, dstr->GetPort(), false );

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

        //  All finished with the temp port
        dstr->Close();

        ret_val = 0;
    }

ret_point:

    if( old_stream )
        CreateAndRestoreSavedStreamProperties();

    return ret_val;
}

