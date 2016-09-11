/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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
#include <wx/tokenzr.h>
#include <wx/textfile.h>
#include <wx/filename.h>

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
    
    //  Create and initialize the S57 Attribute helpers
    s57_attr_init( csv_dir );
    //  Create and initialize the S57 Feature code helpers
    s57_feature_init( csv_dir );
}

s57RegistrarMgr::~s57RegistrarMgr()
{
    delete g_poRegistrar;
    g_poRegistrar = NULL;
}

bool s57RegistrarMgr::s57_attr_init( const wxString& csv_dir ){
    
    //  Find, open, and read the file {csv_dir}/s57attributes.csv
    wxString csv_t = csv_dir;
    wxChar sep = wxFileName::GetPathSeparator();
    if( csv_t.Last() != sep ) csv_t.Append( sep );
    
    
    wxTextFile tFile;
    wxString targetFile = csv_t + _T("s57attributes.csv");
    
    if(!tFile.Open( targetFile ) ){
        wxString msg( _T("   Error: Could not load S57 Attribute Info from ") );
        msg.Append( csv_dir );
        wxLogMessage( msg );
        
        return false;
    }

    //  populate the member hashmaps
    
    //First map: Key is char[] attribute acronym, value is standard ID
    //Second map: Key is standard ID, value is char[] attribute acronym
    
    wxString str;
    for ( str = tFile.GetFirstLine(); !tFile.Eof(); str = tFile.GetNextLine() ){
        wxStringTokenizer tk(str, _T(","));
        
        wxString ident = tk.GetNextToken();
        long nID = -1;
        if( ident.ToLong( &nID )){
            wxString description = tk.GetNextToken();
            wxString acronym = tk.GetNextToken();
            
            m_attrHash1[acronym] = nID;
            m_attrHash2[nID] = acronym.c_str();
            
        }
    }

    return true;     
    
}

bool s57RegistrarMgr::s57_feature_init( const wxString& csv_dir ){
    
    //  Find, open, and read the file {csv_dir}/s57objectclasses.csv
    wxString csv_t = csv_dir;
    wxChar sep = wxFileName::GetPathSeparator();
    if( csv_t.Last() != sep ) csv_t.Append( sep );
    
    
    wxTextFile tFile;
    wxString targetFile = csv_t + _T("s57objectclasses.csv");
    
    if(!tFile.Open( targetFile ) ){
        wxString msg( _T("   Error: Could not load S57 Feature Info from ") );
        msg.Append( csv_dir );
        wxLogMessage( msg );
        
        return false;
    }
    
    //  populate the member hashmaps
    
    //First map: Key is char[] feature acronym, value is standard ID
    //Second map: Key is standard ID, value is char[] feature acronym
    
    wxString str;
    for ( str = tFile.GetFirstLine(); !tFile.Eof(); str = tFile.GetNextLine() ){
        wxStringTokenizer tk(str, _T(","));
        
        wxString ident = tk.GetNextToken();
        long nID = -1;
//         if( ident.ToLong( &nID )){
//             wxString description = tk.GetNextToken();
//             wxString acronym = tk.GetNextToken();
//             
//             m_featureHash1[acronym] = nID;
//             m_featureHash2[nID] = acronym.c_str();
//             
//         }
        if( ident.ToLong( &nID )){
            wxString description = tk.GetNextToken();
//            wxString d2;
            while(!description.EndsWith("\""))
                description += tk.GetNextToken();
            
            wxString acronym = tk.GetNextToken();
            
            m_featureHash1[acronym] = nID;
            m_featureHash2[nID] = acronym.c_str();
            
        }
    }
    
    return true;     
    
}

int s57RegistrarMgr::getAttributeID(const char *pAttrName){
    wxString key(pAttrName);
    
    if( m_attrHash1.find( key ) == m_attrHash1.end())
        return -1;
    else
        return m_attrHash1[key];
}

std::string s57RegistrarMgr::getAttributeAcronym(int nID){
    
    if( m_attrHash2.find( nID ) == m_attrHash2.end())
        return "";
    else
        return m_attrHash2[nID];
}


std::string s57RegistrarMgr::getFeatureAcronym(int nID){
    
    if( m_featureHash2.find( nID ) == m_featureHash2.end())
        return "";
    else
        return m_featureHash2[nID];
}
