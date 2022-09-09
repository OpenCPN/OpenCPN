/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  AIS Decoder Object
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 */

#ifndef __AIS_H__
#define __AIS_H__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include <wx/datetime.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/wxhtml.h>

#include <vector>

#include "wx/sound.h"

#include "dychart.h"
#include "gui_lib.h"
#include "navutil.h"
#include "OCPN_Sound.h"
#include "ais_bitstring.h"
#include "AISTargetListDialog.h"
#include "ais_target_data.h"
#include "ais_defs.h"

//  FWD definitions
class ChartCanvas;

//    Constants
#ifndef PI
#define PI 3.1415926535897931160E0 /* pi */
#endif

#define TIMER_AIS_MSEC 998
#define TIMER_AIS_AUDIO_MSEC 2000


enum {
  tlTRK = 0,
  tlNAME,
  tlCALL,
  tlMMSI,
  tlCLASS,
  tlTYPE,
  tlNAVSTATUS,
  tlBRG,
  tlRNG,
  tlCOG,
  tlSOG,
  tlCPA,
  tlTCPA
};  // AISTargetListCtrl Columns;

//      Describe NavStatus variable
// IMO Circ. 289 Area Notices, based on libais
const size_t AIS8_001_22_NUM_NAMES = 128;
const size_t AIS8_001_22_SUBAREA_SIZE = 87;

extern wxString ais8_001_22_notice_names[];
// WX_DECLARE_LIST(Ais8_001_22_SubArea, Ais8_001_22_SubAreaList);

//---------------------------------------------------------------------------------
//
//  AIS_Decoder Helpers
//
//---------------------------------------------------------------------------------
WX_DEFINE_SORTED_ARRAY(AIS_Target_Data *, ArrayOfAISTarget);

//      Implement the AISTargetList as a wxHashMap


void AISDrawAreaNotices(ocpnDC &dc, ViewPort &vp, ChartCanvas *cp);
void AISDraw(ocpnDC &dc, ViewPort &vp, ChartCanvas *cp);
bool AnyAISTargetsOnscreen(ChartCanvas *cc, ViewPort &vp);

#endif
