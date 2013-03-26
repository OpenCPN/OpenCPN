#ifndef __TTYSCROLL_H__
#define __TTYSCROLL_H__

#include <wx/scrolwin.h>

//    Scrolled TTY-like window for logging, etc....
class TTYScroll : public wxScrolledWindow
{
      public:
            TTYScroll(wxWindow *parent, int n_lines);
            virtual ~TTYScroll();
            virtual void OnDraw(wxDC& dc);
            virtual void Add(wxString &line);
            void OnSize(wxSizeEvent& event);
            void Pause(bool pause) { bpause = pause; }

      protected:

            wxCoord m_hLine;        // the height of one line on screen
            size_t m_nLines;        // the number of lines we draw

            wxArrayString     *m_plineArray;
            bool               bpause;

};

#endif
