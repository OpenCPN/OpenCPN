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

#ifndef __S57CLASSREGISTRAR_H__
#define __S57CLASSREGISTRAR_H__

class S57ClassRegistrar
{
    // Class information:
    int         nClasses;

    int         iCurrentClass;

    char      **papszCurrentFields;

    char      **papszTempResult;

    int        *pnClassesOBJL;
    char     ***papapszClassesTokenized;

    // Attribute Information:
    int         nAttrMax;
    int         nAttrCount;
    char      **papszAttrNames;
    char      **papszAttrAcronym;
    char     ***papapszAttrValues;
    char       *pachAttrType;
    char       *pachAttrClass;
    int        *panAttrIndex; // sorted by acronym.

    int         FindFile( const char *pszTarget, const char *pszDirectory,
                          int bReportErr, FILE **fp );

    const char *ReadLine( FILE * fp );
    char      **papszNextLine;
    void        DestroySparseStringlist(char **papszStrList);

public:
                S57ClassRegistrar();
               ~S57ClassRegistrar();

    int         LoadInfo( const char *, int );

    // class table methods.
    int         SelectClassByIndex( int );
    int         SelectClass( int );
    int         SelectClass( const char * );

    int         Rewind() { return SelectClassByIndex(0); }
    int         NextClass() { return SelectClassByIndex(iCurrentClass+1); }

    int         GetOBJL();
    const char *GetDescription();
    const char *GetAcronym();

    char      **GetAttributeList( const char * = NULL );

    char        GetClassCode();
    char      **GetPrimitives();

    // attribute table methods.
    int         GetMaxAttrIndex() { return nAttrMax; }
    const char *GetAttrName( int i ) { return papszAttrNames[i]; }
    const char *GetAttrAcronym( int i ) { return papszAttrAcronym[i]; }
    char      **GetAttrValues( int i ) { return papapszAttrValues[i]; }
    char        GetAttrType( int i ) { return pachAttrType[i]; }
#define SAT_ENUM        'E'
#define SAT_LIST        'L'
#define SAT_FLOAT       'F'
#define SAT_INT         'I'
#define SAT_CODE_STRING 'A'
#define SAT_FREE_TEXT   'S'

    char        GetAttrClass( int i ) { return pachAttrClass[i]; }
    int         FindAttrByAcronym( const char * );

};

#endif
