#ifdef INITIALIZE_RADAR

PLUGIN_BEGIN_NAMESPACE

// Note that the order of the ports is different on A and B. I guess someone
// in Navico just didn't realize this. Or it is just a bit of obfuscation.
static const NetworkAddress data4G_A(236, 6, 7, 8, 6678);
static const NetworkAddress report4G_A(236, 6, 7, 9, 6679);
static const NetworkAddress send4G_A(236, 6, 7, 10, 6680);

#define RANGE_METRIC_RT_4GA \
  { 50, 75, 100, 250, 500, 750, 1000, 1500, 2000, 3000, 4000, 6000, 8000, 12000, 16000, 24000, 36000, 48000 }
#define RANGE_MIXED_RT_4GA                                                                                                   \
  {                                                                                                                          \
    50, 75, 100, 1852 / 8, 1852 / 4, 1852 / 2, 1852 * 3 / 4, 1852 * 1, 1852 * 3 / 2, 1852 * 2, 1852 * 3, 1852 * 4, 1852 * 6, \
        1852 * 8, 1852 * 12, 1852 * 16, 1852 * 24, 1852 * 36                                                                 \
  }
#define RANGE_NAUTIC_RT_4GA                                                                                                 \
  {                                                                                                                         \
    1852 / 32, 1852 / 16, 1852 / 8, 1852 / 4, 1852 / 2, 1852 * 3 / 4, 1852 * 1, 1852 * 3 / 2, 1852 * 2, 1852 * 3, 1852 * 4, \
        1852 * 6, 1852 * 8, 1852 * 12, 1852 * 16, 1852 * 24, 1852 * 36                                                      \
  }

PLUGIN_END_NAMESPACE

#endif

#include "NavicoCommon.h"

DEFINE_RADAR(RT_4GA,                                                /* Type */
             wxT("Navico 4G A"),                                    /* Name */
             NAVICO_SPOKES,                                         /* Spokes */
             NAVICO_SPOKE_LEN,                                      /* Spoke length (max) */
             NavicoControlsDialog(RT_4GA),                          /* ControlsDialog class constructor */
             NavicoReceive(pi, ri, report4G_A, data4G_A, send4G_A), /* Receive class constructor */
             NavicoControl(),                                       /* Send/Control class constructor */
             RO_PRIMARY                                             /* This type has two radars and this is the first */
)
