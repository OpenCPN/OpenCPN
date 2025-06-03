// TBD

#ifndef CONNECT_NEW_DLG__
#define CONNECT_NEW_DLG__

#include <functional>

#include <wx/panel.h>
#include <wx/scrolwin.h>
#include "model/conn_params.h"
#include "observable_evtvar.h"

class ConnectionsDlg : public wxPanel {
public:
  ConnectionsDlg(wxWindow* parent,
                 const std::vector<ConnectionParams*>& connections);

  /**
   * Traverse root's children and invoke Apply if they implement ApplyCancel
   */
  void ApplySettings();

  /**
   * Traverse root's children and invoke Cancel if they implement ApplyCancel
   */
  void CancelSettings();

  void OnResize(const wxSize& size);

private:
  void DoApply(wxWindow* root);
  void DoCancel(wxWindow* root);

  const std::vector<ConnectionParams*>& m_connections;
  std::function<void()> m_on_exit;
  EventVar m_evt_add_connection;
};

#endif  //  CONNECT_NEW_DLG__
