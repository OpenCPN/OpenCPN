#ifdef INITIALIZE_RADAR

PLUGIN_BEGIN_NAMESPACE

// Note that the order of the ports is different on B and B. I guess someone
// in Navico just didn't realize this. Or it is just a bit of obfuscation.
static const NetworkAddress data4G_B(236, 6, 7, 13, 6657);
static const NetworkAddress send4G_B(236, 6, 7, 14, 6658);
static const NetworkAddress report4G_B(236, 6, 7, 15, 6659);

#define RANGE_METRIC_RT_4GB \
  { 50, 75, 100, 250, 500, 750, 1000, 1500, 2000, 3000, 4000, 6000, 8000, 12000, 16000, 24000, 36000, 48000 }
#define RANGE_MIXED_RT_4GB                                                                                                   \
  {                                                                                                                          \
    50, 75, 100, 1852 / 8, 1852 / 4, 1852 / 2, 1852 * 3 / 4, 1852 * 1, 1852 * 3 / 2, 1852 * 2, 1852 * 3, 1852 * 4, 1852 * 6, \
        1852 * 8, 1852 * 12, 1852 * 16, 1852 * 24, 1852 * 36                                                                 \
  }
#define RANGE_NAUTIC_RT_4GB                                                                                                 \
  {                                                                                                                         \
    1852 / 32, 1852 / 16, 1852 / 8, 1852 / 4, 1852 / 2, 1852 * 3 / 4, 1852 * 1, 1852 * 3 / 2, 1852 * 2, 1852 * 3, 1852 * 4, \
        1852 * 6, 1852 * 8, 1852 * 12, 1852 * 16, 1852 * 24, 1852 * 36                                                      \
  }

PLUGIN_END_NAMESPACE

#endif

#include "NavicoCommon.h"

// 4G has 2048 spokes of exactly 1024 pixels of 4 bits each, packed in 512 bytes

DEFINE_RADAR(RT_4GB,                                                /* Type */
             wxT("Navico 4G B"),                                    /* Name */
             NAVICO_SPOKES,                                         /* Spokes */
             NAVICO_SPOKE_LEN,                                      /* Spoke length (max) */
             NavicoControlsDialog(RT_4GB),                          /* ControlsDialog class constructor */
             NavicoReceive(pi, ri, report4G_B, data4G_B, send4G_B), /* Receive class constructor */
             NavicoControl(),                                       /* Send/Control class constructor */
             RO_SECONDARY                                           /* This type has two radars and this is the second */
)
