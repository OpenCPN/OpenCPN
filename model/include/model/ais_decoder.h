/**************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

/**
 * \file
 *
 * Class AisDecoder and helpers.
 */

#ifndef AIS_DECODER_H_
#define AIS_DECODER_H_

#include <map>
#include <unordered_map>
#include <memory>
#include <vector>

#include <wx/datetime.h>
#include <wx/event.h>
#include <wx/string.h>

#include "rapidjson/fwd.h"
#include "model/ais_bitstring.h"
#include "model/ais_defs.h"
#include "model/ais_target_data.h"
#include "model/comm_navmsg.h"
#include "model/ocpn_types.h"
#include "model/select.h"
#include "model/track.h"
#include "observable_evtvar.h"

using N0183MsgPtr = std::shared_ptr<const Nmea0183Msg>;
using N2000MsgPtr = std::shared_ptr<const Nmea2000Msg>;
using SignalKMsgPtr = std::shared_ptr<const SignalkMsg>;
using NavMsgPtr = std::shared_ptr<const NavMsg>;

class AisDecoder;             // forward
class ArrayOfMmsiProperties;  // forward

// AISTargetAlertDialog in gui layer
extern wxEvtHandler *g_pais_alert_dialog_active;

extern Select *pSelectAIS;
extern wxString AISTargetNameFileName;
extern AisDecoder *g_pAIS;
extern ArrayOfMmsiProperties g_MMSI_Props_Array;
extern unsigned g_OwnShipmmsi;

enum AISAudioSoundType {
  AISAUDIO_NONE,
  AISAUDIO_CPA,
  AISAUDIO_SART,
  AISAUDIO_DSC
};

/** Process incoming AIS messages. */
class MmsiProperties {
public:
  MmsiProperties() = default;

  explicit MmsiProperties(int mmsi) {
    Init();
    MMSI = mmsi;
  }
  explicit MmsiProperties(wxString &spec);

  ~MmsiProperties();

  wxString Serialize();

  void Init();
  int MMSI;
  int TrackType;
  bool m_bignore;
  bool m_bMOB;
  bool m_bVDM;
  bool m_bFollower;
  bool m_bPersistentTrack;
  wxString m_ShipName;
};

WX_DEFINE_ARRAY_PTR(MmsiProperties *, ArrayOfMmsiProperties);

struct AisDecoderCallbacks {
  std::function<bool()> confirm_stop_track;
  std::function<int()> get_target_mmsi;
  AisDecoderCallbacks()
      : confirm_stop_track([]() { return true; }),
        get_target_mmsi([]() { return 0; }) {}
};

class AisDecoder : public wxEvtHandler {
public:
  explicit AisDecoder(const AisDecoderCallbacks &callbacks);

  ~AisDecoder() override;

  AisError DecodeN0183(const wxString &str);
  std::unordered_map<int, std::shared_ptr<AisTargetData>> &GetTargetList() {
    return AISTargetList;
  }
  std::unordered_map<int, std::shared_ptr<AisTargetData>> &
  GetAreaNoticeSourcesList() {
    return AIS_AreaNotice_Sources;
  }
  std::shared_ptr<AisTargetData> Get_Target_Data_From_MMSI(unsigned mmsi);
  int GetNumTargets() const { return m_n_targets; }
  bool IsAISSuppressed() const { return m_bSuppressed; }
  bool IsAISAlertGeneral() const { return m_bGeneralAlert; }
  void UpdateMMSItoNameFile(const wxString &mmsi, const wxString &name);
  wxString GetMMSItoNameEntry(const wxString &mmsi);
  AisError DecodeSingleVDO(const wxString &str, GenericPosDatEx *pos,
                           wxString *acc);
  void DeletePersistentTrack(const Track *track);
  std::map<int, Track *> m_persistent_tracks;
  bool AIS_AlertPlaying() const { return m_bAIS_AlertPlaying; };

  /**
   * Notified when AIS user dialogs should update. Event contains an
   * AIS_Target_data pointer.
   */
  EventVar info_update;

  /** Notified when gFrame->TouchAISActive() should be invoked */
  EventVar touch_state;

  /** Notified when new AIS wp is created. Contains a RoutePoint* pointer. */
  EventVar new_ais_wp;

  /** Notified on new track creation. Contains a Track* pointer. */
  EventVar new_track;

  /** Notified when about to delete track. Contains a MmsiProperties* ptr */
  EventVar delete_track;

  /** A JSON message should be sent. Contains a AisTargetData* pointer. */
  EventVar plugin_msg;

private:
  void OnTimerAIS(wxTimerEvent &event);
  void OnTimerDSC(wxTimerEvent &event);

  bool NMEACheckSumOK(const wxString &str);
  void UpdateAllCPA();
  void UpdateOneCPA(AisTargetData *ptarget);
  void UpdateAllAlarms();
  void UpdateAllTracks();
  void UpdateOneTrack(AisTargetData *ptarget);
  std::shared_ptr<AisTargetData> ProcessDSx(const wxString &str,
                                            bool b_take_dsc = false);

  void getAISTarget(long mmsi, std::shared_ptr<AisTargetData> &pTargetData,
                    std::shared_ptr<AisTargetData> &pStaleTarget,
                    bool &bnewtarget, int &last_report_ticks, wxDateTime &now);
  void handleUpdate(const std::shared_ptr<AisTargetData> &pTargetData,
                    bool bnewtarget, const rapidjson::Value &update);
  void updateItem(const std::shared_ptr<AisTargetData> &pTargetData,
                  bool bnewtarget, const rapidjson::Value &item,
                  wxString &sfixtime) const;
  void CommitAISTarget(const std::shared_ptr<AisTargetData> &pTargetData,
                       const wxString &str, bool message_valid,
                       bool new_target);
  void InitCommListeners();
  bool HandleN0183_AIS(const N0183MsgPtr &n0183_msg);
  void HandleSignalK(const SignalKMsgPtr &sK_msg);

  bool HandleN2K_129038(const N2000MsgPtr &n2k_msg);
  bool HandleN2K_129039(const N2000MsgPtr &n2k_msg);
  bool HandleN2K_129041(const N2000MsgPtr &n2k_msg);
  bool HandleN2K_129794(const N2000MsgPtr &n2k_msg);
  bool HandleN2K_129809(const N2000MsgPtr &n2k_msg);
  bool HandleN2K_129810(const N2000MsgPtr &n2k_msg);
  bool HandleN2K_129793(const N2000MsgPtr &n2k_msg);

  wxString m_signalk_selfid;
  std::unordered_map<int, std::shared_ptr<AisTargetData>> AISTargetList;
  std::unordered_map<int, std::shared_ptr<AisTargetData>>
      AIS_AreaNotice_Sources;
  AIS_Target_Name_Hash *AISTargetNamesC;
  AIS_Target_Name_Hash *AISTargetNamesNC;

  ObservableListener listener_N0183_VDM;
  ObservableListener listener_N0183_FRPOS;
  ObservableListener listener_N0183_CDDSC;
  ObservableListener listener_N0183_CDDSE;
  ObservableListener listener_N0183_TLL;
  ObservableListener listener_N0183_TTM;
  ObservableListener listener_N0183_OSD;
  ObservableListener listener_N0183_WPL;
  ObservableListener listener_SignalK;

  ObservableListener listener_N2K_129038;
  ObservableListener listener_N2K_129039;
  ObservableListener listener_N2K_129041;
  ObservableListener listener_N2K_129794;
  ObservableListener listener_N2K_129809;
  ObservableListener listener_N2K_129810;
  ObservableListener listener_N2K_129793;

  bool m_busy;
  wxTimer TimerAIS;
  wxFrame *m_parent_frame;
  AisDecoderCallbacks m_callbacks;

  int nsentences;
  int isentence;
  wxString sentence_accumulator;
  bool m_OK;

  std::shared_ptr<AisTargetData> m_pLatestTargetData;

  bool m_bAIS_Audio_Alert_On;
  wxTimer m_AIS_Audio_Alert_Timer;
  int m_n_targets;
  bool m_bSuppressed;
  bool m_bGeneralAlert;
  std::shared_ptr<AisTargetData> m_ptentative_dsctarget;
  wxTimer m_dsc_timer;
  wxString m_dsc_last_string;
  std::vector<int> m_MMSI_MismatchVec;

  bool m_bAIS_AlertPlaying;
  DECLARE_EVENT_TABLE()
};

#endif  //  AIS_DECODER_H_
