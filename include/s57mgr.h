/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S57 Chart Manager
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 */



#ifndef __S57MGR_H__
#define __S57MGR_H__



#include "mygdal/ogrsf_frmts.h"
//----------------------------------------------------------------------------
//    Fwd Definitions
//----------------------------------------------------------------------------

bool ddfrecord_test();

//----------------------------------------------------------------------------
// s57mgr
//----------------------------------------------------------------------------
class s57mgr
{
public:
      s57mgr(const wxString& csv_dir);
      ~s57mgr();

      bool GetChartExtent(char *pFullPath, Extent *pext);
      int GetChartScale(char *pFullPath);

      bool GetChartFirstM_COVR(char *pFullPath, OGRDataSource **pDS, OGRFeature **pFeature,
                               OGRLayer **pLayer, int &catcov);

      bool GetChartNextM_COVR(OGRDataSource *pDS, OGRLayer *pLayer, OGRFeature *pLastFeature,
                              OGRFeature **pFeature, int &catcov);

      wxString *GetCSVDir(){return pcsv_locn;}



private:


      wxString              *pcsv_locn;
};

#endif
