#ifndef __S57QUERYDIALOG_H__
#define __S57QUERYDIALOG_H__

#include <wx/dialog.h>

class wxHtmlWindow;

class S57QueryDialog: public wxDialog
{
      DECLARE_CLASS( S57QueryDialog )
                  DECLARE_EVENT_TABLE()
      public:

      /// Constructors

            S57QueryDialog( );
            S57QueryDialog( wxWindow* parent,
                            wxWindowID id = wxID_ANY,
                            const wxString& caption = _("Object Query"),
                                        const wxPoint& pos = wxDefaultPosition,
                                        const wxSize& size = wxDefaultSize,
                                        long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

            ~S57QueryDialog( );
            void Init();

            bool Create( wxWindow* parent,
                         wxWindowID id = wxID_ANY,
                         const wxString& caption = _("Object Query"),
                                     const wxPoint& pos = wxDefaultPosition,
                                     const wxSize& size = wxDefaultSize,
                                     long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

            void SetColorScheme(void);

            void CreateControls();
            void OnSize(wxSizeEvent& event);
            void OnClose(wxCloseEvent& event);

      //    Overrides
            void OnPaint ( wxPaintEvent& event );

            void SetHTMLPage(wxString& page);

      //    Data
            wxHtmlWindow      *m_phtml;

};

#endif
