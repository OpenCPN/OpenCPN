#ifdef INITIALIZE_RADAR

PLUGIN_BEGIN_NAMESPACE

static const NetworkAddress dataBR24(236, 6, 7, 8, 6678);
static const NetworkAddress reportBR24(236, 6, 7, 9, 6679);
static const NetworkAddress sendBR24(236, 6, 7, 10, 6680);

#define RANGE_METRIC_RT_BR24 \
  { 50, 75, 100, 250, 500, 750, 1000, 1500, 2000, 3000, 4000, 6000, 8000, 12000, 16000, 24000, 3600, 48000 }
#define RANGE_MIXED_RT_BR24                                                                                                  \
  {                                                                                                                          \
    50, 75, 100, 1852 / 8, 1852 / 4, 1852 / 2, 1852 * 3 / 4, 1852 * 1, 1852 * 3 / 2, 1852 * 2, 1852 * 3, 1852 * 4, 1852 * 6, \
        1852 * 8, 1852 * 12, 1852 * 16, 1852 * 24                                                                            \
  }
#define RANGE_NAUTIC_RT_BR24                                                                                                \
  {                                                                                                                         \
    1852 / 32, 1852 / 16, 1852 / 8, 1852 / 4, 1852 / 2, 1852 * 3 / 4, 1852 * 1, 1852 * 3 / 2, 1852 * 2, 1852 * 3, 1852 * 4, \
        1852 * 6, 1852 * 8, 1852 * 12, 1852 * 16, 1852 * 24                                                                 \
  }

PLUGIN_END_NAMESPACE

#endif

#include "NavicoCommon.h"

DEFINE_RADAR(RT_BR24,                                               /* Type */
             wxT("Navico BR24"),                                    /* Name */
             NAVICO_SPOKES,                                         /* Spokes */
             NAVICO_SPOKE_LEN,                                      /* Spoke length */
             NavicoControlsDialog(RT_BR24),                         /* Controls class */
             NavicoReceive(pi, ri, reportBR24, dataBR24, sendBR24), /* Receive class */
             NavicoControl(),                                       /* Send/Control class */
             RO_SINGLE /* This type only has a single radar and does not need locating */
)
