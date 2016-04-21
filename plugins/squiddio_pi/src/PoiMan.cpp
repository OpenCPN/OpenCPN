/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose   Squiddio plugin
 * Note:     This work is based on the original RoutePointMan.cpp file which is:
 * Copyright (C) 2010 by David S. Register
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
 **************************************************************************/

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "wx/image.h"
#include "wx/tokenzr.h"
#include <wx/progdlg.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/apptrait.h>
#include <wx/listimpl.cpp>

#include <stdlib.h>
#include "PoiMan.h"

//--------------------------------------------------------------------------------
//      PoiMan   Implementation
//--------------------------------------------------------------------------------

PoiMan::PoiMan()
{
    m_pWayPointList = new PoiList;
    m_nGUID = 0;
}

PoiMan::~PoiMan()
{
    //    Two step here, since the RoutePoint dtor also touches the
    //    RoutePoint list.
    //    Copy the master RoutePoint list to a temporary list,
    //    then clear and delete objects from the temp list

    PoiList temp_list;

    wxPoiListNode *node = m_pWayPointList->GetFirst();
    while( node ) {
        Poi *pr = node->GetData();

        temp_list.Append( pr );
        node = node->GetNext();
    }

    temp_list.DeleteContents( true );
    temp_list.Clear();

    m_pWayPointList->Clear();
    delete m_pWayPointList;
}

bool PoiMan::AddRoutePoint(Poi *prp)
{
    if(!prp)
        return false;
    
    wxPoiListNode *prpnode = m_pWayPointList->Append(prp);
    prp->SetManagerListNode( prpnode );
    
    return true;
}

bool PoiMan::RemoveRoutePoint(Poi *prp)
{
    if(!prp)
        return false;
    
    wxPoiListNode *prpnode = (wxPoiListNode *)prp->GetManagerListNode();
    
    if(prpnode) 
        delete prpnode;
    else
        m_pWayPointList->DeleteObject(prp);
    
    prp->SetManagerListNode( NULL );
    
    return true;
}


