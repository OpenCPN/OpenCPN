#ifndef __ROLLOVERWIN_H__
#define __ROLLOVERWIN_H__

#include <wx/window.h>
#include <wx/timer.h>
#include "ocpn_types.h"

//constants for rollovers fonts
enum
{
    AIS_ROLLOVER =1,
    LEG_ROLLOVER =2,
    TC_ROLLOVER  =3
};

class RolloverWin: public wxWindow
{
public:
    RolloverWin( wxWindow *parent, int timeout = -1 );
    ~RolloverWin();

    void OnPaint( wxPaintEvent& event );

    void SetColorScheme( ColorScheme cs );
    void SetString( wxString &s ) { m_string = s; }
    void SetPosition( wxPoint pt ) { m_position = pt; }
    void SetBitmap( int rollover );
    wxBitmap* GetBitmap() { return m_pbm; }
    void SetBestPosition( int x, int y, int off_x, int off_y, int rollover, wxSize parent_size );
    void OnTimer( wxTimerEvent& event );
    void OnMouseEvent( wxMouseEvent& event );
    void SetMousePropogation( int level ) { m_mmouse_propogate = level; }
    bool IsActive() { return isActive; }
    void IsActive( bool state ) { isActive = state; }

private:
    wxString m_string;
    wxSize m_size;
    wxPoint m_position;
    wxBitmap *m_pbm;
    wxTimer m_timer_timeout;
    int m_timeout_sec;
    int m_mmouse_propogate;
    bool isActive;
    wxFont *m_plabelFont;

DECLARE_EVENT_TABLE()
};


#endif
