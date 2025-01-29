#ifndef _WMSREQPARAMS__
#define _WMSREQPARAMS__
struct WmsReqParams {
  int w; //w and h is the desired image size in px (requested by the browser component such as openlayers
  int h;
  double lonSW, latSW, lonNE, latNE;
  std::string color = "DAY"; //becomes an enum later on when changing later on, values: DEF = 0, DAY = 1, DUSK = 2, NIGHT = 3, NIGHTRED = 4;
  int hitcount;
  struct mg_connection* c; //mongoose connection to use for sending the reply back after rendering
};
#endif
