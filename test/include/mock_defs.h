#ifndef _UNITTEST_MOCK_DEFS
#define _UNITTEST_MOCK_DEFS

#include "select_item.h"
#include "vector2D.h"

class AISTargetAlertDialog;

class Route;

class DataStream;

class OCPN_DataStreamEvent: public wxEvent {
public:
  OCPN_DataStreamEvent() {}
  OCPN_DataStreamEvent(int a, int b) {}
  wxEvent* Clone() const {
    OCPN_DataStreamEvent* ev = new OCPN_DataStreamEvent(*this);
    ev->m_NMEAstring = this->m_NMEAstring;
    ev->m_pDataStream = this->m_pDataStream;
    return ev;
  }
  void SetNMEAString(std::string s) { m_NMEAstring = s; }
  void SetStream(DataStream* ds) { m_pDataStream = ds; }

private:
  std::string m_NMEAstring;
  DataStream *m_pDataStream;
};

class Multiplexer:  public wxEvtHandler {
public:
  Multiplexer() {}
  ~Multiplexer() {}
};

class RoutePoint {
public:
  RoutePoint(double x, double y , wxString const& s1, wxString const& s2,
             wxString const& s3, bool b) {}
  RoutePoint(double x, double y , wxString const& s1, wxString s2,
             wxString s3) {}
  bool m_bIsolatedMark;

};

class TrackPoint {
public:
  TrackPoint() {};
  virtual ~TrackPoint() {}
  double m_lat;
  double m_lon;
};


class Track {
public:
  Track() {};
  virtual ~Track() {}

  TrackPoint *GetLastPoint()  { return 0; };
  void SetName(wxString name) {}
  TrackPoint *AddNewPoint(vector2D point, wxDateTime time) { return new TrackPoint(); }
};



#define SELTYPE_AISTARGET 0x0040
class Select {

public:
  Select() {}
  ~Select() {}

  bool AddSelectableRoutePoint(float slat, float slon,
                               RoutePoint *pRoutePointAdd){ return true; }

  bool AddSelectableTrackSegment(float slat1, float slon1, float slat2,
                                 float slon2, TrackPoint *pTrackPointAdd1,
                                 TrackPoint *pTrackPointAdd2, Track *pTrack)
                                 { return true; }

  bool DeleteSelectablePoint(void *data, int SeltypeToDelete) { return true; }

  SelectItem *AddSelectablePoint(float slat, float slon, const void *data,
                                 int fseltype) { return new SelectItem(); }

};


#endif  //  _UNITTEST_MOCK_DEFS
