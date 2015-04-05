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

#ifndef __DSPORTTYPE_H__
#include <dsPortType.h>
#endif

#ifndef __CONNECTIONPARAMS_H__
#define __CONNECTIONPARAMS_H__

#include <wx/string.h>
#include <wx/dynarray.h>
#include <wx/arrstr.h>

typedef enum
{
    SERIAL = 0,
    NETWORK = 1,
    INTERNAL_GPS = 2,
    INTERNAL_BT = 3
} ConnectionType;

typedef enum
{
    TCP = 0,
    UDP = 1,
    GPSD = 2,
    PROTO_UNDEFINED = 3
} NetworkProtocol;

typedef enum
{
    WHITELIST = 0,
    BLACKLIST = 1
} ListType;

typedef enum
{
    FILTER_INPUT = 0,
    FILTER_OUTPUT = 1
} FilterDirection;

typedef enum
{
    PROTO_NMEA0183 = 0,
    PROTO_SEATALK = 1,
    PROTO_NMEA2000 = 2
} DataProtocol;

class ConnectionParams
{
public:
    ConnectionParams();
    ConnectionParams(const wxString &configStr);

    ConnectionType  Type;
    NetworkProtocol NetProtocol;
    wxString        NetworkAddress;
    int             NetworkPort;
    wxString        LastNetworkAddress;
    int             LastNetworkPort;
    
    DataProtocol    Protocol;
    wxString        Port;
    int             Baudrate;
    bool            ChecksumCheck;
    bool            Garmin;
    bool            GarminUpload;
    bool            FurunoGP3X;
    dsPortType      IOSelect;
    ListType        InputSentenceListType;
    wxArrayString   InputSentenceList;
    ListType        OutputSentenceListType;
    wxArrayString   OutputSentenceList;
    int             Priority;
    bool            bEnabled;

    wxString        Serialize();
    void            Deserialize(const wxString &configStr);

    wxString GetSourceTypeStr();
    wxString GetAddressStr();
    wxString GetParametersStr();
    wxString GetIOTypeValueStr();
    wxString GetFiltersStr();
    wxString GetDSPort();
    wxString GetLastDSPort();
    wxString GetPortStr(){ return Port; }
    void SetPortStr( wxString str ){ Port = str; }
    
    
    bool            Valid;
    bool            b_IsSetup;
private:
    wxString FilterTypeToStr(ListType type, FilterDirection dir);
    
};

WX_DEFINE_ARRAY(ConnectionParams *, wxArrayOfConnPrm);

#endif
