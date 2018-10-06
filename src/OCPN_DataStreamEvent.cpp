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
 ***************************************************************************
 */

#include "OCPN_DataStreamEvent.h"

OCPN_DataStreamEvent::OCPN_DataStreamEvent(wxEventType commandType, int id)
      :wxEvent(id, commandType)
{
    m_pDataStream = NULL;
}

OCPN_DataStreamEvent::~OCPN_DataStreamEvent()
{
}

//----------------------------------------------------------------------------------
//     Strip NMEA V4 tags from message
//----------------------------------------------------------------------------------
wxString OCPN_DataStreamEvent::ProcessNMEA4Tags()
{
    wxString msg = wxString(GetNMEAString().c_str(), wxConvUTF8);
   
    int idxFirst =  msg.Find('\\');
    
    if(wxNOT_FOUND == idxFirst)
        return msg;
    
    if(idxFirst < (int)msg.Length()-1){
        int idxSecond = msg.Mid(idxFirst + 1).Find('\\') + 1;
        if(wxNOT_FOUND != idxSecond){
            if(idxSecond < (int)msg.Length()-1){
                
               // wxString tag = msg.Mid(idxFirst+1, (idxSecond - idxFirst) -1);
                return msg.Mid(idxSecond + 1);
            }
        }
    }
    
    return msg;
}


wxEvent* OCPN_DataStreamEvent::Clone() const
{
    OCPN_DataStreamEvent *newevent=new OCPN_DataStreamEvent(*this);
    newevent->m_NMEAstring=this->m_NMEAstring;
    newevent->m_pDataStream = this->m_pDataStream;
    return newevent;
}

