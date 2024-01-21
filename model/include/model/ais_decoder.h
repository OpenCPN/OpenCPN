/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
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
 ***************************************************************************
 */

#ifndef _AIS_DECODER_H__
#define _AIS_DECODER_H__

#include <map>
#include <unordered_map>
#include <memory>
#include <vector>

#include <wx/datetime.h>
#include <wx/event.h>
#include <wx/string.h>

#include "ais_bitstring.h"
#include "ais_defs.h"
#include "ais_target_data.h"
#include "comm_navmsg.h"
#include "observable_evtvar.h"
#include "ocpn_types.h"
#include "rapidjson/fwd.h"
#include "select.h"
#include "track.h"

class AisDecoder;  // forward

// AISTargetAlertDialog in gui layer
extern wxEvtHandler* g_pais_alert_dialog_active;

extern Select* pSelectAIS;
extern wxString AISTargetNameFileName;
extern AisDecoder *g_pAIS;

enum AISAudioSoundType {
  AISAUDIO_NONE,
  AISAUDIO_CPA,
  AISAUDIO_SART,
  AISAUDIO_DSC
};

class MmsiProperties {
public:
  MmsiProperties(){};
  MmsiProperties(int mmsi) {
    Init();
    MMSI = mmsi;
  }
  MmsiProperties(wxString &spec);

  ~MmsiProperties();

  wxString Serialize();

  void Init(void);
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
        : confirm_stop_track([]() { return true; } ),
          get_target_mmsi([]() { return 0; })  {}
};

class AisDecoder : public wxEvtHandler {
public:
  AisDecoder(AisDecoderCallbacks callbacks);

  ~AisDecoder(void);

  AisError DecodeN0183(const wxString &str);
  std::unordered_map<int, std::shared_ptr <AisTargetData>> &GetTargetList(void) {
    return AISTargetList;
  }
  std::unordered_map<int, std::shared_ptr <AisTargetData>> &GetAreaNoticeSourcesList(void) {
    return AIS_AreaNotice_Sources;
  }
  std::shared_ptr<AisTargetData> Get_Target_Data_From_MMSI(int mmsi);
  int GetNumTargets(void) { return m_n_targets; }
  bool IsAISSuppressed(void) { return m_bSuppressed; }
  bool IsAISAlertGeneral(void) { return m_bGeneralAlert; }
  AisError DecodeSingleVDO(const wxString &str, GenericPosDatEx *pos,
                            wxString *acc);
  void DeletePersistentTrack(Track *track);
  std::map<int, Track *> m_persistent_tracks;
  bool AIS_AlertPlaying(void) { return m_bAIS_AlertPlaying; };

  /**
   * Notified when AIS user dialogs should update. Event contains a
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
  void OnActivate(wxActivateEvent &event);
  void OnTimerAIS(wxTimerEvent &event);
  void OnTimerDSC(wxTimerEvent &event);

  bool NMEACheckSumOK(const wxString &str);
  bool Parse_VDXBitstring(AisBitstring *bstr, std::shared_ptr<AisTargetData> ptd);
  void UpdateAllCPA(void);
  void UpdateOneCPA(AisTargetData *ptarget);
  void UpdateAllAlarms(void);
  void UpdateAllTracks(void);
  void UpdateOneTrack(AisTargetData *ptarget);
  void BuildERIShipTypeHash(void);
  std::shared_ptr<AisTargetData> ProcessDSx(const wxString &str, bool b_take_dsc = false);

  wxString DecodeDSEExpansionCharacters(wxString dseData);
  void getAISTarget(long mmsi, std::shared_ptr<AisTargetData> &pTargetData,
                    std::shared_ptr<AisTargetData> &pStaleTarget, bool &bnewtarget,
                    int &last_report_ticks, wxDateTime &now);
  void getMmsiProperties(std::shared_ptr<AisTargetData> &pTargetData);
  void handleUpdate(std::shared_ptr<AisTargetData> pTargetData, bool bnewtarget,
                    const rapidjson::Value &update);
  void updateItem(std::shared_ptr<AisTargetData> pTargetData, bool bnewtarget,
                  const rapidjson::Value &item, wxString &sfixtime) const;
  void CommitAISTarget( std::shared_ptr<AisTargetData> pTargetData,
                        const wxString &str, bool message_valid,
                        bool new_target);
  void InitCommListeners(void);
  bool HandleN0183_AIS( std::shared_ptr <const Nmea0183Msg> n0183_msg );
  void HandleSignalK(std::shared_ptr<const SignalkMsg> sK_msg);

  bool HandleN2K_129038( std::shared_ptr<const Nmea2000Msg> n2k_msg );
  bool HandleN2K_129039( std::shared_ptr<const Nmea2000Msg> n2k_msg );
  bool HandleN2K_129041( std::shared_ptr<const Nmea2000Msg> n2k_msg );
  bool HandleN2K_129794( std::shared_ptr<const Nmea2000Msg> n2k_msg );
  bool HandleN2K_129809( std::shared_ptr<const Nmea2000Msg> n2k_msg );
  bool HandleN2K_129810( std::shared_ptr<const Nmea2000Msg> n2k_msg );
  bool HandleN2K_129793( std::shared_ptr<const Nmea2000Msg> n2k_msg );

  wxString m_signalk_selfid;
  std::unordered_map<int, std::shared_ptr<AisTargetData>> AISTargetList;
  std::unordered_map<int, std::shared_ptr<AisTargetData>> AIS_AreaNotice_Sources;
  AIS_Target_Name_Hash *AISTargetNamesC;
  AIS_Target_Name_Hash *AISTargetNamesNC;

  ObservableListener listener_N0183_VDM;
  ObservableListener listener_N0183_FRPOS;
  ObservableListener listener_N0183_CDDSC;
  ObservableListener listener_N0183_CDDSE;
  ObservableListener listener_N0183_TLL;
  ObservableListener listener_N0183_TTM;
  ObservableListener listener_N0183_OSD;
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

#endif //  _AIS_DECODER_H__
