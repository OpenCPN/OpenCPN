/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  RAZ Symbology Parser
 * Author:   David Register
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 */


#ifndef RAZDSPARSER_H_
#define RAZDSPARSER_H_
class RazdsParser {
public:
	RazdsParser();
	virtual ~RazdsParser();
	int LoadFile ( s52plib* plib, const wxString& PLib );

private:
	int ReadS52Line( char *pBuffer, const char *delim, int nCount, FILE *fp );
	int ChopS52Line( char *pBuffer, char c );
	int ParsePos( position *pos, char *buf, bool patt );
    int ParseLBID(FILE *fp);
    int ParseCOLS(FILE *fp);
    int ParseLUPT(FILE *fp);
    int ParseLNST(FILE *fp);
    int ParsePATT(FILE *fp);
    int ParseSYMB(FILE *fp, RuleHash *pHash);
    int _CIE2RGB( S52color* toRGB, double x, double y, double L );

    // working buffer
    #define  MAX_BUF  1024
    char buffer[MAX_BUF];
    char *pBuf;
    s52plib* plib;
    wxArrayPtrVoid* ColorTableArray;
    int m_LUPSequenceNumber;
};

#endif /* RAZDSPARSER_H_ */
