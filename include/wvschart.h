/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  World Vector Shoreline (WVS) Chart Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *
 */



#ifndef __WVSCHART_H__
#define __WVSCHART_H__


#include "chartbase.h"

class ocpnDC;
// ----------------------------------------------------------------------------
// Useful Prototypes
// ----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// WVSChart
//----------------------------------------------------------------------------
class WVSChart
{
public:
      WVSChart(wxWindow *parent, const wxString& wvs_home_dir);
      ~WVSChart();

      void RenderViewOnDC(ocpnDC& dc, ViewPort& VPoint);

private:
      wxString    *pwvs_home_dir;
      wxString    *pwvs_file_name;

      float       *plat_ray[360][180];
      float       *plon_ray[360][180];
      int         *pseg_ray[360][180];
      int         nseg[360][180];

      wxPoint     *ptp;
      int         cur_seg_cnt_max;

      bool        m_ok;


};

#endif
