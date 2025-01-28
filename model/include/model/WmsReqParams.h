#ifndef _WMSREQPARAMS__
#define _WMSREQPARAMS__
struct WmsReqParams {
  int w;
  int h;

  double lonSW, latSW, lonNE, latNE;

  int color = 0; //BSB_Color_Capability DEF=0, DAY=1, DUSK=2,  NIGHT=3, NIGHTRED=4,
   

  int hitcount;

  struct mg_connection* c; //mongoose connection to use for sending the reply post rendering

};
#endif
