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


//    Define and declare a hasmap for ERI Ship type strings, keyed by their UN Codes.
WX_DECLARE_HASH_MAP(int, wxString, wxIntegerHash, wxIntegerEqual, ERIShipTypeHash);

static ERIShipTypeHash s_ERI_hash;

void make_hash_ERI(int key, const wxString & description)
{
	s_ERI_hash[key] = description;
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
    n_alarm_state = AIS_NO_ALARM;
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

    b_OwnShip = false;
    b_in_ack_timeout = false;

    m_ptrack = new AISTargetTrackList;
    b_active = false;
    blue_paddle = 0;
    bCPA_Valid = false;
    ROTIND = 0;
    b_show_track = true;
}

AIS_Target_Data::~AIS_Target_Data()
{
      delete m_ptrack;
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
    if( ( Class != AIS_BASE ) && ( Class != AIS_SART ) ) {
        if( b_nameValid ) {
            wxString shipName = trimAISField( ShipName );
            wxString intlName;
            if( shipName == _T("Unknown") ) intlName = wxGetTranslation( shipName );
            else
                intlName = shipName;
            html << _T("<font size=+2><i><b>") << intlName ;
            if( strlen( ShipNameExtension ) ) html << wxString( ShipNameExtension, wxConvUTF8 );
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

    if( IMOstr.Length() )
        html << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
            << rowStart <<_("MMSI") << _T("</font></td><td>&nbsp;</td><td><font size=-2>")
            << _("Class") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>")
            << _("IMO") << _T("</font></td></tr>")
            << rowStartH << _T("<b>") << MMSIstr << _T("</b></td><td>&nbsp;</td><td><b>")
            << ClassStr << _T("</b></td><td>&nbsp;</td><td align=right><b>")
            << IMOstr << rowEnd << _T("</table></td></tr>")
            << vertSpacer;
    else
        html << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
            << rowStart <<_("MMSI") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>")
            << _("Class") << _T("</font></td></tr>")
            << rowStartH << _T("<b>") << MMSIstr << _T("</b></td><td>&nbsp;</td><td align=right><b>")
            << ClassStr << rowEnd << _T("</table></td></tr>")
            << vertSpacer;

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
            } else {
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

    else if( ( Class != AIS_ATON ) && ( Class != AIS_BASE ) && ( Class != AIS_DSC ) ) {
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
        html << vertSpacer << rowStart << _("Report as of") << rowEnd
             << rowStartH << wxString::Format( _T("<b>%d:%d UTC "), m_utc_hour, m_utc_min )
             << rowEnd;
    } else {
        if( Class == AIS_CLASS_A ) {
            html << vertSpacer << rowStart << _("Destination")
                 << _T("</font></td><td align=right><font size=-2>")
                 << _("ETA") << _T("</font></td></tr>\n")
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

            if( SOG <= 102.2 )
                sogStr = wxString::Format( _T("%5.2f ") + getUsrSpeedUnit(), toUsrSpeed( SOG ) );
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
            else
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

    wxString turnRateHdr; // Blank if ATON or BASE
    if( ( Class != AIS_ATON ) && ( Class != AIS_BASE ) && ( Class != AIS_DSC ) ) {
        html << vertSpacer << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
            << rowStart <<_("Speed") << _T("</font></td><td>&nbsp;</td><td><font size=-2>")
            << _("Course") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>")
            << _("Heading") << _T("</font></td></tr>")
            << rowStartH << _T("<b>") << sogStr << _T("</b></td><td>&nbsp;</td><td><b>")
            << courseStr << _T("</b></td><td>&nbsp;</td><td align=right><b>")
            << hdgStr << rowEnd << _T("</table></td></tr>")
            << vertSpacer;
        turnRateHdr = _("Turn Rate");
    }
    html << _T("<tr><td colspan=2><table width=100% border=0 cellpadding=0 cellspacing=0>")
        << rowStart <<_("Range") << _T("</font></td><td>&nbsp;</td><td><font size=-2>")
        << _("Bearing") << _T("</font></td><td>&nbsp;</td><td align=right><font size=-2>")
        << turnRateHdr << _T("</font></td></tr>")
        << rowStartH << _T("<b>") << rngStr << _T("</b></td><td>&nbsp;</td><td><b>")
        << brgStr << _T("</b></td><td>&nbsp;</td><td align=right><b>")
        << rotStr << rowEnd << _T("</table></td></tr>")
        << vertSpacer;

    wxString tcpaStr;
    if( bCPA_Valid ) tcpaStr << _(" </b>in<b> ") << FormatTimeAdaptive( (int)(TCPA*60.) );

    if( bCPA_Valid ) {
        html<< vertSpacer << rowStart << _("CPA") << rowEnd
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

    html << _T("</table>");
    return html;
}

wxString AIS_Target_Data::GetRolloverString( void )
{
    wxString result;
    wxString t;
    if( b_nameValid ) {
        result.Append( _T("\"") );
        wxString uret = trimAISField( ShipName );
        wxString ret;
        if( uret == _T("Unknown") ) ret = wxGetTranslation( uret );
        else
            ret = uret;

        result.Append( ret );
        if( strlen( ShipNameExtension ) ) result.Append(
                wxString( ShipNameExtension, wxConvUTF8 ) );

        result.Append( _T("\" ") );
    }
    if( Class != AIS_GPSG_BUDDY ) {
        t.Printf( _T("%09d"), abs( MMSI ) );
        result.Append( t );
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

            if( Class != AIS_SART ) result.Append(
                    wxGetTranslation( Get_vessel_type_string( false ) ) );

            if( ( Class != AIS_CLASS_B ) && ( Class != AIS_SART ) ) {
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

    if( g_bAISRolloverShowCOG && ( SOG <= 102.2 )
            && ( ( Class != AIS_ATON ) && ( Class != AIS_BASE ) ) ) {
        if( result.Len() ) result << _T("\n");
        
        double speed_show = toUsrSpeed( SOG );
        if( speed_show < 10.0 )
            result << wxString::Format( _T("SOG %.2f "), speed_show ) << getUsrSpeedUnit() << _T(" ");
        else
            result << wxString::Format( _T("SOG %.1f "), speed_show ) << getUsrSpeedUnit() << _T(" ");

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
