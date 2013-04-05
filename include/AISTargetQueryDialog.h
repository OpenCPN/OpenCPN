#ifndef __AISTARGETQUERYDIALOG_H__
#define __AISTARGETQUERYDIALOG_H__

#include <wx/dialog.h>

#include "ocpn_types.h"

class wxHtmlWindow;

class AISTargetQueryDialog: public wxDialog
{
DECLARE_CLASS( AISTargetQueryDialog )
DECLARE_EVENT_TABLE()
public:

      /// Constructors

      AISTargetQueryDialog( );
      AISTargetQueryDialog( wxWindow* parent,
            wxWindowID id = wxID_ANY,
            const wxString& caption = _("Object Query"),
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

      ~AISTargetQueryDialog( );
      /// Initialise our variables
      void Init();

      /// Creation
      bool Create( wxWindow* parent,
            wxWindowID id = wxID_ANY,
            const wxString& caption = _("Object Query"),
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

      void OnClose(wxCloseEvent& event);
      void OnIdOKClick( wxCommandEvent& event );
      void OnMove( wxMoveEvent& event );

      void CreateControls();

      void SetText(wxString &text_string);
      void SetColorScheme(ColorScheme cs);

      void UpdateText(void);
      void SetMMSI(int mmsi){ m_MMSI = mmsi; }
      int  GetMMSI(void){ return m_MMSI; }

      //    Data
      int               m_MMSI;
      wxHtmlWindow     *m_pQueryTextCtl;
      ColorScheme       m_colorscheme;
      wxBoxSizer        *m_pboxSizer;
      int               m_nl;
      wxButton          *m_okButton;
};

#endif
