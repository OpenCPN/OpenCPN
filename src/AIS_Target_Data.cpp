/***************************************************************************
 *
 * Project:  OpenCPN
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
 ***************************************************************************
 */

#include "AIS_Target_Data.h"

extern bool bGPSValid;
extern ChartCanvas *cc1;
extern bool g_bAISRolloverShowClass;
extern bool g_bAISRolloverShowCOG;
extern bool g_bAISRolloverShowCPA;
extern bool g_bShowMag;
extern MyFrame *gFrame;
extern bool g_bAISShowTracks;


//    Define and declare a hasmap for ERI Ship type strings, keyed by their UN Codes.
WX_DECLARE_HASH_MAP(int, wxString, wxIntegerHash, wxIntegerEqual, ERIShipTypeHash);

static ERIShipTypeHash s_ERI_hash;

void make_hash_ERI(int key, const wxString & description)
{
	s_ERI_hash[key] = description;
}

void clear_hash_ERI()
{
    s_ERI_hash.clear();
}

static wxString FormatTimeAdaptive( int seconds )
{
    int s = seconds % 60;
    int m = seconds / 60;
    if( seconds < 100 )
        return wxString::Format( _T("%3ds"), seconds );
    else if( seconds < 3600 ) {
        int m = seconds / 60;
        int s = seconds % 60;
        return wxString::Format( _T("%2dmin %02ds"), m, s );
    }
    int h = seconds / 3600;
    m -= h* 60;
    return wxString::Format( _T("%2dh %02dmin"), h, m );
}


AIS_Target_Data::AIS_Target_Data()
{
    strncpy(ShipName, "Unknown             ", 21);
    strncpy(CallSign, "       ", 8);
    strncpy(Destination, "                    ", 21);
    ShipNameExtension[0] = 0;
    b_show_AIS_CPA = false;

    SOG = 555.;
    COG = 666.;
    HDG = 511.;
    ROTAIS = -128;
    Lat = 0.;
    Lon = 0.;

    wxDateTime now = wxDateTime::Now();
    now.MakeGMT();
    PositionReportTicks = now.GetTicks();       // Default is my idea of NOW
    StaticReportTicks = now.GetTicks();
    b_lost = false;

    IMO = 0;
    MID = 555;
    MMSI = 666;
    NavStatus = UNDEFINED;
    SyncState = 888;
    SlotTO = 999;
    ShipType = 19;    // "Unknown"

    CPA = 100;     // Large values avoid false alarms
    TCPA = 100;

    Range_NM = -1.;
    Brg = -1.;

    DimA = DimB = DimC = DimD = 0;;

    ETA_Mo = 0;
    ETA_Day = 0;
    ETA_Hr = 24;
    ETA_Min = 60;

    Draft = 0.;

    RecentPeriod = 0;

    m_utc_hour = 0;
    m_utc_min = 0;
    m_utc_sec = 0;

    Class = AIS_CLASS_A;      // default
    n_alert_state = AIS_NO_ALERT;
    b_suppress_audio = false;
    b_positionDoubtful = false;
    b_positionOnceValid = false;
    b_nameValid = false;

    Euro_Length = 0;            // Extensions for European Inland AIS
    Euro_Beam = 0;
    Euro_Draft = 0;
    strncpy(Euro_VIN, "       ", 8);
    UN_shiptype = 0;

    b_isEuroInland = false;
    b_blue_paddle = false;

    b_NoTrack = false;
    b_OwnShip = false;
    b_PersistTrack = false;
    b_in_ack_timeout = false;

    m_ptrack = new AISTargetTrackList;
    m_ptrack->DeleteContents(true);
    
    b_active = false;
    blue_paddle = 0;
    bCPA_Valid = false;
    ROTIND = 0;
    b_show_track = g_bAISShowTracks;
    b_SarAircraftPosnReport = false;
    altitude = 0;
    b_nameFromCache = false;
    importance = 0.;
    last_scale = 0.50; //new target starts at 50% scale
}

void AIS_Target_Data::CloneFrom( AIS_Target_Data* q )
{
    strncpy(ShipName, q->ShipName, 21);
    strncpy(CallSign, q->CallSign, 8);
    strncpy(Destination, q->Destination, 21);
    ShipNameExtension[0] = 0;
    b_show_AIS_CPA = q->b_show_AIS_CPA;;
    
    SOG = q->SOG;
    COG = q->COG;
    HDG = q->HDG;
    ROTAIS = q->ROTAIS;
    Lat = q->Lat;
    Lon = q->Lon;
    
    PositionReportTicks = q->PositionReportTicks;
    StaticReportTicks = q->StaticReportTicks;
    b_lost = q->b_lost;
    
    IMO = q->IMO;
    MID = q->MID;
    MMSI = q->MMSI;
    NavStatus = q->NavStatus;
    SyncState = q->SyncState;
    SlotTO = q->SlotTO;
    ShipType = q->ShipType;    
    
    CPA = q->CPA;
    TCPA = q->TCPA;
    
    Range_NM = q->Range_NM;
    Brg = q->Brg;
    
    DimA = q->DimA;
    DimB = q->DimB;
    DimC = q->DimC;
    DimD = q->DimD;
    
    ETA_Mo = q->ETA_Mo;
    ETA_Day = q->ETA_Day;
    ETA_Hr = q->ETA_Hr;
    ETA_Min = q->ETA_Min;
    
    Draft = q->Draft;
    
    RecentPeriod = q->RecentPeriod;
    
    m_utc_hour = q->m_utc_hour;
    m_utc_min = q->m_utc_min;
    m_utc_sec = q->m_utc_sec;
    
    Class = q->Class;
    n_alert_state = q->n_alert_state;
    b_suppress_audio = q->b_suppress_audio;
    b_positionDoubtful = q->b_positionDoubtful;
    b_positionOnceValid = q->b_positionOnceValid;
    b_nameValid = q->b_nameValid;
    
    Euro_Length = q->Euro_Length;            // Extensions for European Inland AIS
    Euro_Beam = q->Euro_Beam;
    Euro_Draft = q->Euro_Draft;
    strncpy(Euro_VIN, q->Euro_VIN, 8);
    UN_shiptype = q->UN_shiptype;
    
    b_isEuroInland = q->b_isEuroInland;
    b_blue_paddle = q->b_blue_paddle;
    
    b_OwnShip = q->b_OwnShip;
    b_in_ack_timeout = q->b_in_ack_timeout;
    
    m_ptrack = new AISTargetTrackList;
    m_ptrack->DeleteContents(true);
    
    wxAISTargetTrackListNode *node = q->m_ptrack->GetFirst();
    while( node ) {
        AISTargetTrackPoint *ptrack_point = node->GetData();
        m_ptrack->Append( ptrack_point );
        node = node->GetNext();
    }
    
    
    b_active = q->b_active;
    blue_paddle = q->blue_paddle;
    bCPA_Valid = q->bCPA_Valid;
    ROTIND = q->ROTIND;
    b_show_track = q->b_show_track;
    b_SarAircraftPosnReport = q->b_SarAircraftPosnReport;
    altitude = q->altitude;
}


AIS_Target_Data::~AIS_Target_Data()
{
    m_ptrack->Clear();
    delete m_ptrack;
}

wxString AIS_Target_Data::GetFullName( void )
{
    wxString retName;
    if( b_nameValid ) {
        wxString shipName = trimAISField( ShipName );
        if( shipName == _T("Unknown") )
            retName = wxGetTranslation( shipName );
        else
            retName = shipName;

        if( strlen( ShipNameExtension ) ) {
            wxString shipNameExt = trimAISField( ShipNameExtension );
            retName += shipNameExt;
        }
    }
    
    return retName;
}
    
    
wxString AIS_Target_Data::BuildQueryResult( void )
{
    wxString html;
    wxDateTime now = wxDateTime::Now();

    wxString tableStart = _T("\n<table border=0 cellpadding=1 cellspacing=0>\n");
    wxString tableEnd = _T("</table>\n\n");
    wxString rowStart = _T("<tr><td><font size=-2>");
    wxString rowStartH = _T("<tr><td nowrap>");
    wxString rowSeparator = _T("</font></td><td></td><td><b>");
    wxString rowSeparatorH = _T("</td><td></td><td>");
    wxString colSeparator = _T("<td></td>");
    wxString rowEnd = _T("</b></td></tr>\n");
    wxString vertSpacer = _T("<tr><td></td></tr><tr><td></td></tr><tr><td></td></tr>\n\n");

    wxString IMOstr, MMSIstr, ClassStr;

    html << tableStart << _T("<tr><td nowrap colspan=2>");
    if( /*( Class != AIS_BASE ) &&*/ ( Class != AIS_SART ) ) {
        if( b_nameValid ) {
            html << _T("<font size=+2><i><b>") << GetFullName() ;
            html << _T("</b></i></font>&nbsp;&nbsp;<b>");
        }
    }

    if( ( Class != AIS_ATON ) && ( Class != AIS_BASE ) && ( Class != AIS_GPSG_BUDDY )
            && ( Class != AIS_SART ) ) {
        html << trimAISField( CallSign ) << _T("</b>") << rowEnd;

        if( Class != AIS_CLASS_B ) {
            if( IMO > 0 ) IMOstr = wxString::Format( _T("%08d"), abs( IMO ) );
        }
    }
    else html << _T("</b>") << rowEnd;

    html << vertSpacer;

    if( Class != AIS_GPSG_BUDDY ) {
        MMSIstr = wxString::Format( _T("%09d"), abs( MMSI ) );
    }
    ClassStr = wxGetTranslation( Get_class_string( false ) );
    
    if( Class == AIS_ATON ) {
        wxString cls(_T("AtoN: ") );
        cls += Get_vessel_type_string(false);
        ClassStr = wxGetTranslation( cls );
    }
    
    if(b_SarAircraftPosnReport)
        ClassStr = _("SAR Aircraft");

    if( IMOstr.Length() )
        html << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
            << rowStart <<_("MMSI") << _T("</font></td><td>&nbsp;</td><td><font size=-2>")
            << _("Class") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>")
            << _("IMO") << _T("</font></td></tr>")
            << rowStartH << _T("<b>") << MMSIstr << _T("</b></td><td>&nbsp;</td><td><b>")
            << ClassStr << _T("</b></td><td>&nbsp;</td><td align=right><b>")
            << IMOstr << rowEnd << _T("</table></td></tr>");

    else
       html << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
            << rowStart <<_("MMSI") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>")
            << _("Class") << _T("</font></td></tr>")
            << rowStartH << _T("<b>") << MMSIstr << _T("</b></td><td>&nbsp;</td><td align=right><b>")
            << ClassStr << rowEnd << _T("</table></td></tr>");

    if((Class != AIS_SART ) && ( Class != AIS_BASE ) && ( Class != AIS_DSC ) ) 
        html << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
             << rowStart << _("Flag") << rowEnd << _T("</font></td></tr>")
             << rowStartH << _T("<b>")<< GetCountryCode(true) << rowEnd << _T("</table></td></tr>");
    
    html << vertSpacer;

    wxString navStatStr;
    if( ( Class != AIS_BASE ) && ( Class != AIS_CLASS_B ) && ( Class != AIS_SART ) ) {
        if( ( NavStatus <= 21  ) && ( NavStatus >= 0 ) )
            navStatStr = wxGetTranslation(ais_get_status(NavStatus));
    } else if( Class == AIS_SART ) {
        if( NavStatus == RESERVED_14 ) navStatStr = _("Active");
        else if( NavStatus == UNDEFINED ) navStatStr = _("Testing");
    }

    wxString sart_sub_type;
    if( Class == AIS_SART ) {
        int mmsi_start = MMSI / 1000000;
        switch( mmsi_start ){
            case 970:
//                sart_sub_type = _T("SART");
                break;
            case 972:
                sart_sub_type = _T("MOB");
                break;
            case 974:
                sart_sub_type = _T("EPIRB");
                break;
            default:
                sart_sub_type = _("Unknown");
                break;
        }
    }

    wxString AISTypeStr, UNTypeStr, sizeString;
    if( ( Class != AIS_BASE ) && ( Class != AIS_SART ) && ( Class != AIS_DSC ) ) {

        //      Ship type
        AISTypeStr = wxGetTranslation( Get_vessel_type_string() );

        if( b_isEuroInland && UN_shiptype ) {
            ERIShipTypeHash::iterator it = s_ERI_hash.find( UN_shiptype );
            wxString type;
            if( it == s_ERI_hash.end() ) type = _("Undefined");
            else
                type = it->second;

            UNTypeStr = wxGetTranslation( type );
        }

        if( b_SarAircraftPosnReport ){
            AISTypeStr.Clear();
            UNTypeStr.Clear();
            navStatStr.Clear();
        }
            
            
        if( Class == AIS_SART ) {
            if( MSG_14_text.Len() ) {
                html << rowStart << _("Safety Broadcast Message") << rowEnd
                    << rowStartH << _T("<b>") << MSG_14_text << rowEnd;
            }
        }

       //  Dimensions

        if( NavStatus != ATON_VIRTUAL && Class != AIS_ARPA && Class != AIS_APRS ) {
            if( ( Class == AIS_CLASS_B ) || ( Class == AIS_ATON ) ) {
                sizeString = wxString::Format( _T("%dm x %dm"), ( DimA + DimB ), ( DimC + DimD ) );
            } else if(!b_SarAircraftPosnReport) {
                if( ( DimA + DimB + DimC + DimD ) == 0 ) {
                    if( b_isEuroInland ) {
                        if( Euro_Length == 0.0 ) {
                            if( Euro_Draft > 0.01 ) {
                                sizeString << wxString::Format( _T("---m x ---m x %4.1fm"), Euro_Draft );
                            } else {
                                sizeString << _T("---m x ---m x ---m");
                            }
                        } else {
                            if( Euro_Draft > 0.01 ) {
                                sizeString
                                        << wxString::Format( _T("%5.1fm x %4.1fm x %4.1fm"), Euro_Length,
                                                Euro_Beam, Euro_Draft );
                            } else {
                                sizeString
                                        << wxString::Format( _T("%5.1fm x %4.1fm x ---m\n\n"), Euro_Length,
                                                Euro_Beam );
                            }
                        }
                    } else {
                        if( Draft > 0.01 ) {
                            sizeString << wxString::Format( _T("---m x ---m x %4.1fm"), Draft );
                        } else {
                            sizeString << _T("---m x ---m x ---m");
                        }
                    }
                } else if( Draft < 0.01 ) {
                    sizeString
                            << wxString::Format( _T("%dm x %dm x ---m"), ( DimA + DimB ), ( DimC + DimD ) );
                } else {
                    sizeString
                            << wxString::Format( _T("%dm x %dm x %4.1fm"), ( DimA + DimB ), ( DimC + DimD ),
                                    Draft );
                }
            }
        }
    }

    if( Class == AIS_SART ) {
        html << _T("<tr><td colspan=2>") << _T("<b>") << AISTypeStr;
        if( sart_sub_type.Length() )
            html << _T(" (") << sart_sub_type << _T("), ");
        html << navStatStr;
        html << rowEnd << _T("<tr><td colspan=2>") << _T("<b>") << sizeString << rowEnd;
    }

    else if( Class == AIS_ATON )  {
        html << _T("<tr><td colspan=2>") << _T("<b>") << navStatStr;
        html << rowEnd << _T("<tr><td colspan=2>") << _T("<b>") << sizeString << rowEnd;
    }
    
    else if( ( Class != AIS_BASE ) && ( Class != AIS_DSC ) ) {
        html << _T("<tr><td colspan=2>") << _T("<b>") << AISTypeStr;
        if( navStatStr.Length() )
            html << _T(", ") << navStatStr;
        if( UNTypeStr.Length() )
            html << _T(" (UN Type ") << UNTypeStr << _T(")");
        html << rowEnd << _T("<tr><td colspan=2>") << _T("<b>") << sizeString << rowEnd;
    }

    if( b_positionOnceValid ) {
        wxString posTypeStr;
        if( b_positionDoubtful ) posTypeStr << _(" (Last Known)");

        now.MakeGMT();
        int target_age = now.GetTicks() - PositionReportTicks;

        html << vertSpacer
             << rowStart << _("Position") << posTypeStr << _T("</font></td><td align=right><font size=-2>")
             << _("Report Age") << _T("</font></td></tr>")

             << rowStartH << _T("<b>") << toSDMM( 1, Lat ) << _T("</b></td><td align=right><b>")
             << FormatTimeAdaptive( target_age ) << rowEnd
             << rowStartH << _T("<b>") << toSDMM( 2, Lon ) << rowEnd;
    }

    wxString courseStr, sogStr, hdgStr, rotStr, rngStr, brgStr, destStr, etaStr;

    if( Class == AIS_GPSG_BUDDY ) {
        long month, year, day;
        m_date_string.Mid(0,2).ToLong(&day);
        m_date_string.Mid(2,2).ToLong(&month);
        m_date_string.Mid(4,2).ToLong(&year);
        wxDateTime date;
        date.SetDay(day);
        date.SetMonth((wxDateTime::Month)(month-1));
        date.SetYear(year + 2000);
        
        wxString f_date = date.FormatISODate();
        
        html << vertSpacer << rowStart << _("Report as of") << rowEnd
             << rowStartH << _T("<b>")
             << f_date + _T("</b> at <b>")
             << wxString::Format( _T("%d:%d UTC "), m_utc_hour, m_utc_min )
             << rowEnd;
    } else {
        if( Class == AIS_CLASS_A && !b_SarAircraftPosnReport ) {
            html << vertSpacer << rowStart << _("Destination")
                 << _T("</font></td><td align=right><font size=-2>")
                 << _("ETA (UTC)") << _T("</font></td></tr>\n")
                 << rowStartH << _T("<b>");
                 wxString dest =  trimAISField( Destination );
                 if(dest.Length() )
                     html << dest;
                 else
                     html << _("---");
                 html << _T("</b></td><td nowrap align=right><b>");

            if( ( ETA_Mo ) && ( ETA_Hr < 24 ) ) {
                int yearOffset = 0;
                if( now.GetMonth() > ( ETA_Mo - 1 ) ) yearOffset = 1;
                wxDateTime eta( ETA_Day, wxDateTime::Month( ETA_Mo - 1 ),
                        now.GetYear() + yearOffset, ETA_Hr, ETA_Min );
                html << eta.Format( _T("%b %d %H:%M") );
            }
            else html << _("---");
            html << rowEnd;
        }

        if( Class == AIS_CLASS_A || Class == AIS_CLASS_B || Class == AIS_ARPA || Class == AIS_APRS ) {
            int crs = wxRound( COG );
            if( crs < 360 ) {
                if( g_bShowMag )
                    courseStr << wxString::Format( wxString("%03d°(M)  ", wxConvUTF8 ), (int)gFrame->GetTrueOrMag( crs ) );
                else
                    courseStr << wxString::Format( wxString("%03d°  ", wxConvUTF8 ), (int)gFrame->GetTrueOrMag( crs ) );
            }   
            else if( COG == 360.0 )
                courseStr = _T("---");
            else if( crs == 360 )
                courseStr = _T("0&deg;");

            double speed_show = toUsrSpeed( SOG );
            
            if( ( SOG <= 102.2 ) || b_SarAircraftPosnReport ){
                if( speed_show < 10.0 )
                    sogStr = wxString::Format( _T("%.2f "), speed_show ) + getUsrSpeedUnit();
                else if( speed_show < 100.0 )
                    sogStr = wxString::Format( _T("%.1f "), speed_show ) + getUsrSpeedUnit();
                else
                    sogStr = wxString::Format( _T("%.0f "), speed_show ) + getUsrSpeedUnit();
            }
//                sogStr = wxString::Format( _T("%5.2f ") + getUsrSpeedUnit(), toUsrSpeed( SOG ) );
            else
                sogStr = _("---");

            if( (int) HDG != 511 )
                hdgStr = wxString::Format( _T("%03d&deg;"), (int) HDG );
            else
                hdgStr = _T("---");


            if( ROTAIS != -128 ) {
                if( ROTAIS == 127 ) rotStr << _T("> 5&deg;/30s ") << _("Right");
                else if( ROTAIS == -127 ) rotStr << _T("> 5&deg;/30s ") << _("Left");
                else {
                    if( ROTIND > 0 ) rotStr << wxString::Format( _T("%3d&deg;/Min "), ROTIND ) << _("Right");
                    else if( ROTIND < 0 ) rotStr << wxString::Format( _T("%3d&deg;/Min "), -ROTIND ) << _("Left");
                    else rotStr = _T("0");
                }
            }
            else if( !b_SarAircraftPosnReport )
                rotStr = _("---");
        }
    }

    if( b_positionOnceValid && bGPSValid && ( Range_NM >= 0. ) )
        rngStr = cc1->FormatDistanceAdaptive( Range_NM );
    else
        rngStr = _("---");

    int brg = (int) wxRound( Brg );
    if( Brg > 359.5 )
        brg = 0;
    if( b_positionOnceValid && bGPSValid && ( Brg >= 0. ) && ( Range_NM > 0. ) && ( fabs( Lat ) < 85. ) ){
        if( g_bShowMag )
            brgStr << wxString::Format( wxString("%03d°(M)  ", wxConvUTF8 ), (int)gFrame->GetTrueOrMag( Brg ) );
        else
            brgStr << wxString::Format( wxString("%03d°  ", wxConvUTF8 ), (int)gFrame->GetTrueOrMag( Brg ) );
    }   
    else
        brgStr = _("---");

    wxString turnRateHdr; // Blank if ATON or BASE or Special Position Report (9)
    if( ( Class != AIS_ATON ) && ( Class != AIS_BASE ) && ( Class != AIS_DSC ) ) {
        html << vertSpacer << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
            << rowStart <<_("Speed") << _T("</font></td><td>&nbsp;</td><td><font size=-2>")
            << _("Course") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>");
            if( !b_SarAircraftPosnReport )
                html << _("Heading") ;
            
            html << _T("</font></td></tr>")
            << rowStartH << _T("<b>") << sogStr << _T("</b></td><td>&nbsp;</td><td><b>")
            << courseStr << _T("</b></td><td>&nbsp;</td><td align=right><b>");
            if(!b_SarAircraftPosnReport)
                html << hdgStr;
            html  << rowEnd << _T("</table></td></tr>")
            << vertSpacer;
            
            if( !b_SarAircraftPosnReport )
            turnRateHdr = _("Turn Rate");
    }
    html << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
        << rowStart <<_("Range") << _T("</font></td><td>&nbsp;</td><td><font size=-2>")
        << _("Bearing") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>")
        << turnRateHdr << _T("</font></td></tr>")
        << rowStartH << _T("<b>") << rngStr << _T("</b></td><td>&nbsp;</td><td><b>")
        << brgStr << _T("</b></td><td>&nbsp;</td><td align=right><b>");
        if(!b_SarAircraftPosnReport)
            html << rotStr;
        html << rowEnd << _T("</table></td></tr>")
        << vertSpacer;

    if( bCPA_Valid ) {
        wxString tcpaStr;
        tcpaStr << _T("</b> ") << _("in ") << _T("</td><td align=right><b>") << FormatTimeAdaptive( (int)(TCPA*60.) );
                                                                  
        html<< /*vertSpacer << */rowStart << _T("<font size=-2>") <<_("CPA") << _T("</font>") << rowEnd
            << rowStartH << _T("<b>") << cc1->FormatDistanceAdaptive( CPA )
            << tcpaStr << rowEnd;
    }

    if( Class != AIS_BASE ) {
        if( blue_paddle == 1 ) {
            html << rowStart << _("Inland Blue Flag") << rowEnd
                 << rowStartH << _T("<b>") << _("Clear") << rowEnd;
        } else if( blue_paddle == 2 ) {
            html << rowStart << _("Inland Blue Flag") << rowEnd
                 << rowStartH << _T("<b>") << _("Set") << rowEnd;
        }
    }

    if(b_SarAircraftPosnReport) {
        wxString altStr;
        if(altitude != 4095) 
            altStr.Printf(_T("%4d m"), altitude );
        else
            altStr = _("Unknown");
            
        html    << rowStart <<_("Altitude") << _T("</font></td><td>&nbsp;</td><td><font size=-0>")
        << rowStartH << _T("<b>") << altStr << _T("</b></td><td>&nbsp;</td><td><b>")
        << rowEnd << _T("</table></td></tr>")
        << vertSpacer;
    }
    
    html << _T("</table>");
    return html;
}

wxString AIS_Target_Data::GetRolloverString( void )
{
    wxString result;
    wxString t;
    if( b_nameValid ) {
        result.Append( _T("\"") );
        result.Append( GetFullName() );
        result.Append( _T("\" ") );
    }
    if( Class != AIS_GPSG_BUDDY ) {
        t.Printf( _T("%09d"), abs( MMSI ) );
        result.Append( t );
        result.Append( _T(" ") );
        result.Append( GetCountryCode(false) );
    }
    t = trimAISField( CallSign );
    if( t.Len() ) {
        result.Append( _T(" (") );
        result.Append( t );
        result.Append( _T(")") );
    }
    if( g_bAISRolloverShowClass || ( Class == AIS_SART ) ) {
        if( result.Len() ) result.Append( _T("\n") );
        result.Append( _T("[") );
        if( Class == AIS_ATON ) {
            result.Append( wxGetTranslation( Get_class_string( true ) ) );
            result.Append(_T(": "));
            result.Append( wxGetTranslation( Get_vessel_type_string( false ) ) );
        }
        else if(b_SarAircraftPosnReport)
            result.Append(_("SAR Aircraft"));
        else
            result.Append( wxGetTranslation( Get_class_string( false ) ) );
        
        result.Append( _T("] ") );
        if( ( Class != AIS_ATON ) && ( Class != AIS_BASE ) ) {
            if( Class == AIS_SART ) {
                int mmsi_start = MMSI / 1000000;
                switch( mmsi_start ){
                    case 970:
                        break;
                    case 972:
                        result += _T("MOB");
                        break;
                    case 974:
                        result += _T("EPIRB");
                        break;
                    default:
                        result += _("Unknown");
                        break;
                }
            }

            if( Class != AIS_SART ) {
                if( !b_SarAircraftPosnReport )
                    result.Append( wxGetTranslation( Get_vessel_type_string( false ) ) );
            }

            if( ( Class != AIS_CLASS_B ) && ( Class != AIS_SART ) && !b_SarAircraftPosnReport) {
                if( ( NavStatus <= 15 ) && ( NavStatus >= 0 ) ) {
                    result.Append( _T(" (") );
                    result.Append(wxGetTranslation(ais_get_status(NavStatus)));
                    result.Append( _T(")") );
                }
            } else if( Class == AIS_SART ) {
                result.Append( _T(" (") );
                if( NavStatus == RESERVED_14 ) result.Append( _("Active") );
                else if( NavStatus == UNDEFINED ) result.Append( _("Testing") );

                result.Append( _T(")") );
            }

        }
    }

    if( g_bAISRolloverShowCOG && (( SOG <= 102.2 ) || b_SarAircraftPosnReport) 
            && ( ( Class != AIS_ATON ) && ( Class != AIS_BASE ) ) ) {
        if( result.Len() ) result << _T("\n");
        
        double speed_show = toUsrSpeed( SOG );
        if( speed_show < 10.0 )
            result << wxString::Format( _T("SOG %.2f "), speed_show ) << getUsrSpeedUnit() << _T(" ");
        else if( speed_show < 100.0 )
            result << wxString::Format( _T("SOG %.1f "), speed_show ) << getUsrSpeedUnit() << _T(" ");
        else
            result << wxString::Format( _T("SOG %.0f "), speed_show ) << getUsrSpeedUnit() << _T(" ");
        
        int crs = wxRound( COG );
        if( b_positionOnceValid ) {
            if( crs < 360 ) {
                if( g_bShowMag )
                    result << wxString::Format( wxString("COG %03d°(M)  ", wxConvUTF8 ), (int)gFrame->GetTrueOrMag( crs ) );
                else
                    result << wxString::Format( wxString("COG %03d°  ", wxConvUTF8 ), (int)gFrame->GetTrueOrMag( crs ) );
            }
                
            else if( COG == 360.0 )
                result << _(" COG Unavailable");
            else if( crs == 360 )
                result << wxString( " COG 000°", wxConvUTF8 );
        } else
            result << _(" COG Unavailable");
    }

    if( g_bAISRolloverShowCPA && bCPA_Valid ) {
        if( result.Len() ) result << _T("\n");
        result << _("CPA") << _T(" ") << cc1->FormatDistanceAdaptive( CPA )
        << _T(" ") << _("in") << _T(" ")
        << wxString::Format( _T("%.0f"), TCPA ) << _T(" ") << _("min");
    }
    return result;
}

wxString AIS_Target_Data::Get_vessel_type_string( bool b_short )
{
    int i = 19;
    if( Class == AIS_ATON ) {
        i = ShipType + 20;
    } else
        switch( ShipType ){
            case 30:
                i = 0;
                break;
            case 31:
                i = 1;
                break;
            case 32:
                i = 2;
                break;
            case 33:
                i = 3;
                break;
            case 34:
                i = 4;
                break;
            case 35:
                i = 5;
                break;
            case 36:
                i = 6;
                break;
            case 37:
                i = 7;
                break;
            case 50:
                i = 9;
                break;
            case 51:
                i = 10;
                break;
            case 52:
                i = 11;
                break;
            case 53:
                i = 12;
                break;
            case 54:
                i = 13;
                break;
            case 55:
                i = 14;
                break;
            case 58:
                i = 15;
                break;
            default:
                i = 19;
                break;
        }

    if( ( Class == AIS_CLASS_B ) || ( Class == AIS_CLASS_A ) ) {
        if( ( ShipType >= 40 ) && ( ShipType < 50 ) ) i = 8;

        if( ( ShipType >= 60 ) && ( ShipType < 70 ) ) i = 16;

        if( ( ShipType >= 70 ) && ( ShipType < 80 ) ) i = 17;

        if( ( ShipType >= 80 ) && ( ShipType < 90 ) ) i = 18;
    } else if( Class == AIS_GPSG_BUDDY )
        i = 52;
    else if( Class == AIS_ARPA )
        i = 55;
    else if( Class == AIS_APRS )
        i = 56;
    else if( Class == AIS_DSC )
        i = ( ShipType == 12 ) ? 54 : 53;  // 12 is distress

    if( !b_short )
        return ais_get_type(i);
    else
        return ais_get_short_type(i);
}

wxString AIS_Target_Data::Get_class_string( bool b_short )
{
    switch( Class ){
        case AIS_CLASS_A:
            return _("A");
        case AIS_CLASS_B:
            return _("B");
        case AIS_ATON:
            return b_short ? _("AtoN") : _("Aid to Navigation");
        case AIS_BASE:
            return b_short ? _("Base") : _("Base Station");
        case AIS_GPSG_BUDDY:
            return b_short ? _("Buddy") : _("GPSGate Buddy");
        case AIS_DSC:
            if( ShipType == 12 )
                return b_short ? _("DSC") : _("DSC Distress");
            else
                return b_short ? _("DSC") : _("DSC Position Report");
        case AIS_SART:
            return b_short ? _("SART") : _("SART");
        case AIS_ARPA:
            return b_short ? _("ARPA") : _("ARPA");
        case AIS_APRS:
            return b_short ? _("APRS") : _("APRS Position Report");

        default:
            return b_short ? _("Unk") : _("Unknown");
    }
}

void AIS_Target_Data::Toggle_AIS_CPA(void)
{
    b_show_AIS_CPA = !b_show_AIS_CPA ? true : false;
}

void AIS_Target_Data::ToggleShowTrack(void)
{
    b_show_track = !b_show_track ? true : false;
}


//Get country name and code according to ISO 3166-2 2014-01 (www.itu.int/online/mms/glad/cga_mids.sh?lng=E)

wxString AIS_Target_Data::GetCountryCode( bool b_CntryLongStr )  //false = Short country code, true = Full country name
{
  int mmsi_start = MMSI / 1000000;
  if (mmsi_start == 111) mmsi_start = (MMSI - 111000000)/1000 ; //SAR Aircraft start with 111 and has a MID.

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3,0,0)

  switch(mmsi_start) {
    case 201: return b_CntryLongStr ? _("Albania") : _T("AL") ;
    case 202: return b_CntryLongStr ? _("Andorra") : _T("AD") ;
    case 203: return b_CntryLongStr ? _("Austria") : _T("AT") ;
    case 204: return b_CntryLongStr ? _("Azores") : _T("AZ") ;
    case 205: return b_CntryLongStr ? _("Belgium") : _T("BE") ;
    case 206: return b_CntryLongStr ? _("Belarus") : _T("BY") ;
    case 207: return b_CntryLongStr ? _("Bulgaria") : _T("BG") ;
    case 208: return b_CntryLongStr ? _("Vatican City State") : _T("VA") ;
    case 209: return b_CntryLongStr ? _("Cyprus") : _T("CY") ;
    case 210: return b_CntryLongStr ? _("Cyprus") : _T("CY") ;
    case 211: return b_CntryLongStr ? _("Germany") : _T("DE") ;
    case 212: return b_CntryLongStr ? _("Cyprus") : _T("CY") ;
    case 213: return b_CntryLongStr ? _("Georgia") : _T("GE") ;
    case 214: return b_CntryLongStr ? _("Moldova") : _T("MD") ;
    case 215: return b_CntryLongStr ? _("Malta") : _T("MT") ;
    case 216: return b_CntryLongStr ? _("Armenia") : _T("AM") ;
    case 218: return b_CntryLongStr ? _("Germany") : _T("DE") ;
    case 219: return b_CntryLongStr ? _("Denmark") : _T("DK") ;
    case 220: return b_CntryLongStr ? _("Denmark") : _T("DK") ;
    case 224: return b_CntryLongStr ? _("Spain") : _T("ES") ;
    case 225: return b_CntryLongStr ? _("Spain") : _T("ES") ;
    case 226: return b_CntryLongStr ? _("France") : _T("FR") ;
    case 227: return b_CntryLongStr ? _("France") : _T("FR") ;
    case 228: return b_CntryLongStr ? _("France") : _T("FR") ;
    case 229: return b_CntryLongStr ? _("Malta") : _T("MT") ;
    case 230: return b_CntryLongStr ? _("Finland") : _T("FI") ;
    case 231: return b_CntryLongStr ? _("Faroe Islands") : _T("FO") ;
    case 232: 
    case 233: 
    case 234: 
    case 235: return b_CntryLongStr ? _("Great Britain") : _T("GB") ;
    case 236: return b_CntryLongStr ? _("Gibraltar") : _T("GI") ;
    case 237: return b_CntryLongStr ? _("Greece") : _T("GR") ;
    case 238: return b_CntryLongStr ? _("Croatia") : _T("HR") ;
    case 239: return b_CntryLongStr ? _("Greece") : _T("GR") ;
    case 240: return b_CntryLongStr ? _("Greece") : _T("GR") ;
    case 241: return b_CntryLongStr ? _("Greece") : _T("GR") ;
    case 242: return b_CntryLongStr ? _("Morocco") : _T("MA") ;
    case 243: return b_CntryLongStr ? _("Hungary") : _T("HU") ;
    case 244: return b_CntryLongStr ? _("Netherlands") : _T("NL") ;
    case 245: return b_CntryLongStr ? _("Netherlands") : _T("NL") ;
    case 246: return b_CntryLongStr ? _("Netherlands") : _T("NL") ;
    case 247: return b_CntryLongStr ? _("Italy") : _T("IT") ;
    case 248: return b_CntryLongStr ? _("Malta") : _T("MT") ;
    case 249: return b_CntryLongStr ? _("Malta") : _T("MT") ;
    case 250: return b_CntryLongStr ? _("Ireland") : _T("IE") ;
    case 251: return b_CntryLongStr ? _("Iceland") : _T("IS") ;
    case 252: return b_CntryLongStr ? _("Liechtenstein") : _T("LI") ;
    case 253: return b_CntryLongStr ? _("Luxembourg") : _T("LU") ;
    case 254: return b_CntryLongStr ? _("Monaco") : _T("MC") ;
    case 255: return b_CntryLongStr ? _("Madeira") : _T("PT") ;
    case 256: return b_CntryLongStr ? _("Malta") : _T("MT") ;
    case 257: 
    case 258: 
    case 259: return b_CntryLongStr ? _("Norway") : _T("NO") ;
    case 261: return b_CntryLongStr ? _("Poland") : _T("PL") ;
    case 262: return b_CntryLongStr ? _("Montenegro") : _T("ME") ;
    case 263: return b_CntryLongStr ? _("Portugal") : _T("PT") ;
    case 264: return b_CntryLongStr ? _("Romania") : _T("RO") ;
    case 265: return b_CntryLongStr ? _("Sweden") : _T("SE") ;
    case 266: return b_CntryLongStr ? _("Sweden") : _T("SE") ;
    case 267: return b_CntryLongStr ? _("Slovak Republic") : _T("SK") ;
    case 268: return b_CntryLongStr ? _("San Marino") : _T("SM") ;
    case 269: return b_CntryLongStr ? _("Switzerland") : _T("CH") ;
    case 270: return b_CntryLongStr ? _("Czech Republic") : _T("CZ") ;
    case 271: return b_CntryLongStr ? _("Turkey") : _T("TR") ;
    case 272: return b_CntryLongStr ? _("Ukraine") : _T("UA") ;
    case 273: return b_CntryLongStr ? _("Russian") : _T("RU") ;
    case 274: return b_CntryLongStr ? _("Macedonia") : _T("MK") ;
    case 275: return b_CntryLongStr ? _("Latvia") : _T("LV") ;
    case 276: return b_CntryLongStr ? _("Estonia") : _T("EE") ;
    case 277: return b_CntryLongStr ? _("Lithuania") : _T("LT") ;
    case 278: return b_CntryLongStr ? _("Slovenia") : _T("SI") ;
    case 279: return b_CntryLongStr ? _("Serbia") : _T("RS") ;
    case 301: return b_CntryLongStr ? _("Anguilla") : _T("AI") ;
    case 303: return b_CntryLongStr ? _("Alaska") : _T("AK") ;
    case 304: return b_CntryLongStr ? _("Antigua and Barbuda") : _T("AG") ;
    case 305: return b_CntryLongStr ? _("Antigua and Barbuda") : _T("AG") ;
    case 306: return b_CntryLongStr ? _("Antilles") : _T("AN") ;
    case 307: return b_CntryLongStr ? _("Aruba") : _T("AW") ;
    case 308: return b_CntryLongStr ? _("Bahamas") : _T("BS") ;
    case 309: return b_CntryLongStr ? _("Bahamas") : _T("BS") ;
    case 310: return b_CntryLongStr ? _("Bermuda") : _T("BM") ;
    case 311: return b_CntryLongStr ? _("Bahamas") : _T("BS") ;
    case 312: return b_CntryLongStr ? _("Belize") : _T("BZ") ;
    case 314: return b_CntryLongStr ? _("Barbados") : _T("BB") ;
    case 316: return b_CntryLongStr ? _("Canada") : _T("CA") ;
    case 319: return b_CntryLongStr ? _("Cayman Islands") : _T("KY") ;
    case 321: return b_CntryLongStr ? _("Costa Rica") : _T("CR") ;
    case 323: return b_CntryLongStr ? _("Cuba") : _T("CU") ;
    case 325: return b_CntryLongStr ? _("Dominica") : _T("DM") ;
    case 327: return b_CntryLongStr ? _("Dominican Republic") : _T("DM") ;
    case 329: return b_CntryLongStr ? _("Guadeloupe") : _T("GP") ;
    case 330: return b_CntryLongStr ? _("Grenada") : _T("GD") ;
    case 331: return b_CntryLongStr ? _("Greenland") : _T("GL") ;
    case 332: return b_CntryLongStr ? _("Guatemala") : _T("GT") ;
    case 334: return b_CntryLongStr ? _("Honduras") : _T("HN") ;
    case 336: return b_CntryLongStr ? _("Haiti") : _T("HT") ;
    case 338: return b_CntryLongStr ? _("United States of America") : _T("US") ;
    case 339: return b_CntryLongStr ? _("Jamaica") : _T("JM") ;
    case 341: return b_CntryLongStr ? _("Saint Kitts and Nevis") : _T("KN") ;
    case 343: return b_CntryLongStr ? _("Saint Lucia") : _T("LC") ;
    case 345: return b_CntryLongStr ? _("Mexico") : _T("MX") ;
    case 347: return b_CntryLongStr ? _("Martinique") : _T("MQ") ;
    case 348: return b_CntryLongStr ? _("Montserrat") : _T("MS") ;
    case 350: return b_CntryLongStr ? _("Nicaragua") : _T("NI") ;
    case 351: 
    case 352: 
    case 353: 
    case 354: 
    case 355: 
    case 356: 
    case 357: return b_CntryLongStr ? _("Panama") : _T("PA") ;
    case 358: return b_CntryLongStr ? _("Puerto Rico") : _T("PR") ;
    case 359: return b_CntryLongStr ? _("El Salvador") : _T("SV") ;
    case 361: return b_CntryLongStr ? _("Saint Pierre and Miquelon") : _T("PM") ;
    case 362: return b_CntryLongStr ? _("Trinidad and Tobago") : _T("TT") ;
    case 364: return b_CntryLongStr ? _("Turks and Caicos Islands") : _T("TC") ;
    case 366: 
    case 367: 
    case 368: 
    case 369: return b_CntryLongStr ? _("United States of America") : _T("US") ;
    case 370: 
    case 371: 
    case 372:
    case 373:
    case 374: return b_CntryLongStr ? _("Panama") : _T("PA") ;
    case 375: 
    case 376: 
    case 377: return b_CntryLongStr ? _("Saint Vincent and the Grenadines") : _T("VC") ;
    case 378: return b_CntryLongStr ? _("British Virgin Islands") : _T("VG") ;
    case 379: return b_CntryLongStr ? _("United States Virgin Islands") : _T("AE") ;
    case 401: return b_CntryLongStr ? _("Afghanistan") : _T("AF") ;
    case 403: return b_CntryLongStr ? _("Saudi Arabia") : _T("SA") ;
    case 405: return b_CntryLongStr ? _("Bangladesh") : _T("BD") ;
    case 408: return b_CntryLongStr ? _("Bahrain") : _T("BH") ;
    case 410: return b_CntryLongStr ? _("Bhutan") : _T("BT") ;
    case 412: 
    case 413: 
    case 414: return b_CntryLongStr ? _("China") : _T("CN") ;
    case 416: return b_CntryLongStr ? _("Taiwan") : _T("TW") ;
    case 417: return b_CntryLongStr ? _("Sri Lanka") : _T("LK") ;
    case 419: return b_CntryLongStr ? _("India") : _T("IN") ;
    case 422: return b_CntryLongStr ? _("Iran") : _T("IR") ;
    case 423: return b_CntryLongStr ? _("Azerbaijani Republic") : _T("AZ") ;
    case 425: return b_CntryLongStr ? _("Iraq") : _T("IQ") ;
    case 428: return b_CntryLongStr ? _("Israel") : _T("IL") ;
    case 431: return b_CntryLongStr ? _("Japan") : _T("JP") ;
    case 432: return b_CntryLongStr ? _("Japan") : _T("JP") ;
    case 434: return b_CntryLongStr ? _("Turkmenistan") : _T("TM") ;
    case 436: return b_CntryLongStr ? _("Kazakhstan") : _T("KZ") ;
    case 437: return b_CntryLongStr ? _("Uzbekistan") : _T("UZ") ;
    case 438: return b_CntryLongStr ? _("Jordan") : _T("JO") ;
    case 440: return b_CntryLongStr ? _("Korea") : _T("KR") ;
    case 441: return b_CntryLongStr ? _("Korea") : _T("KR") ;
    case 443: return b_CntryLongStr ? _("Palestine") : _T("PS") ;
    case 445: return b_CntryLongStr ? _("People's Rep. of Korea") : _T("KP") ;
    case 447: return b_CntryLongStr ? _("Kuwait") : _T("KW") ;
    case 450: return b_CntryLongStr ? _("Lebanon") : _T("LB") ;
    case 451: return b_CntryLongStr ? _("Kyrgyz Republic") : _T("KG") ;
    case 453: return b_CntryLongStr ? _("Macao") : _T("MO") ;
    case 455: return b_CntryLongStr ? _("Maldives") : _T("MV") ;
    case 457: return b_CntryLongStr ? _("Mongolia") : _T("MN") ;
    case 459: return b_CntryLongStr ? _("Nepal") : _T("NP") ;
    case 461: return b_CntryLongStr ? _("Oman") : _T("OM") ;
    case 463: return b_CntryLongStr ? _("Pakistan") : _T("PK") ;
    case 466: return b_CntryLongStr ? _("Qatar") : _T("QA") ;
    case 468: return b_CntryLongStr ? _("Syrian Arab Republic") : _T("SY") ;
    case 470: return b_CntryLongStr ? _("United Arab Emirates") : _T("AE") ;
    case 472: return b_CntryLongStr ? _("Tajikistan") : _T("TJ") ;
    case 473: return b_CntryLongStr ? _("Yemen") : _T("YE") ;
    case 475: return b_CntryLongStr ? _("Yemen") : _T("YE") ;
    case 477: return b_CntryLongStr ? _("Hong Kong") : _T("HK") ;
    case 478: return b_CntryLongStr ? _("Bosnia and Herzegovina") : _T("BA") ;
    case 501: return b_CntryLongStr ? _("Adelie Land") : _T("TF") ;
    case 503: return b_CntryLongStr ? _("Australia") : _T("AU") ;
    case 506: return b_CntryLongStr ? _("Myanmar") : _T("MM") ;
    case 508: return b_CntryLongStr ? _("Brunei Darussalam") : _T("BN") ;
    case 510: return b_CntryLongStr ? _("Micronesia") : _T("FM") ;
    case 511: return b_CntryLongStr ? _("Palau") : _T("PW") ;
    case 512: return b_CntryLongStr ? _("New Zealand") : _T("NZ") ;
    case 514: return b_CntryLongStr ? _("Cambodia") : _T("KH") ;
    case 515: return b_CntryLongStr ? _("Cambodia") : _T("KH") ;
    case 516: return b_CntryLongStr ? _("Christmas Island") : _T("CX") ;
    case 518: return b_CntryLongStr ? _("Cook Islands") : _T("CK") ;
    case 520: return b_CntryLongStr ? _("Fiji") : _T("FJ") ;
    case 523: return b_CntryLongStr ? _("Cocos") : _T("CC") ;
    case 525: return b_CntryLongStr ? _("Indonesia") : _T("ID") ;
    case 529: return b_CntryLongStr ? _("Kiribati") : _T("KI") ;
    case 531: return b_CntryLongStr ? _("Lao People's Dem. Rep.") : _T("LA") ;
    case 533: return b_CntryLongStr ? _("Malaysia") : _T("MY") ;
    case 536: return b_CntryLongStr ? _("Northern Mariana Islands") : _T("MP") ;
    case 538: return b_CntryLongStr ? _("Marshall Islands") : _T("MH") ;
    case 540: return b_CntryLongStr ? _("New Caledonia") : _T("NC") ;
    case 542: return b_CntryLongStr ? _("Niue") : _T("NU") ;
    case 544: return b_CntryLongStr ? _("Nauru") : _T("NR") ;
    case 546: return b_CntryLongStr ? _("French Polynesia") : _T("PF") ;
    case 548: return b_CntryLongStr ? _("Philippines") : _T("PH") ;
    case 553: return b_CntryLongStr ? _("Papua New Guinea") : _T("PG") ;
    case 555: return b_CntryLongStr ? _("Pitcairn Island") : _T("PN") ;
    case 557: return b_CntryLongStr ? _("Solomon Islands") : _T("SB") ;
    case 559: return b_CntryLongStr ? _("American Samoa") : _T("AS") ;
    case 561: return b_CntryLongStr ? _("Samoa") : _T("WS") ;
    case 563: 
    case 564: 
    case 565: 
    case 566: return b_CntryLongStr ? _("Singapore") : _T("SG") ;
    case 567: return b_CntryLongStr ? _("Thailand") : _T("TH") ;
    case 570: return b_CntryLongStr ? _("Tonga") : _T("TO") ;
    case 572: return b_CntryLongStr ? _("Tuvalu") : _T("TV") ;
    case 574: return b_CntryLongStr ? _("Viet Nam") : _T("VN") ;
    case 576: return b_CntryLongStr ? _("Vanuatu") : _T("VU") ;
    case 577: return b_CntryLongStr ? _("Vanuatu") : _T("VU") ;
    case 578: return b_CntryLongStr ? _("Wallis and Futuna Islands") : _T("WF") ;
    case 601: return b_CntryLongStr ? _("South Africa") : _T("ZA") ;
    case 603: return b_CntryLongStr ? _("Angola") : _T("AO") ;
    case 605: return b_CntryLongStr ? _("Algeria") : _T("DZ") ;
    case 607: return b_CntryLongStr ? _("Saint Paul") : _T("TF") ;
    case 608: return b_CntryLongStr ? _("Ascension Island") : _T("SH") ;
    case 609: return b_CntryLongStr ? _("Burundi") : _T("BI") ;
    case 610: return b_CntryLongStr ? _("Benin") : _T("BJ") ;
    case 611: return b_CntryLongStr ? _("Botswana") : _T("BW") ;
    case 612: return b_CntryLongStr ? _("Central African Republic") : _T("CF") ;
    case 613: return b_CntryLongStr ? _("Cameroon") : _T("CM") ;
    case 615: return b_CntryLongStr ? _("Congo") : _T("CD") ;
    case 616: return b_CntryLongStr ? _("Comoros") : _T("KM") ;
    case 617: return b_CntryLongStr ? _("Cape Verde") : _T("CV") ;
    case 618: return b_CntryLongStr ? _("Crozet Archipelago") : _T("TF") ;
    case 619: return b_CntryLongStr ? _("Ivory Coast") : _T("CI") ;
    case 620: return b_CntryLongStr ? _("Comoros (Union of the)") : _T("KM") ;
    case 621: return b_CntryLongStr ? _("Djibouti") : _T("DJ") ;
    case 622: return b_CntryLongStr ? _("Egypt") : _T("EG") ;
    case 624: return b_CntryLongStr ? _("Ethiopia") : _T("ET") ;
    case 625: return b_CntryLongStr ? _("Eritrea") : _T("ER") ;
    case 626: return b_CntryLongStr ? _("Gabonese Republic") : _T("GA") ;
    case 627: return b_CntryLongStr ? _("Ghana") : _T("GH") ;
    case 629: return b_CntryLongStr ? _("Gambia") : _T("GM") ;
    case 630: return b_CntryLongStr ? _("Guinea-Bissau") : _T("GW") ;
    case 631: return b_CntryLongStr ? _("Equatorial Guinea") : _T("GQ") ;
    case 632: return b_CntryLongStr ? _("Guinea") : _T("GN") ;
    case 633: return b_CntryLongStr ? _("Burkina Faso") : _T("BF") ;
    case 634: return b_CntryLongStr ? _("Kenya") : _T("KE") ;
    case 635: return b_CntryLongStr ? _("Kerguelen Islands") : _T("TF") ;
    case 636: return b_CntryLongStr ? _("Liberia") : _T("LR") ;
    case 637: return b_CntryLongStr ? _("Liberia") : _T("LR") ;
    case 638: return b_CntryLongStr ? _("South Sudan (Republic of)") : _T("SS") ;
    case 642: return b_CntryLongStr ? _("Libya") : _T("LY") ;
    case 644: return b_CntryLongStr ? _("Lesotho") : _T("LS") ;
    case 645: return b_CntryLongStr ? _("Mauritius") : _T("MU") ;
    case 647: return b_CntryLongStr ? _("Madagascar") : _T("MG") ;
    case 649: return b_CntryLongStr ? _("Mali") : _T("ML") ;
    case 650: return b_CntryLongStr ? _("Mozambique") : _T("MZ") ;
    case 654: return b_CntryLongStr ? _("Mauritania") : _T("MR") ;
    case 655: return b_CntryLongStr ? _("Malawi") : _T("MW") ;
    case 656: return b_CntryLongStr ? _("Niger") : _T("NE") ;
    case 657: return b_CntryLongStr ? _("Nigeria") : _T("NG") ;
    case 659: return b_CntryLongStr ? _("Namibia") : _T("NA") ;
    case 660: return b_CntryLongStr ? _("Reunion") : _T("RE") ;
    case 661: return b_CntryLongStr ? _("Rwanda") : _T("RW") ;
    case 662: return b_CntryLongStr ? _("Sudan") : _T("SD") ;
    case 663: return b_CntryLongStr ? _("Senegal") : _T("SN") ;
    case 664: return b_CntryLongStr ? _("Seychelles") : _T("SC") ;
    case 665: return b_CntryLongStr ? _("Saint Helena") : _T("SH") ;
    case 666: return b_CntryLongStr ? _("Somali Democratic Republic") : _T("SO") ;
    case 667: return b_CntryLongStr ? _("Sierra Leone") : _T("SL") ;
    case 668: return b_CntryLongStr ? _("Sao Tome and Principe") : _T("ST") ;
    case 669: return b_CntryLongStr ? _("Swaziland") : _T("SZ") ;
    case 670: return b_CntryLongStr ? _("Chad") : _T("TD") ;
    case 671: return b_CntryLongStr ? _("Togolese Republic") : _T("TG") ;
    case 672: return b_CntryLongStr ? _("Tunisia") : _T("TN") ;
    case 674: return b_CntryLongStr ? _("Tanzania") : _T("TZ") ;
    case 675: return b_CntryLongStr ? _("Uganda") : _T("UG") ;
    case 676: return b_CntryLongStr ? _("Dem Rep.of the Congo") : _T("CD") ;
    case 677: return b_CntryLongStr ? _("Tanzania") : _T("TZ") ;
    case 678: return b_CntryLongStr ? _("Zambia") : _T("ZM") ;
    case 679: return b_CntryLongStr ? _("Zimbabwe") : _T("ZW") ;
    case 701: return b_CntryLongStr ? _("Argentine Republic") : _T("AR") ;
    case 710: return b_CntryLongStr ? _("Brazil") : _T("BR") ;
    case 720: return b_CntryLongStr ? _("Bolivia") : _T("BO") ;
    case 725: return b_CntryLongStr ? _("Chile") : _T("CL") ;
    case 730: return b_CntryLongStr ? _("Colombia") : _T("CO") ;
    case 735: return b_CntryLongStr ? _("Ecuador") : _T("EC") ;
    case 740: return b_CntryLongStr ? _("Falkland Islands") : _T("FK") ;
    case 745: return b_CntryLongStr ? _("Guiana") : _T("GY") ;
    case 750: return b_CntryLongStr ? _("Guyana") : _T("GY") ;
    case 755: return b_CntryLongStr ? _("Paraguay") : _T("PY") ;
    case 760: return b_CntryLongStr ? _("Peru") : _T("PE") ;
    case 765: return b_CntryLongStr ? _("Suriname") : _T("SR") ;
    case 770: return b_CntryLongStr ? _("Uruguay") : _T("UY") ;
    case 775: return b_CntryLongStr ? _("Venezuela") : _T("VE") ;

    default: return wxEmptyString;
  }
#else
  return wxEmptyString;
#endif    
  
}
