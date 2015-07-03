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
 **************************************************************************/

#include <wx/tokenzr.h>
#include <wx/regex.h>

#include "PositionParser.h"
#include "navutil.h"

PositionParser::PositionParser(const wxString & src)
{
    parsedOk = false;
    if( FindSeparator( src ) ) {
        latitude = fromDMM( latitudeString );
        longitude = fromDMM( longitudeString );
        if( (latitude != 0.0) && (longitude != 0.0) ) parsedOk = true;
    }
}

bool PositionParser::FindSeparator(const wxString & src)
{ 

    // Used when format is similar to "12 34.56 N 12 34.56 E"
    wxString posPartOfSeparator = _T("");

    // First the XML case:
    // Generalized XML tag format, accepts anything like <XXX yyy="<lat>" zzz="<lon>" >
    // GPX format <wpt lat="<lat>" lon="<lon>" /> tag among others.

    wxRegEx regex;

    int re_compile_flags = wxRE_ICASE;
#ifdef wxHAS_REGEX_ADVANCED
    re_compile_flags |= wxRE_ADVANCED;
#endif

    regex.Compile(
            _T( "<[a-z,A-Z]*\\s*[a-z,A-Z]*=\"([0-9,.]*)\"\\s*[a-z,A-Z]*=\"([-,0-9,.]*)\"\\s*/*>" ),
                  re_compile_flags );

    if( regex.IsValid() ) {
        if( regex.Matches( src ) ) {
			int n = regex.GetMatchCount();
            latitudeString = regex.GetMatch( src, 1 );
            longitudeString = regex.GetMatch( src, 2 );
            latitudeString.Trim( true );
            latitudeString.Trim( false );
            longitudeString.Trim( true );
            longitudeString.Trim( false );
            return true;
        }
    }

    // Now try various separators.

    separator = _T(", ");
    wxStringTokenizer tk1(src, separator);
    if (tk1.CountTokens() == 2) {
        latitudeString = tk1.GetNextToken();
        latitudeString.Trim(true);
        latitudeString.Trim(false);
        longitudeString = tk1.GetNextToken();
        longitudeString.Trim(true);
        longitudeString.Trim(false);

        return true;
    }

    separator = _T(",");
    wxStringTokenizer tk2 (src, separator);
    if (tk2.CountTokens() == 2) {
        latitudeString = tk2.GetNextToken();
        latitudeString.Trim(true);
        latitudeString.Trim(false);
        longitudeString = tk2.GetNextToken();
        longitudeString.Trim(true);
        longitudeString.Trim(false);

        return true;
    }   

    separator = _T(" ");
    wxStringTokenizer tk3(src, separator);
    if (tk3.CountTokens() == 2) {
        latitudeString = tk3.GetNextToken();
        latitudeString.Trim(true);
        latitudeString.Trim(false);
        longitudeString = tk3.GetNextToken();
        longitudeString.Trim(true);
        longitudeString.Trim(false);

        return true;
    }
    
    separator = _T("\t");
    wxStringTokenizer tk4(src, separator);
    if (tk4.CountTokens() == 2) {
        latitudeString = tk4.GetNextToken();
        latitudeString.Trim(true);
        latitudeString.Trim(false);
        longitudeString = tk4.GetNextToken();
        longitudeString.Trim(true);
        longitudeString.Trim(false);

        return true;
    }
   
    separator = _T("\n");
    wxStringTokenizer tk5(src, separator);
    if (tk5.CountTokens() == 2) {
        latitudeString = tk5.GetNextToken();
        latitudeString.Trim(true);
        latitudeString.Trim(false);
        longitudeString = tk5.GetNextToken();
        longitudeString.Trim(true);
        longitudeString.Trim(false);

        return true;
    }
  
    separator = _T("N");   
    posPartOfSeparator = _T("N");
    wxStringTokenizer tk6(src, separator);
    if (tk6.CountTokens() == 2) {
        latitudeString = tk6.GetNextToken() << posPartOfSeparator;
        latitudeString.Trim(true);
        latitudeString.Trim(false);
        longitudeString = tk6.GetNextToken();
        longitudeString.Trim(true);
        longitudeString.Trim(false);

        return true;
    }
   
    separator = _T("S");   
    posPartOfSeparator = _T("S");
    wxStringTokenizer tk7(src, separator);
    if (tk7.CountTokens() == 2) {
        latitudeString = tk7.GetNextToken() << posPartOfSeparator;
        latitudeString.Trim(true);
        latitudeString.Trim(false);
        longitudeString = tk7.GetNextToken();
        longitudeString.Trim(true);
        longitudeString.Trim(false);

        return true;
    }   

    // Give up.
    return false;
}

