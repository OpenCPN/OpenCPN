/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_RUN_KIND_H_
#define CHARTDLDR_RUN_KIND_H_

enum class ChartDldrBulkRunKind { Interactive, Scheduled };

inline bool ChartDldrBulkRunIsScheduled(ChartDldrBulkRunKind kind) {
  return kind == ChartDldrBulkRunKind::Scheduled;
}

#endif  // CHARTDLDR_RUN_KIND_H_
