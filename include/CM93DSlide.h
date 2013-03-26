#ifndef __CM93DSLIDE_H__
#define __CM93DSLIDE_H__

#include <wx/dialog.h>

class CM93DSlide: public wxDialog
{
public:
    CM93DSlide( wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue,
            const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
            long style = 0, const wxString& title = _T("") );

    ~CM93DSlide( void );

    void Init( void );
    bool Create( wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue,
            const wxPoint& pos, const wxSize& size, long style, const wxString& title );

    void OnCancelClick( wxCommandEvent& event );
    void OnMove( wxMoveEvent& event );
    void OnChangeValue( wxScrollEvent& event );
    void OnClose( wxCloseEvent& event );

    wxSlider *m_pCM93DetailSlider;
    wxWindow *m_pparent;

DECLARE_EVENT_TABLE()
};

#endif
