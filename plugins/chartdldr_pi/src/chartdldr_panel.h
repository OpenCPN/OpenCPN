/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef CHARTDLDR_PANEL_H_
#define CHARTDLDR_PANEL_H_

class wxScrolledWindow;

class chartdldr_pi;

bool ChartDldrPanelOnOptionsPage(const chartdldr_pi* pi);
void ChartDldrAttachDownloaderPanelToOptions(chartdldr_pi* pi,
                                             wxScrolledWindow* page);
void ChartDldrDestroyDownloaderUI(chartdldr_pi* pi);

#endif  // CHARTDLDR_PANEL_H_
