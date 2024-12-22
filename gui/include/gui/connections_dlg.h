// TBD

#ifndef CONNECT_NEW_DLG__
#define CONNECT_NEW_DLG__

#include <functional>

#include <wx/frame.h>
#include "model/conn_params.h"

class ConnectionsDlg : public wxFrame {
public:
  ConnectionsDlg(wxWindow* parent,
                 const std::vector<ConnectionParams*>& connections,
                 std::function<void()> on_exit);

private:
  const std::vector<ConnectionParams*>& m_connections;
  std::function<void()> m_on_exit;
};

#endif  //  CONNECT_NEW_DLG__
