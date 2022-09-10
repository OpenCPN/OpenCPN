/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  AIS info GUI parts.
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   Copyright (C) 2022 Alec Leamas                                        *
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

#include <wx/datetime.h>
#include <wx/event.h>
#include <wx/string.h>

#include "ais_decoder.h"
#include "ais_info_gui.h"
#include "AISTargetAlertDialog.h"
#include "ais_target_data.h"
#include "chcanv.h"
#include "ocpn_frame.h"
#include "OCPNPlatform.h"
#include "navutil.h"
#include "routemanagerdialog.h"
#include "RoutePoint.h"
#include "SoundFactory.h"
#include "undo.h"

wxDEFINE_EVENT(EVT_AIS_DEL_TRACK, wxCommandEvent);
wxDEFINE_EVENT(EVT_AIS_INFO, wxCommandEvent);
wxDEFINE_EVENT(EVT_AIS_NEW_TRACK, wxCommandEvent);
wxDEFINE_EVENT(EVT_AIS_TOUCH, wxCommandEvent);
wxDEFINE_EVENT(EVT_AIS_WP, wxCommandEvent);
wxDEFINE_EVENT(SOUND_PLAYED_EVTYPE, wxCommandEvent);

extern AisDecoder *g_pAIS;
extern AISTargetAlertDialog *g_pais_alert_dialog_active;
extern ArrayOfMmsiProperties g_MMSI_Props_Array;
extern bool g_bAIS_CPA_Alert;
extern bool g_bAIS_CPA_Alert_Audio;
extern bool g_bAIS_DSC_Alert_Audio;
extern bool g_bAIS_GCPA_Alert_Audio;
extern bool g_bAIS_SART_Alert_Audio;
extern bool g_bquiting;
extern int g_iSoundDeviceIndex;
extern OCPNPlatform *g_Platform;
extern Route *pAISMOBRoute;
extern wxString g_AIS_sound_file;
extern wxString g_CmdSoundString;
extern wxString g_DSC_sound_file;
extern wxString g_SART_sound_file;
extern MyConfig* pConfig;
extern RouteManagerDialog *pRouteManagerDialog;
extern MyFrame* gFrame;


static void onSoundFinished(void *ptr) {
  if (!g_bquiting) {
    wxCommandEvent ev(SOUND_PLAYED_EVTYPE);
    wxPostEvent(g_pAIS, ev);   // FIXME(leamas): Review sound handling.
  }
}

static void OnNewAisWaypoint(RoutePoint* pWP) {
  pConfig->AddNewWayPoint(pWP, -1);  // , -1 use auto next num
  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateWptListCtrl();
  if (gFrame->GetPrimaryCanvas()) {
    gFrame->GetPrimaryCanvas()->undo->BeforeUndoableAction(
        Undo_CreateWaypoint, pWP, Undo_HasParent, NULL);
    gFrame->GetPrimaryCanvas()->undo->AfterUndoableAction(NULL);
    gFrame->RefreshAllCanvas(false);
    gFrame->InvalidateAllGL();
  }
}

const static char* const kDeleteTrackPrompt =
R"""(
This AIS target has Persistent Tracking selected by MMSI properties
A Persistent track recording will therefore be restarted for this target.

Do you instead want to stop Persistent Tracking for this target?
)""";


static void OnDeleteTrack(MmsiProperties* props) {
  if (wxID_NO == OCPNMessageBox(NULL, kDeleteTrackPrompt, _("OpenCPN Info"),
                                wxYES_NO | wxCENTER, 60))
  {
    props->m_bPersistentTrack = true;
  }
}


AisInfoGui::AisInfoGui() {
  ais_info_listener = g_pAIS->info_update.GetListener(this, EVT_AIS_INFO);
  Bind(EVT_AIS_INFO, [&](wxCommandEvent ev) {
       auto palert_target = static_cast<AisTargetData*>(ev.GetClientData());
       ShowAisInfo(palert_target); });

  ais_touch_listener = g_pAIS->touch_state.GetListener(this, EVT_AIS_TOUCH);
  Bind(EVT_AIS_TOUCH, [&](wxCommandEvent ev) { gFrame->TouchAISActive(); });

  ais_wp_listener = g_pAIS->new_ais_wp.GetListener(this, EVT_AIS_WP);
  Bind(EVT_AIS_WP, [&](wxCommandEvent ev) {
       auto pWP = static_cast<RoutePoint*>(ev.GetClientData());
       OnNewAisWaypoint(pWP); });

  ais_new_track_listener = g_pAIS->new_ais_wp.GetListener(this, EVT_AIS_NEW_TRACK);
  Bind(EVT_AIS_NEW_TRACK, [&](wxCommandEvent ev) {
       auto t = static_cast<Track*>(ev.GetClientData());
       pConfig->AddNewTrack(t); });

  ais_del_track_listener = g_pAIS->new_ais_wp.GetListener(this, EVT_AIS_DEL_TRACK);
  Bind(EVT_AIS_DEL_TRACK, [&](wxCommandEvent ev) {
       auto t = static_cast< MmsiProperties*>(ev.GetClientData());
       OnDeleteTrack(t); });
}

void AisInfoGui::ShowAisInfo(AisTargetData* palert_target) {
   int audioType = AISAUDIO_NONE;
   if (palert_target) {
      bool b_jumpto = (palert_target->Class == AIS_SART) ||
                      (palert_target->Class == AIS_DSC);
      bool b_createWP = palert_target->Class == AIS_DSC;
      bool b_ack = palert_target->Class != AIS_DSC;

      //    Show the Alert dialog

      //      See FS# 968/998
      //      If alert occurs while OCPN is iconized to taskbar, then clicking
      //      the taskbar icon only brings up the Alert dialog, and not the
      //      entire application. This is an OS specific behavior, not seen on
      //      linux or Mac. This patch will allow the audio alert to occur, and
      //      the visual alert will pop up soon after the user selects the OCPN
      //      icon from the taskbar. (on the next timer tick, probably)

#ifndef __ANDROID__
      if (gFrame->IsIconized() || !gFrame->IsActive())
        gFrame->RequestUserAttention();
#endif

      if (!gFrame->IsIconized()) {
        AISTargetAlertDialog *pAISAlertDialog = new AISTargetAlertDialog();
        pAISAlertDialog->Create(palert_target->MMSI, gFrame, g_pAIS,
                                b_jumpto, b_createWP, b_ack, -1,
                                _("AIS Alert"));
        wxTimeSpan alertLifeTime(0, 1, 0,
                                 0);  // Alert default lifetime, 1 minute.
        palert_target->dtAlertExpireTime = wxDateTime::Now() + alertLifeTime;
        g_Platform->PositionAISAlert(pAISAlertDialog);

        g_pais_alert_dialog_active = pAISAlertDialog;
        pAISAlertDialog->Show();  // Show modeless, so it stays on the screen
      }

      //    Audio alert if requested
      m_bAIS_Audio_Alert_On = true;  // always on when alert is first shown
    }


  //    The AIS Alert dialog is already shown.  If the  dialog MMSI number is
  //    still alerted, update the dialog otherwise, destroy the dialog
  else {
    // Find the target with shortest CPA, ignoring DSC and SART targets
    double tcpa_min = 1e6;  // really long
    AisTargetData *palert_target_lowestcpa = NULL;
    const auto& current_targets = g_pAIS->GetTargetList();
    for (auto& it : current_targets) {
      AisTargetData *td = it.second;
      if (td) {
        if ((td->Class != AIS_SART) && (td->Class != AIS_DSC)) {
          if (g_bAIS_CPA_Alert && td->b_active) {
            if ((AIS_ALERT_SET == td->n_alert_state) && !td->b_in_ack_timeout) {
              if (td->TCPA < tcpa_min) {
                tcpa_min = td->TCPA;
                palert_target_lowestcpa = td;
              }
            }
          }
        }
      }
    }

    // Get the target currently displayed
    palert_target = g_pAIS->Get_Target_Data_From_MMSI(
        g_pais_alert_dialog_active->Get_Dialog_MMSI());

    //  If the currently displayed target is not alerted, it must be in "expiry
    //  delay" We should cancel that alert display now, and pick up the more
    //  critical CPA target on the next timer tick
    if (palert_target) {
      if (AIS_NO_ALERT == palert_target->n_alert_state) {
        if (palert_target_lowestcpa) {
          palert_target = NULL;
        }
      }
    }

    if (palert_target) {
      wxDateTime now = wxDateTime::Now();
      if (((AIS_ALERT_SET == palert_target->n_alert_state) &&
           !palert_target->b_in_ack_timeout) ||
          (palert_target->Class == AIS_SART) ) {
        g_pais_alert_dialog_active->UpdateText();
        // Retrigger the alert expiry timeout if alerted now
        wxTimeSpan alertLifeTime(0, 1, 0,
                                 0);  // Alert default lifetime, 1 minute.
        palert_target->dtAlertExpireTime = wxDateTime::Now() + alertLifeTime;
      }
      //  In "expiry delay"?
      else if (!palert_target->b_in_ack_timeout &&
               (now.IsEarlierThan(palert_target->dtAlertExpireTime))) {
        g_pais_alert_dialog_active->UpdateText();
      } else {
        g_pais_alert_dialog_active->Close();
        m_bAIS_Audio_Alert_On = false;
      }

      if (true == palert_target->b_suppress_audio)
        m_bAIS_Audio_Alert_On = false;
      else
        m_bAIS_Audio_Alert_On = true;
    } else {  // this should not happen, however...
      g_pais_alert_dialog_active->Close();
      m_bAIS_Audio_Alert_On = false;
    }
  }

  //    At this point, the audio flag is set
  //    Honor the global flag
  if (!g_bAIS_CPA_Alert_Audio) m_bAIS_Audio_Alert_On = false;

  if (m_bAIS_Audio_Alert_On) {
    if (!m_AIS_Sound) {
      m_AIS_Sound = SoundFactory(g_CmdSoundString.mb_str(wxConvUTF8));
    }
    if (!AIS_AlertPlaying()) {
      m_bAIS_AlertPlaying = true;
      wxString soundFile;
      switch (audioType) {
        case AISAUDIO_CPA:
        default:
          if (g_bAIS_GCPA_Alert_Audio) soundFile = g_AIS_sound_file;
          break;
        case AISAUDIO_DSC:
          if (g_bAIS_DSC_Alert_Audio) soundFile = g_DSC_sound_file;
          break;
        case AISAUDIO_SART:
          if (g_bAIS_SART_Alert_Audio) soundFile = g_SART_sound_file;
          break;
      }

      m_AIS_Sound->Load(soundFile, g_iSoundDeviceIndex);
      if (m_AIS_Sound->IsOk()) {
        m_AIS_Sound->SetFinishedCallback(onSoundFinished, this);
        if (!m_AIS_Sound->Play()) m_bAIS_AlertPlaying = false;
      } else
        m_bAIS_AlertPlaying = false;
    }
  }
  //  If a SART Alert is active, check to see if the MMSI has special properties
  //  set indicating that this Alert is a MOB for THIS ship.
  if (palert_target && (palert_target->Class == AIS_SART)) {
    for (unsigned int i = 0; i < g_MMSI_Props_Array.GetCount(); i++) {
      if (palert_target->MMSI == g_MMSI_Props_Array[i]->MMSI) {
        if (pAISMOBRoute)
          gFrame->UpdateAISMOBRoute(palert_target);
        else
          gFrame->ActivateAISMOBRoute(palert_target);
        break;
      }
    }
  }
}
