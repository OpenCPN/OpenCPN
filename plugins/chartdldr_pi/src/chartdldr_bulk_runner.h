/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_BULK_RUNNER_H_
#define CHARTDLDR_BULK_RUNNER_H_

#include "chartdldr_bulk.h"

class chartdldr_pi;
class ChartDldrPanelImpl;
class wxCommandEvent;

/** Plugin-level entry for manual and scheduled bulk update. */
bool ChartDldrRunBulkUpdate(chartdldr_pi* pi, ChartDldrBulkRunKind kind,
                            wxCommandEvent& event);

/**
 * Multi-source bulk orchestration. Uses ChartDldrPanelImpl as the wx download
 * transfer parent only (catalog refresh + chart loop).
 */
class ChartDldrBulkRunner {
public:
  static bool Run(ChartDldrPanelImpl* panel, ChartDldrBulkRunKind kind,
                  wxCommandEvent& event, ChartDldrBulkRunStats& stats);

private:
  static bool ConfirmBeforeStart(ChartDldrPanelImpl* panel);
};

#endif  // CHARTDLDR_BULK_RUNNER_H_
