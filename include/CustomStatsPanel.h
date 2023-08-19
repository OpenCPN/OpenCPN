#include "ocpn_frame.h"

class ChartCanvas;
class MyFrame;
class CustomStatsPanel : public wxFrame {
public:
  CustomStatsPanel(ChartCanvas *parent, int orientation = wxHORIZONTAL,
         float size_factor = 1.0, wxWindowID id = wxID_ANY,
         const wxPoint &pos = wxDefaultPosition,
         const wxSize &size = wxDefaultSize, long style = 0,
         const wxString &name = wxPanelNameStr);

  ~CustomStatsPanel();
  //void UpdateDynamicValues();
  int GetOrientation() { return m_orientation; }
  wxStaticText *statsTextControl(const wxString &label);
  void SetBestPosition();
  void updateCursorPositions(double cursorLat, double cursorLon);
  void updateShipDetailsAndPositions(MyFrame* parentFrame, double g_lat, double g_lon,
                                     double g_sog, double g_cog);

private:
  void Init(void);
  //void CreateControls();
  ChartCanvas *m_parentCanvas;
  int m_orientation;
  float m_scaleFactor;

  wxStaticText *shipLon;
  wxStaticText *shipLat;
  wxStaticText *cursorLon;
  wxStaticText *cursorLat;
  wxStaticText *SOG;
  wxStaticText *COG;
  DECLARE_EVENT_TABLE()
  void OnPaint(wxPaintEvent &event);
};
