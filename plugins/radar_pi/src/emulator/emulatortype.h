#ifdef INITIALIZE_RADAR

PLUGIN_BEGIN_NAMESPACE

PLUGIN_END_NAMESPACE

#endif

#define RANGE_METRIC_RT_EMULATOR \
  { 1000, 4000 }
#define RANGE_MIXED_RT_EMULATOR \
  { 1852 / 2, 1852 * 2 }
#define RANGE_NAUTIC_RT_EMULATOR \
  { 1852 / 2, 1852 * 2 }

// Emulator has 1440 spokes of exactly 768 bytes each, to emulate Garmin
#define EMULATOR_SPOKES 1440
#define EMULATOR_MAX_SPOKE_LEN 768

#if SPOKES_MAX < EMULATOR_SPOKES
#undef SPOKES_MAX
#define SPOKES_MAX EMULATOR_SPOKES
#endif
#if SPOKE_LEN_MAX < EMULATOR_MAX_SPOKE_LEN
#undef SPOKE_LEN_MAX
#define SPOKE_LEN_MAX EMULATOR_MAX_SPOKE_LEN
#endif

DEFINE_RADAR(RT_EMULATOR,             /* Type */
             wxT("Emulator"),         /* Name */
             EMULATOR_SPOKES,         /* Spokes */
             EMULATOR_MAX_SPOKE_LEN,  /* Spoke length */
             EmulatorControlsDialog,  /* Controls class */
             EmulatorReceive(pi, ri), /* Receive class */
             EmulatorControl,         /* Send/Control class */
             RO_SINGLE                /* This type only has a single radar and does not need locating */
)
