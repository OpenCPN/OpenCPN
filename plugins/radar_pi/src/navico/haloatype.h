#ifdef INITIALIZE_RADAR

PLUGIN_BEGIN_NAMESPACE

// HALO used dynamic mcast address + port, so don't even try anything.
// Let the NavicoLocate thread find it for us.
// in Navico just didn't realize this. Or it is just a bit of obfuscation.
static const NetworkAddress dataHalo_A(0, 0, 0, 0, 0);
static const NetworkAddress reportHalo_A(0, 0, 0, 0, 0);
static const NetworkAddress sendHalo_A(0, 0, 0, 0, 0);

#define RANGE_METRIC_RT_HaloA \
  { 50, 75, 100, 250, 500, 750, 1000, 1500, 2000, 3000, 4000, 6000, 8000, 12000, 16000, 24000, 36000, 48000 }
#define RANGE_MIXED_RT_HaloA                                                                                                 \
  {                                                                                                                          \
    50, 75, 100, 1852 / 8, 1852 / 4, 1852 / 2, 1852 * 3 / 4, 1852 * 1, 1852 * 3 / 2, 1852 * 2, 1852 * 3, 1852 * 4, 1852 * 6, \
        1852 * 8, 1852 * 12, 1852 * 16, 1852 * 24, 1852 * 36, 1852 * 48, 1852 * 64, 1852 * 72                                \
  }
#define RANGE_NAUTIC_RT_HaloA                                                                                               \
  {                                                                                                                         \
    1852 / 32, 1852 / 16, 1852 / 8, 1852 / 4, 1852 / 2, 1852 * 3 / 4, 1852 * 1, 1852 * 3 / 2, 1852 * 2, 1852 * 3, 1852 * 4, \
        1852 * 6, 1852 * 8, 1852 * 12, 1852 * 16, 1852 * 24, 1852 * 36, 1852 * 48, 1852 * 64, 1852 * 72                     \
  }

PLUGIN_END_NAMESPACE

#endif

#include "NavicoCommon.h"

DEFINE_RADAR(RT_HaloA,                                                    /* Type */
             wxT("Navico Halo A"),                                        /* Name */
             NAVICO_SPOKES,                                               /* Spokes */
             NAVICO_SPOKE_LEN,                                            /* Spoke length (max) */
             NavicoControlsDialog(RT_HaloA),                              /* ControlsDialog class constructor */
             NavicoReceive(pi, ri, reportHalo_A, dataHalo_A, sendHalo_A), /* Receive class constructor */
             NavicoControl(),                                             /* Send/Control class constructor */
             RO_PRIMARY                                                   /* This type has two radars and this is the first */
)
