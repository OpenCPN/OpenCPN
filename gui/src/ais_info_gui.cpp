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

#include <memory>

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers


#include <wx/datetime.h>
#include <wx/event.h>
#include <wx/string.h>

#include "model/ais_decoder.h"
#include "model/ais_state_vars.h"
#include "model/ais_target_data.h"
#include "model/route_point.h"

#include "ais_info_gui.h"
#include "AISTargetAlertDialog.h"
#include "chcanv.h"
#include "navutil.h"
#include "ocpn_frame.h"
#include "OCPNPlatform.h"
#include "routemanagerdialog.h"
#include "SoundFactory.h"
#include "undo.h"

wxDEFINE_EVENT(EVT_AIS_DEL_TRACK, wxCommandEvent);
wxDEFINE_EVENT(EVT_AIS_INFO, ObservedEvt);
wxDEFINE_EVENT(EVT_AIS_NEW_TRACK, wxCommandEvent);
wxDEFINE_EVENT(EVT_AIS_TOUCH, wxCommandEvent);
wxDEFINE_EVENT(EVT_AIS_WP, wxCommandEvent);
wxDEFINE_EVENT(SOUND_PLAYED_EVTYPE, wxCommandEvent);

extern ArrayOfMmsiProperties g_MMSI_Props_Array;
extern bool g_bquiting;
extern int g_iSoundDeviceIndex;
extern OCPNPlatform *g_Platform;
extern Route *pAISMOBRoute;
extern wxString g_CmdSoundString;
extern MyConfig* pConfig;
extern RouteManagerDialog *pRouteManagerDialog;
extern MyFrame* gFrame;
extern AisInfoGui *g_pAISGUI;

static void onSoundFinished(void *ptr) {
  if (!g_bquiting) {
    wxCommandEvent ev(SOUND_PLAYED_EVTYPE);
    wxPostEvent(g_pAISGUI, ev);   // FIXME(leamas): Review sound handling.
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
_(R"(
This AIS target has Persistent Tracking selected by MMSI properties
A Persistent track recording will therefore be restarted for this target.

Do you instead want to stop Persistent Tracking for this target?
)");


static void OnDeleteTrack(MmsiProperties* props) {
  if (wxID_NO == OCPNMessageBox(NULL, kDeleteTrackPrompt, _("OpenCPN Info"),
                                wxYES_NO | wxCENTER, 60))
  {
    props->m_bPersistentTrack = true;
  }
}


AisInfoGui::AisInfoGui() {
  ais_info_listener.Listen(g_pAIS->info_update, this, EVT_AIS_INFO);

  Bind(EVT_AIS_INFO, [&](ObservedEvt &ev) {
        auto ptr = ev.GetSharedPtr();
        auto palert_target = std::static_pointer_cast<const AisTargetData>(ptr);
        ShowAisInfo(palert_target); }
       );

  ais_touch_listener.Listen(g_pAIS->touch_state, this, EVT_AIS_TOUCH);
  Bind(EVT_AIS_TOUCH, [&](wxCommandEvent ev) { gFrame->TouchAISActive(); });

  ais_wp_listener.Listen(g_pAIS->new_ais_wp, this, EVT_AIS_WP);
  Bind(EVT_AIS_WP, [&](wxCommandEvent ev) {
       auto pWP = static_cast<RoutePoint*>(ev.GetClientData());
       OnNewAisWaypoint(pWP); });

  ais_new_track_listener.Listen(g_pAIS->new_ais_wp, this,
                                EVT_AIS_NEW_TRACK);
  Bind(EVT_AIS_NEW_TRACK, [&](wxCommandEvent ev) {
       auto t = static_cast<Track*>(ev.GetClientData());
       pConfig->AddNewTrack(t); });

  ais_del_track_listener.Listen(g_pAIS->new_ais_wp, this,
                                EVT_AIS_DEL_TRACK);
  Bind(EVT_AIS_DEL_TRACK, [&](wxCommandEvent ev) {
       auto t = static_cast< MmsiProperties*>(ev.GetClientData());
       OnDeleteTrack(t); });

  Bind(SOUND_PLAYED_EVTYPE, [&](wxCommandEvent ev) {
       OnSoundFinishedAISAudio(ev); });

  m_AIS_Sound = 0;
  m_bAIS_Audio_Alert_On = false;
  m_bAIS_AlertPlaying = false;

}

void AisInfoGui::OnSoundFinishedAISAudio(wxCommandEvent &event) {
  // By clearing this flag the main event loop will trigger repeated
  // sounds for as long as the alert condition remains.

  // Unload/reset OcpnSound object.
  m_AIS_Sound->UnLoad();

  m_bAIS_AlertPlaying = false;
}

void AisInfoGui::ShowAisInfo(std::shared_ptr<const AisTargetData> palert_target) {
   if (!palert_target) return;

   int audioType = AISAUDIO_NONE;

   switch (palert_target->Class){
    case AIS_DSC:
      audioType = AISAUDIO_DSC;
      break;
    case AIS_SART:
      audioType = AISAUDIO_SART;
      break;
    default:
      audioType = AISAUDIO_CPA;
      break;
   }

   // If no alert dialog shown yet...
   if (!g_pais_alert_dialog_active) {
      bool b_jumpto = palert_target->Class == AIS_SART ||
                      palert_target->Class == AIS_DSC;
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

        g_pais_alert_dialog_active = pAISAlertDialog;

        wxTimeSpan alertLifeTime(0, 1, 0,
                                 0);  // Alert default lifetime, 1 minute.
        auto alert_dlg_active =
            dynamic_cast<AISTargetAlertDialog*>(g_pais_alert_dialog_active);
        alert_dlg_active->dtAlertExpireTime = wxDateTime::Now() + alertLifeTime;
        g_Platform->PositionAISAlert(pAISAlertDialog);

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
      auto td = it.second;
      if (td) {
        if (td->Class != AIS_SART && td->Class != AIS_DSC) {
          if (g_bAIS_CPA_Alert && td->b_active) {
            if (AIS_ALERT_SET == td->n_alert_state && !td->b_in_ack_timeout) {
              if (td->TCPA < tcpa_min) {
                tcpa_min = td->TCPA;
                palert_target_lowestcpa = td.get();
              }
            }
          }
        }
      }
    }

    // Get the target currently displayed
    auto alert_dlg_active =
        dynamic_cast<AISTargetAlertDialog*>(g_pais_alert_dialog_active);
    palert_target = g_pAIS->Get_Target_Data_From_MMSI(
        alert_dlg_active->Get_Dialog_MMSI());

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
      if ((AIS_ALERT_SET == palert_target->n_alert_state &&
           !palert_target->b_in_ack_timeout) ||
          palert_target->Class == AIS_SART ) {
        alert_dlg_active->UpdateText();
        // Retrigger the alert expiry timeout if alerted now
        wxTimeSpan alertLifeTime(0, 1, 0,
                                 0);  // Alert default lifetime, 1 minute.
        alert_dlg_active->dtAlertExpireTime = wxDateTime::Now() + alertLifeTime;
      }
      //  In "expiry delay"?
      else if (!palert_target->b_in_ack_timeout &&
               now.IsEarlierThan(alert_dlg_active->dtAlertExpireTime)) {
        alert_dlg_active->UpdateText();
      } else {
        alert_dlg_active->Close();
        m_bAIS_Audio_Alert_On = false;
      }

      if (true == palert_target->b_suppress_audio)
        m_bAIS_Audio_Alert_On = false;
      else
        m_bAIS_Audio_Alert_On = true;
    } else {  // this should not happen, however...
      alert_dlg_active->Close();
      m_bAIS_Audio_Alert_On = false;
    }
  }

  //    At this point, the audio flag is set
  //    Honor the global flag
  if (!g_bAIS_CPA_Alert_Audio) m_bAIS_Audio_Alert_On = false;

  if (m_bAIS_Audio_Alert_On) {
    if (!m_AIS_Sound) {
      m_AIS_Sound = SoundFactory(/*g_CmdSoundString.mb_str(wxConvUTF8)*/);
    }
    if (!AIS_AlertPlaying()) {
      m_bAIS_AlertPlaying = true;
      wxString soundFile;
      switch (audioType) {
        case AISAUDIO_DSC:
          if (g_bAIS_DSC_Alert_Audio) soundFile = g_DSC_sound_file;
          break;
        case AISAUDIO_SART:
          if (g_bAIS_SART_Alert_Audio) soundFile = g_SART_sound_file;
          break;
        case AISAUDIO_CPA:
        default:
          if (g_bAIS_GCPA_Alert_Audio) soundFile = g_AIS_sound_file;
          break;
      }

      m_AIS_Sound->Load(soundFile, g_iSoundDeviceIndex);
      if (m_AIS_Sound->IsOk()) {
        m_AIS_Sound->SetFinishedCallback(onSoundFinished, this);
        if (!m_AIS_Sound->Play()){
          delete m_AIS_Sound;
          m_AIS_Sound = 0;
          m_bAIS_AlertPlaying = false;
        }
      } else {
        delete m_AIS_Sound;
        m_AIS_Sound = 0;
        m_bAIS_AlertPlaying = false;
      }
    }
  }
  //  If a SART Alert is active, check to see if the MMSI has special properties
  //  set indicating that this Alert is a MOB for THIS ship.
  if (palert_target && palert_target->Class == AIS_SART) {
    for (unsigned int i = 0; i < g_MMSI_Props_Array.GetCount(); i++) {
      if (palert_target->MMSI == g_MMSI_Props_Array[i]->MMSI) {
        if (pAISMOBRoute)
          gFrame->UpdateAISMOBRoute(palert_target.get());
        else
          gFrame->ActivateAISMOBRoute(palert_target.get());
        break;
      }
    }
  }
}
