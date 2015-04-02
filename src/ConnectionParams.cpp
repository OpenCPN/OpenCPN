/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#include <wx/tokenzr.h>
#include <wx/intl.h>

#include "ConnectionParams.h"

#if !wxUSE_XLOCALE && wxCHECK_VERSION(3,0,0)
#define wxAtoi(arg) atoi(arg)
#endif


ConnectionParams::ConnectionParams(const wxString &configStr )
{
    Deserialize( configStr );
}

void ConnectionParams::Deserialize(const wxString &configStr)
{
    Valid = true;
    wxArrayString prms = wxStringTokenize( configStr, _T(";") );
    if (prms.Count() < 17) {
        Valid = false;
        return;
    }

    Type = (ConnectionType)wxAtoi(prms[0]);
    NetProtocol = (NetworkProtocol)wxAtoi(prms[1]);
    NetworkAddress = prms[2];
    NetworkPort = (ConnectionType)wxAtoi(prms[3]);
    Protocol = (DataProtocol)wxAtoi(prms[4]);
    Port = prms[5];
    Baudrate = wxAtoi(prms[6]);
    ChecksumCheck = !!wxAtoi(prms[7]);
    int iotval = wxAtoi(prms[8]);
    IOSelect=((iotval <= 2)?static_cast <dsPortType>(iotval):DS_TYPE_INPUT);
    InputSentenceListType = (ListType)wxAtoi(prms[9]);
    InputSentenceList = wxStringTokenize(prms[10], _T(","));
    OutputSentenceListType = (ListType)wxAtoi(prms[11]);
    OutputSentenceList = wxStringTokenize(prms[12], _T(","));
    Priority = wxAtoi(prms[13]);
    Garmin = !!wxAtoi(prms[14]);
    GarminUpload = !!wxAtoi(prms[15]);
    FurunoGP3X = !!wxAtoi(prms[16]);

    bEnabled = true;
    if (prms.Count() >= 18)
        bEnabled = !!wxAtoi(prms[17]);
}

wxString ConnectionParams::Serialize()
{
    wxString istcs;
    for( size_t i = 0; i < InputSentenceList.Count(); i++ )
    {
        if (i > 0)
            istcs.Append( _T(",") );
        istcs.Append( InputSentenceList[i] );
    }
    wxString ostcs;
    for( size_t i = 0; i < OutputSentenceList.Count(); i++ )
    {
        if (i > 0)
            ostcs.Append( _T(",") );
        ostcs.Append( OutputSentenceList[i] );
    }
    wxString ret = wxString::Format( _T("%d;%d;%s;%d;%d;%s;%d;%d;%d;%d;%s;%d;%s;%d;%d;%d;%d;%d"),
                                     Type,
                                     NetProtocol,
                                     NetworkAddress.c_str(),
                                     NetworkPort,
                                     Protocol,
                                     Port.c_str(),
                                     Baudrate,
                                     ChecksumCheck,
                                     IOSelect,
                                     InputSentenceListType,
                                     istcs.c_str(),
                                     OutputSentenceListType,
                                     ostcs.c_str(),
                                     Priority,
                                     Garmin,
                                     GarminUpload,
                                     FurunoGP3X,
                                     bEnabled
                                   );

    return ret;
}

ConnectionParams::ConnectionParams()
{
    Type = SERIAL;
    NetProtocol = TCP;
    NetworkAddress = wxEmptyString;
    NetworkPort = 0;
    Protocol = PROTO_NMEA0183;
    Port = wxEmptyString;
    Baudrate = 4800;
    ChecksumCheck = true;
    Garmin = false;
    FurunoGP3X = false;
    IOSelect = DS_TYPE_INPUT;
    InputSentenceListType = WHITELIST;
    OutputSentenceListType = WHITELIST;
    Priority = 0;
    Valid = true;
    bEnabled = true;
}

wxString ConnectionParams::GetSourceTypeStr()
{
    if ( Type == SERIAL )
        return _("Serial");
    else if ( Type == NETWORK )
        return _("Net");
    else if ( Type == INTERNAL_GPS )
        return _("GPS");
    else if ( Type == INTERNAL_BT )
        return _("BT");
    else
        return _T("");
}

wxString ConnectionParams::GetAddressStr()
{
    if ( Type == SERIAL )
        return wxString::Format( _T("%s"), Port.c_str() );
    else if ( Type == NETWORK )
        return wxString::Format( _T("%s:%d"), NetworkAddress.c_str(), NetworkPort );
    else if ( Type == INTERNAL_GPS )
        return _("Internal");
    else if ( Type == INTERNAL_BT )
        return NetworkAddress;
    else
        return _T("");
}

wxString ConnectionParams::GetParametersStr()
{
    if ( Type == SERIAL )
        return wxString::Format( _T("%d"), Baudrate );
    else if ( Type == NETWORK ){
        if ( NetProtocol == TCP )
            return _("TCP");
        else if (NetProtocol == UDP)
            return _("UDP");
        else
            return _("GPSD");
    }
    else if ( Type == INTERNAL_GPS )
        return _T("");
    else if ( Type == INTERNAL_BT )
        return Port;
    else
        return _T("");
}

wxString ConnectionParams::GetIOTypeValueStr()
{
    if ( IOSelect == DS_TYPE_INPUT )
        return _("Input");
    else if ( IOSelect == DS_TYPE_OUTPUT )
        return _("Output");
    else
        return _("In/Out");
}

wxString ConnectionParams::FilterTypeToStr(ListType type, FilterDirection dir)
{
    if(dir == FILTER_INPUT) {
        if ( type == BLACKLIST )
            return _("Reject");
        else
            return _("Accept");
    }
    else {
        if ( type == BLACKLIST )
            return _("Drop");
        else
            return _("Send");
    }
}

wxString ConnectionParams::GetFiltersStr()
{
    wxString istcs;
    for( size_t i = 0; i < InputSentenceList.Count(); i++ )
    {
        if ( i > 0 )
            istcs.Append( _T(",") );
        istcs.Append( InputSentenceList[i] );
    }
    wxString ostcs;
    for( size_t i = 0; i < OutputSentenceList.Count(); i++ )
    {
        if ( i > 0 )
            ostcs.Append( _T(",") );
        ostcs.Append( OutputSentenceList[i] );
    }
    wxString ret = wxEmptyString;
    if ( istcs.Len() > 0 ){
        ret.Append( _("In") );
        ret.Append(wxString::Format( _T(": %s %s"),
                                     FilterTypeToStr(InputSentenceListType, FILTER_INPUT).c_str(), istcs.c_str()) );
    }
    else
        ret.Append( _("In: None") );

    if ( ostcs.Len() > 0 ){
        ret.Append(  _T(", ") );
        ret.Append(  _("Out") );
        ret.Append( wxString::Format( _T(": %s %s"),
                                      FilterTypeToStr(OutputSentenceListType, FILTER_OUTPUT).c_str(), ostcs.c_str() ) );
    }
    else
        ret.Append( _(", Out: None") );
    return  ret;
}

wxString ConnectionParams::GetDSPort()
{
    if ( Type == SERIAL )
        return wxString::Format( _T("Serial:%s"), Port.c_str() );
    else if( Type == NETWORK){
        wxString proto;
        if ( NetProtocol == TCP )
            proto = _T("TCP");
        else if (NetProtocol == UDP)
            proto = _T("UDP");
        else
            proto = _T("GPSD");
        return wxString::Format( _T("%s:%s:%d"), proto.c_str(), NetworkAddress.c_str(), NetworkPort );
    }
    else if( Type == INTERNAL_BT ){
        return Port;
    }
    else
        return _T("");
    
}

wxString ConnectionParams::GetLastDSPort()
{
    if ( Type == SERIAL )
        return wxString::Format( _T("Serial:%s"), Port.c_str() );
    else
    {
        wxString proto;
        if ( NetProtocol == TCP )
            proto = _T("TCP");
        else if (NetProtocol == UDP)
            proto = _T("UDP");
        else
            proto = _T("GPSD");
        return wxString::Format( _T("%s:%s:%d"), proto.c_str(), LastNetworkAddress.c_str(), LastNetworkPort );
    }
}
