/****************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN iENCToolbar
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2017 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#include "toolbar.h"

//----------------------------------------------------------------------------------------------------------
//    iENCToolbar Specification
//----------------------------------------------------------------------------------------------------------

#define ID_DENSITY 20000
#define ID_RPLUS 20001
#define ID_RMINUS 20002

#define STATE_TIMER 3689

class iENCToolbar : public ocpnFloatingToolbarDialog {
public:
  iENCToolbar(wxWindow *parent, wxPoint position, long orient,
              float size_factor);
  ~iENCToolbar();

  void LoadToolBitmaps();
  void OnToolLeftClick(wxCommandEvent &event);
  void SetDensityToolBitmap(int nDensity);
  void SetRangeToolBitmap();

  void StateTimerEvent(wxTimerEvent &event);
  void SetColorScheme(ColorScheme cs);

  wxBitmap m_bmMinimum, m_bmStandard, m_bmAll, m_bmUStd, m_bmRPlus, m_bmRMinus;
  wxBitmap *m_pbmScratch;
  wxBitmap m_bmTemplate;

  wxToolBarToolBase *m_toolDensity;

  int m_nDensity;
  wxTimer m_state_timer;
  double m_range;
  wxFont *m_rangeFont;

#if 0
      void OnClose( wxCloseEvent& event );
      void OnWindowCreate( wxWindowCreateEvent& event );
      void OnToolLeftClick( wxCommandEvent& event );
      void MouseEvent( wxMouseEvent& event );
      void FadeTimerEvent( wxTimerEvent& event );
      bool IsToolbarShown() { return ( m_ptoolbar != 0 ); }
      float GetScaleFactor() { return m_sizefactor; }
      void SetGrabber( wxString icon_name );
      void DestroyTimerEvent( wxTimerEvent& event );

      void Realize();
      ocpnToolBarSimple *GetToolbar();
      ocpnToolBarSimple *CreateNewToolbar();

      void CreateConfigMenu();
      bool _toolbarConfigMenuUtil( int toolid, wxString tipString );

      void Submerge();
      void SubmergeToGrabber();
      bool isSubmergedToGrabber();
      void Surface();
      void SurfaceFromGrabber();
      void HideTooltip();
      void ShowTooltips();
      void EnableTooltips() { if(m_ptoolbar) m_ptoolbar->EnableTooltips(); }
      void DisableTooltips() { if(m_ptoolbar) m_ptoolbar->DisableTooltips(); }
      void UpdateRecoveryWindow(bool b_toolbarEnable);
      void EnableTool( int toolid, bool enable );
      void SetToolShortHelp( int toolid, const wxString& helpString );

      void DestroyToolBar();
      void ToggleOrientation();
      void MoveDialogInScreenCoords( wxPoint posn, wxPoint posn_old );
      void RePosition();
      void LockPosition(bool lock){ m_block = lock; }
      void SetColorScheme( ColorScheme cs );
      ColorScheme GetColorScheme(){ return m_cs; }
      bool CheckSurfaceRequest( wxMouseEvent &event );

      void SetGeometry(bool bAvoid, wxRect rectAvoid);
      long GetOrient() {
            return m_orient;
      }
      void RefreshFadeTimer();
      void SetAutoHideTimer(int time);
      void SetAutoHide( bool hide ){ m_bAutoHideToolbar = hide; }

      int GetDockX() {
            return m_dock_x;
      }
      int GetDockY() {
            return m_dock_y;
      }
      bool toolbarConfigChanged;
      GrabberWin *m_pRecoverwin;
      bool m_bnavgrabber;

      wxMenu  *m_FloatingToolbarConfigMenu;
#endif
private:
#if 0
      void DoFade( int value );

      bool  m_bsubmerged;
      bool  m_bsubmergedToGrabber;

      wxWindow *m_pparent;
      ocpnToolBarSimple *m_ptoolbar;
      wxBoxSizer *m_topSizer;

      GrabberWin *m_pGrabberwin;

      long m_orient;
      wxTimer m_fade_timer;
      int m_opacity;
      ColorScheme m_cs;

      wxPoint m_position;
      int m_dock_x;
      int m_dock_y;
      ocpnStyle::Style* m_style;
      bool m_block;

      bool m_marginsInvisible;
      float m_sizefactor;
      wxTimer m_destroyTimer;
      GrabberWin *m_destroyGrabber;
      wxSize m_recoversize;

      bool m_bAutoHideToolbar;
      int m_nAutoHideToolbar;
#endif
};
