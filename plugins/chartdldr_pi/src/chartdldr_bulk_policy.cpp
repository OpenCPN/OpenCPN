/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "chartdldr_bulk_policy.h"

bool ChartDldrShouldAdvanceScheduledLastRun(int downloaded_ok, int attempted,
                                            int failed) {
  (void)failed;
  return attempted > 0 && downloaded_ok > 0;
}
