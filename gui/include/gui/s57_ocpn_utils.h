/***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   Copyright (C) 2024 Alec Leamas                                        *
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

/** \file s57_ocpn_util.h S57 utilities with core opencpn dependencies. */

#ifndef S57_OCPN_UTILS_H_GUARD__
#define S57_OCPN_UTILS_H_GUARD__

#include "OCPNRegion.h"
#include "ocpndc.h"
#include "s57chart.h"

bool s57_ProcessExtendedLightSectors(ChartCanvas *cc,
                                     ChartPlugInWrapper *target_plugin_chart,
                                     s57chart *Chs57,
                                     ListOfObjRazRules *rule_list,
                                     std::list<S57Obj *> *pi_rule_list,
                                     std::vector<s57Sector_t> &sectorlegs);

void s57_DrawExtendedLightSectors(ocpnDC &temp_dc, ViewPort &VPoint,
                                  std::vector<s57Sector_t> &sectorlegs);

void s57_DrawExtendedLightSectorsGL(ocpnDC &temp_dc, ViewPort &VPoint,
                                    std::vector<s57Sector_t> &sectorlegs);

#endif  //  S57_OCPN_UTILS_H_GUARD__
