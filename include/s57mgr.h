/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S57 Chart Manager
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   bdbcat@yahoo.com   *
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
 * $Log: s57mgr.h,v $
 * Revision 1.2  2010/04/27 01:45:32  bdbcat
 * Build 426
 *
 * Revision 1.1.1.1  2006/08/21 05:52:11  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.5  2006/08/04 11:43:37  dsr
 * no message
 *
 * Revision 1.4  2006/07/28 20:47:51  dsr
 * Cleanup
 *
 * Revision 1.3  2006/06/15 02:37:13  dsr
 * Support Multiple M_COVR objects
 *
 * Revision 1.2  2006/04/23 04:05:45  dsr
 * Add some accessors
 *
 * Revision 1.1.1.1  2006/04/19 03:23:28  dsr
 * Rename/Import to OpenCPN
 *
 * Revision 1.5  2006/03/16 03:28:12  dsr
 * Cleanup tabs
 *
 * Revision 1.4  2006/02/24 18:00:46  dsr
 * Cleanup, and delete static s57ClassRegistrar on dtor
 *
 * Revision 1.3  2006/02/23 01:26:04  dsr
 * Cleanup
 *
 *
 *
 */



#ifndef __S57MGR_H__
#define __S57MGR_H__


#include "s52s57.h"                 //types

#include <ogrsf_frmts.h>
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
