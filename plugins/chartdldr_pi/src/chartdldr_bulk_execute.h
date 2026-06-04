/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_EXECUTE_H_
#define CHARTDLDR_BULK_EXECUTE_H_

#include "chartdldr_bulk_run.h"

class ChartDldrPanelImpl;
class wxCommandEvent;

struct ChartDldrBulkRunStats {
  int attempted = 0;
  int failed = 0;
  int new_downloads = 0;
  int updated_downloads = 0;

  int downloaded_ok() const { return attempted - failed; }
};

bool ChartDldrExecuteBulkUpdate(ChartDldrPanelImpl* panel,
                                ChartDldrBulkRunKind kind,
                                wxCommandEvent& event,
                                ChartDldrBulkRunStats& stats);

#endif  // CHARTDLDR_BULK_EXECUTE_H_
