#ifdef INITIALIZE_RADAR

PLUGIN_BEGIN_NAMESPACE

static const NetworkAddress data3G(236, 6, 7, 8, 6678);
static const NetworkAddress report3G(236, 6, 7, 9, 6679);
static const NetworkAddress send3G(236, 6, 7, 10, 6680);

#define RANGE_METRIC_RT_3G \
  { 50, 75, 100, 250, 500, 750, 1000, 1500, 2000, 3000, 4000, 6000, 8000, 12000, 16000, 24000, 36000, 48000 }
#define RANGE_MIXED_RT_3G                                                                                                    \
  {                                                                                                                          \
    50, 75, 100, 1852 / 8, 1852 / 4, 1852 / 2, 1852 * 3 / 4, 1852 * 1, 1852 * 3 / 2, 1852 * 2, 1852 * 3, 1852 * 4, 1852 * 6, \
        1852 * 8, 1852 * 12, 1852 * 16, 1852 * 24                                                                            \
  }
#define RANGE_NAUTIC_RT_3G                                                                                                  \
  {                                                                                                                         \
    1852 / 32, 1852 / 16, 1852 / 8, 1852 / 4, 1852 / 2, 1852 * 3 / 4, 1852 * 1, 1852 * 3 / 2, 1852 * 2, 1852 * 3, 1852 * 4, \
        1852 * 6, 1852 * 8, 1852 * 12, 1852 * 16, 1852 * 24                                                                 \
  }

PLUGIN_END_NAMESPACE

#endif

#include "NavicoCommon.h"

DEFINE_RADAR(RT_3G,                                           /* Type */
             wxT("Navico 3G"),                                /* Name */
             NAVICO_SPOKES,                                   /* Spokes */
             NAVICO_SPOKE_LEN,                                /* Spoke length (max) */
             NavicoControlsDialog(RT_3G),                     /* ControlsDialog class constructor */
             NavicoReceive(pi, ri, report3G, data3G, send3G), /* Receive class constructor */
             NavicoControl(),                                 /* Send/Control class constructor */
             RO_SINGLE                                        /* This type only has a single radar and does not need locating */
)
