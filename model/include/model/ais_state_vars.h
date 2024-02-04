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

/** \file ais_state_vars.h Global state for AIS decoder */

extern bool g_bAIS_ACK_Timeout;
extern bool g_bAIS_CPA_Alert;
extern bool g_bAIS_CPA_Alert_Audio;
extern bool g_bAIS_CPA_Alert_Suppress_Moored;
extern bool g_bAIS_DSC_Alert_Audio;
extern bool g_bAIS_GCPA_Alert_Audio;
extern bool g_bAIS_SART_Alert_Audio;
extern bool g_bAISShowTracks;
extern bool g_bAisTargetList_sortReverse;
extern bool g_bAllowShowScaled;
extern bool g_bAnchor_Alert_Audio;
extern bool g_bCPAMax;
extern bool g_bCPAWarn;
extern bool g_bDrawAISRealtime;
extern bool g_bDrawAISSize;
extern bool g_benableAISNameCache;
extern bool g_bHideMoored;
extern bool g_bMarkLost;
extern bool g_bRemoveLost;
extern bool g_bShowAIS;
extern bool g_bShowAISName;
extern bool g_bShowAreaNotices;
extern bool g_bShowCOG;
extern bool g_bShowScaled;
extern bool g_bSyncCogPredictors;
extern bool g_bTCPA_Max;
extern bool g_bTrackCarryOver;
extern bool g_AisFirstTimeUse;

extern double g_AckTimeout_Mins;
extern double g_AIS_RealtPred_Kts;
extern double g_AISShowTracks_Limit;
extern double g_AISShowTracks_Mins;
extern double g_CPAMax_NM;
extern double g_CPAWarn_NM;
extern double g_MarkLost_Mins;
extern double g_RemoveLost_Mins;
extern double g_ShowCOG_Mins;
extern double g_ShowMoored_Kts;
extern double g_TCPA_Max;

extern int g_ais_alert_dialog_sx, g_ais_alert_dialog_sy;
extern int g_ais_alert_dialog_x, g_ais_alert_dialog_y;
extern int g_ais_query_dialog_x, g_ais_query_dialog_y;
extern int g_AisTargetList_range;
extern int g_AisTargetList_sortColumn;
extern int g_ScaledNumWeightCPA;
extern int g_ScaledNumWeightRange;
extern int g_ScaledNumWeightSizeOfT;
extern int g_ScaledNumWeightSOG;
extern int g_ScaledNumWeightTCPA;
extern int g_ScaledSizeMinimal;
extern int g_ShowScaled_Num;
extern int g_Show_Target_Name_Scale;
extern int g_track_rotate_time;
extern int g_track_rotate_time_type;

extern wxString g_AIS_sound_file;
extern wxString g_AisTargetList_column_order;
extern wxString g_AisTargetList_column_spec;
extern wxString g_AisTargetList_perspective;
extern wxString g_anchorwatch_sound_file;
extern wxString g_DSC_sound_file;
extern wxString g_sAIS_Alert_Sound_File;
