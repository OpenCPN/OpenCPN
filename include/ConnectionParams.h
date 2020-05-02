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

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

class ConnectionParams;
class options;

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
    SIGNALK = 3,
    PROTO_UNDEFINED = 4
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

#define CONN_ENABLE_ID 47621

class ConnectionParamsPanel: public wxPanel
{
public:
    ConnectionParamsPanel( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                           ConnectionParams *p_itemConnectionParams, options *pContainer );
    ~ConnectionParamsPanel();
    
    void OnSelected( wxMouseEvent &event );
    void SetSelected( bool selected );
    void OnPaint( wxPaintEvent &event );
    void OnEraseBackground( wxEraseEvent &event );
    void CreateControls( void );
    void OnEnableCBClick(wxCommandEvent &event);
    void Update( ConnectionParams *ConnectionParams);
 
    bool GetSelected(){ return m_bSelected; }
    int GetUnselectedHeight(){ return m_unselectedHeight; }
    ConnectionParams *m_pConnectionParams;
    
private:
    options *m_pContainer;
    bool m_bSelected;
    wxStaticText *m_pName;
    wxColour m_boxColour;
    int m_unselectedHeight;
    wxCheckBox *m_cbEnable;
    wxStaticText *t2;
    wxStaticText *t4;
    wxStaticText *t6;
    wxStaticText *t12;
    wxStaticText *t14;
    wxStaticText *t16;
    wxStaticText *t18;

    wxStaticText *t21;

    
    DECLARE_EVENT_TABLE()
};




class ConnectionParams
{
public:
    ConnectionParams();
    ~ConnectionParams();
    ConnectionParams(const wxString &configStr);

    ConnectionType  Type;
    NetworkProtocol NetProtocol;
    wxString        NetworkAddress;
    int             NetworkPort;
    
    wxString        LastNetworkAddress;
    int             LastNetworkPort;
    NetworkProtocol LastNetProtocol;
    
    DataProtocol    Protocol;
    wxString        Port;
    int             Baudrate;
    bool            ChecksumCheck;
    bool            Garmin;
    bool            GarminUpload;
    bool            FurunoGP3X;
    bool            AutoSKDiscover;
    dsPortType      IOSelect;
    ListType        InputSentenceListType;
    wxArrayString   InputSentenceList;
    ListType        OutputSentenceListType;
    wxArrayString   OutputSentenceList;
    int             Priority;
    bool            bEnabled;
    wxString        UserComment;
    
    wxString        Serialize() const;
    void            Deserialize(const wxString &configStr);

    wxString GetSourceTypeStr() const;
    wxString GetAddressStr() const;
    wxString GetParametersStr() const;
    wxString GetIOTypeValueStr() const;
    wxString GetFiltersStr() const;
    wxString GetDSPort() const;
    wxString GetLastDSPort() const;
    wxString GetPortStr() const { return Port; }
    void SetPortStr( wxString str ){ Port = str; }
    
    
    bool            Valid;
    bool            b_IsSetup;
    ConnectionParamsPanel *m_optionsPanel;
private:
    wxString FilterTypeToStr(ListType type, FilterDirection dir) const;
    
};

WX_DEFINE_ARRAY(ConnectionParams *, wxArrayOfConnPrm);

#endif
