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

/** True when the downloader panel is currently parented by the options page. */
bool ChartDldrPanelOnOptionsPage(const chartdldr_pi* pi);

/** Temporary leave-host: show the long-lived panel on the options page. */
void ChartDldrShowDownloaderPanelOnOptions(chartdldr_pi* pi,
                                           wxScrolledWindow* page);

/**
 * Sole home for the panel outside an open toolbox. Survives options
 * destroy/recreate (font/locale reload) and keeps scheduled runs alive.
 */
void ChartDldrParkDownloaderPanelOnHost(chartdldr_pi* pi);

void ChartDldrDestroyDownloaderUI(chartdldr_pi* pi);

#endif  // CHARTDLDR_PANEL_H_
