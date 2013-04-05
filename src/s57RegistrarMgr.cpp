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

#include <wx/log.h>

#include "s57RegistrarMgr.h"
#include "S57ClassRegistrar.h"

#ifdef USE_S57
extern S57ClassRegistrar *g_poRegistrar;
#endif

static int s57_initialize( const wxString& csv_dir, FILE *flog )
{

    //      Get one instance of the s57classregistrar,
    //      And be prepared to give it to any module that needs it

    if( g_poRegistrar == NULL ) {
        g_poRegistrar = new S57ClassRegistrar();

        if( !g_poRegistrar->LoadInfo( csv_dir.mb_str(), FALSE ) ) {
            wxString msg( _T("   Error: Could not load S57 ClassInfo from ") );
            msg.Append( csv_dir );
            wxLogMessage( msg );

            delete g_poRegistrar;
            g_poRegistrar = NULL;
        }
    }

    return 0;
}

s57RegistrarMgr::s57RegistrarMgr( const wxString& csv_dir, FILE *flog )
{
    s57_initialize( csv_dir, flog );
}

s57RegistrarMgr::~s57RegistrarMgr()
{
    delete g_poRegistrar;
    g_poRegistrar = NULL;
}

