/******************************************************************************
 * $Id: wvschart.h,v 1.8 2010/05/15 03:55:58 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  World Vector Shoreline (WVS) Chart Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 * $Log: wvschart.h,v $
 * Revision 1.8  2010/05/15 03:55:58  bdbcat
 * Build 514
 *
 * Revision 1.7  2010/04/27 01:45:49  bdbcat
 * Build 426
 *
 * Revision 1.6  2010/03/29 02:59:02  bdbcat
 * 2.1.0 Beta Initial
 *
 */



#ifndef __WVSCHART_H__
#define __WVSCHART_H__


#include "chartbase.h"
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

      void RenderViewOnDC(wxMemoryDC& dc, ViewPort& VPoint);
      bool RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint, const wxRegion &Region);

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
