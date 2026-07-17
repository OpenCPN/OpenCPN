/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_ANDROID_UI_H_
#define CHARTDLDR_ANDROID_UI_H_

#include <wx/colour.h>
#include <wx/window.h>

/** Display mm density used by Android dialog sizing (add-source, prefs). */
extern double g_androidDPmm;

#ifdef __ANDROID__
/** Load display density from the Android activity; sets g_androidDPmm. */
bool ChartDldrInitAndroidDisplayMetrics();
/** Recursively apply background color for Android prefs/dialogs. */
void ChartDldrSetAndroidBackColor(wxWindow* ctrl, wxColour col);
#else
inline bool ChartDldrInitAndroidDisplayMetrics() { return false; }
inline void ChartDldrSetAndroidBackColor(wxWindow*, wxColour) {}
#endif

#endif  // CHARTDLDR_ANDROID_UI_H_
