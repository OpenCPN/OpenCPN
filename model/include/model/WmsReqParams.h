#ifndef _WMSREQPARAMS__
#define _WMSREQPARAMS__
struct WmsReqParams {
  int w;
  int h;

  double lonSW, latSW, lonNE, latNE;

  int hitcount;

  struct mg_connection* c; //mongoose connection to use for sending the reply post rendering

};
#endif
