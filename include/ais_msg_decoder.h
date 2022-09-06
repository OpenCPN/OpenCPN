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

#ifndef _AIS_DECODE_MSG_H__
#define _AIS_DECODE_MSG_H__

#include <wx/string.h>

#include "AIS_Target_Data.h"
#include "AIS_Bitstring.h"
#include "AIS_Defs.h"
#include "ocpn_types.h"

struct AisDecodeCtx {
  wxString* accumulator; 
  AisDecodeCtx(wxString* s) : accumulator(s) {}
#ifdef AIS_DEBUG
  int& n_msg1;
  int& n_msg5;
  int& n_msg24;
  AisDecodeCtx(wxString* s, int& msg1, int& msg2, int &msg24)
      : accumulator(s), n_msg1(msg1), n_msg5(msg5), n_msg24(msg24) {}
#endif
};

class AisMsgDecoder {
public:
  AisMsgDecoder();

  ~AisMsgDecoder();

  bool Parse_VDXBitstring(AIS_Bitstring *bstr, AIS_Target_Data *ptd,
                          AisDecodeCtx ctx);

  AIS_Error DecodeSingleVDO(const wxString &str, GenericPosDatEx *pos,
                            AisDecodeCtx ctx);
  bool NMEACheckSumOK(const wxString& str);
private:
};

#endif    // _AIS_DECODE_MSG_H__
