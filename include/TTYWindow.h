#ifndef __TTYWINDOW_H__
#define __TTYWINDOW_H__

#include <wx/dialog.h>
#include <wx/bitmap.h>

class TTYScroll;
class wxButton;

class TTYWindow : public wxDialog
{
      DECLARE_DYNAMIC_CLASS( TTYWindow )
      DECLARE_EVENT_TABLE()

      public:
            TTYWindow();
            TTYWindow(wxWindow *parent, int n_lines);
            ~TTYWindow();

             void Add(wxString &line);
             void OnCloseWindow(wxCloseEvent& event);
             void Close();
             void OnSize( wxSizeEvent& event );
             void OnMove( wxMoveEvent& event );
             void OnPauseClick( wxCommandEvent& event );

      protected:
            void CreateLegendBitmap();
            TTYScroll   *m_pScroll;
            wxButton    *m_buttonPause;
            bool        bpause;
            wxBitmap    m_bm_legend;
};

#endif
