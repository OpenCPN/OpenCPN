/**************************************************************************
 *   Copyright (C) 2026 by David S. Register                               *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 ***************************************************************************/

/**
 * \file
 *
 * Implement threaded chart database creation
 */

#include <stdint.h>

#include <wx/wxprec.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>
#include <wx/wx.h>

#include "config.h"

#include "chartbase.h"
#include "chartdb.h"
#include "chartdbs.h"
#include "chartimg.h"
#include "chcanv.h"
#include "dychart.h"
#include "gl_chart_canvas.h"
#include "gl_tex_cache.h"
#include "gl_texture_descr.h"
#include "lz4.h"
#include "lz4hc.h"
#include "ocpn_platform.h"
#include "quilt.h"
#include "squish.h"
#include "viewport.h"
#include "chartdb_thread.h"

wxDEFINE_EVENT(wxEVT_OCPN_CHARTTABLEENTRYTHREAD,
               OCPN_ChartTableEntryThreadEvent);

// Static globals
extern ChartDB *ChartData;

//  ChartTableEntryJobTicket implementation
bool ChartTableEntryJobTicket::DoJob() {
  // printf("DoJob\n");
  ChartDatabase *db = dynamic_cast<ChartDatabase *>(ChartData);
  ChartTableEntry *pnewChartTableEntry =
      db->CreateChartTableEntry(m_ChartPath, m_ChartPath, chart_desc);
  if (pnewChartTableEntry) {
    std::shared_ptr<ChartTableEntry> safe_ptr(pnewChartTableEntry);
    m_chart_table_entry = safe_ptr;  // class member
  }

  return true;
}
