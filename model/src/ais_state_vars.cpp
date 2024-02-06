/**************************************************************************
 *   Copyright (C) 2022 David Register                                     *
 *   Copyright (C) 2022-2023  Alec Leamas                                  *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

/** \file  ais_state_vars.cpp Implement ais_state_vars.h */

#include <wx/string.h>

bool g_bAIS_ACK_Timeout;
bool g_bAIS_CPA_Alert;
bool g_bAIS_CPA_Alert_Audio;
bool g_bAIS_CPA_Alert_Suppress_Moored;
bool g_bAIS_DSC_Alert_Audio;
bool g_bAIS_GCPA_Alert_Audio;
bool g_bAIS_SART_Alert_Audio;
bool g_bAISShowTracks;
bool g_bAisTargetList_sortReverse;
bool g_bAllowShowScaled;
bool g_bAnchor_Alert_Audio;
bool g_bCPAMax;
bool g_bCPAWarn;
bool g_bDrawAISRealtime;
bool g_bDrawAISSize;
bool g_benableAISNameCache;
bool g_bHideMoored;
bool g_bMarkLost;
bool g_bRemoveLost;
bool g_bShowAIS;
bool g_bShowAISName;
bool g_bShowAreaNotices;
bool g_bShowCOG;
bool g_bShowScaled;
bool g_bSyncCogPredictors;
bool g_bTCPA_Max;
bool g_bTrackCarryOver;
bool g_AisFirstTimeUse;

double g_AckTimeout_Mins;
double g_AIS_RealtPred_Kts;
double g_AISShowTracks_Limit;
double g_AISShowTracks_Mins;
double g_CPAMax_NM;
double g_CPAWarn_NM;
double g_MarkLost_Mins;
double g_RemoveLost_Mins;
double g_ShowCOG_Mins;
double g_ShowMoored_Kts;
double g_TCPA_Max;

int g_ais_alert_dialog_sx, g_ais_alert_dialog_sy;
int g_ais_alert_dialog_x, g_ais_alert_dialog_y;
int g_ais_query_dialog_x, g_ais_query_dialog_y;
int g_AisTargetList_range;
int g_AisTargetList_sortColumn;
int g_ScaledNumWeightCPA;
int g_ScaledNumWeightRange;
int g_ScaledNumWeightSizeOfT;
int g_ScaledNumWeightSOG;
int g_ScaledNumWeightTCPA;
int g_ScaledSizeMinimal;
int g_ShowScaled_Num;
int g_Show_Target_Name_Scale;
int g_track_rotate_time;
int g_track_rotate_time_type;

wxString g_AIS_sound_file;
wxString g_AisTargetList_column_order;
wxString g_AisTargetList_column_spec;
wxString g_AisTargetList_perspective;
wxString g_anchorwatch_sound_file;
wxString g_DSC_sound_file;
wxString g_sAIS_Alert_Sound_File;
