class ChartCanvas;
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

private:
  void Init(void);
  //void CreateControls();
  ChartCanvas *m_parentCanvas;
  int m_orientation;
  float m_scaleFactor;

  wxStaticText *shipLon;
  wxStaticText *shipLat;
  wxStaticText *SOG;
  wxStaticText *COG;
  DECLARE_EVENT_TABLE()
  void OnPaint(wxPaintEvent &event);
};
