#ifndef __CHINFOWIN_H__
#define __CHINFOWIN_H__

#include <wx/window.h>
#include <wx/stattext.h>

class ChInfoWin: public wxWindow
{
public:
    ChInfoWin( wxWindow *parent );
    ~ChInfoWin();

    void SetString( wxString &s )
    {
        m_string = s;
    }
    void SetPosition( wxPoint pt )
    {
        m_position = pt;
    }
    void SetWinSize( wxSize sz )
    {
        m_size = sz;
    }
    void SetBitmap( void );
    void FitToChars( int char_width, int char_height );
    wxSize GetWinSize( void )
    {
        return m_size;
    }
    void OnPaint( wxPaintEvent& event );
    void OnEraseBackground( wxEraseEvent& event );

private:

    wxString m_string;
    wxSize m_size;
    wxPoint m_position;
    wxStaticText *m_pInfoTextCtl;

    DECLARE_EVENT_TABLE()
};


#endif
