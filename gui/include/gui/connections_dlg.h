// TBD

#ifndef CONNECT_NEW_DLG__
#define CONNECT_NEW_DLG__

#include <functional>

#include <wx/frame.h>
#include "model/conn_params.h"
#include "observable_evtvar.h"

class ConnectionsDlg : public wxPanel {
public:
  ConnectionsDlg(wxWindow* parent,
                 const std::vector<ConnectionParams*>& connections);

private:
  const std::vector<ConnectionParams*>& m_connections;
  std::function<void()> m_on_exit;
  EventVar m_evt_add_connection;
  ObsListener m_add_connection_lstnr;
};

#endif  //  CONNECT_NEW_DLG__
