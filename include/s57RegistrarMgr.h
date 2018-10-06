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

#ifndef __S57REGISTRARMGR_H__
#define __S57REGISTRARMGR_H__

#include <wx/string.h>

WX_DECLARE_STRING_HASH_MAP( int, CSVHash1 );

WX_DECLARE_HASH_MAP( int,
                     std::string,
                     wxIntegerHash,
                     wxIntegerEqual,
                     CSVHash2 );


/**
 * s57RegistrarMgr Definition
 * This is a class holding the ctor and dtor for the global registrar
 */
class s57RegistrarMgr
{
public:
    s57RegistrarMgr(const wxString& csv_dir, FILE *flog);
    ~s57RegistrarMgr();
   
    int getAttributeID(const char *pAttrName);
    std::string getAttributeAcronym(int nID);
    std::string getFeatureAcronym(int nID);
    
private:
    
    bool s57_attr_init( const wxString& csv_dir );
    bool s57_feature_init( const wxString& csv_dir );
    
    CSVHash1       m_attrHash1;
    CSVHash2       m_attrHash2;

    CSVHash1       m_featureHash1;
    CSVHash2       m_featureHash2;
    
};

#endif
