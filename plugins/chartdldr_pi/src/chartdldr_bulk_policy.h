/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_POLICY_H_
#define CHARTDLDR_BULK_POLICY_H_

/** True when a completed scheduled run should advance ScheduledUpdateLastRun. */
bool ChartDldrShouldAdvanceScheduledLastRun(int downloaded_ok, int attempted,
                                            int failed);

#endif  // CHARTDLDR_BULK_POLICY_H_
