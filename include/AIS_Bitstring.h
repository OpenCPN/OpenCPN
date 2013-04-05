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

#ifndef __AIS_BITSTRING_H__
#define __AIS_BITSTRING_H__

#define AIS_MAX_MESSAGE_LEN (10 * 82)           // AIS Spec allows up to 9 sentences per message, 82 bytes each
class AIS_Bitstring
{
public:

    AIS_Bitstring(const char *str);
    unsigned char to_6bit(const char c);

    /// sp is starting bit, 1-based
    int GetInt(int sp, int len, bool signed_flag = false);
    int GetStr(int sp, int bit_len, char *dest, int max_len);
    int GetBitCount();


private:

    unsigned char bitbytes[AIS_MAX_MESSAGE_LEN];
    int byte_length;
};

#endif
