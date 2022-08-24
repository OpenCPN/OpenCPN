#include "AIS_Target_Data.h"
#include "OCPN_Sound.h"

class AisInfoGui {
  void ShowAisInfo(AIS_Target_Data* palert_target);
  bool AIS_AlertPlaying(void) { return m_bAIS_AlertPlaying; };

  bool m_bAIS_Audio_Alert_On;
  bool  m_bAIS_AlertPlaying;
  OcpnSound* m_AIS_Sound;
};
