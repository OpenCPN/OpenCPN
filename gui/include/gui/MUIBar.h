/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  MUI Control Bar
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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
 ***************************************************************************
 */

#ifndef __muibar_H__
#define __muibar_H__

#include <cstdint>

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

enum { ID_MUI_MENU = 21500 };

enum {
  CO_ANIMATION_LINEAR = 0,
  CO_ANIMATION_QUADRATIC,
  CO_ANIMATION_CUBIC,
  CO_ANIMATION_CUBIC_BOUNCE_IN,
  CO_ANIMATION_CUBIC_BACK_IN,
  CO_ANIMATION_CUBIC_REVERSE,
  CO_PULL,
  CO_PUSH
};

class MyFrame;
class ChartCanvas;
class MUIButton;
class MUITextButton;
class CanvasOptions;

//----------------------------------------------------------------------------
// MUIBar
//----------------------------------------------------------------------------
class MUIBar : public wxWindow {
public:
  MUIBar();
  MUIBar(ChartCanvas *parent, int orientation = wxHORIZONTAL,
         float size_factor = 1.0, wxWindowID id = wxID_ANY,
         const wxPoint &pos = wxDefaultPosition,
         const wxSize &size = wxDefaultSize, long style = 0,
         const wxString &name = wxPanelNameStr);

  ~MUIBar();

  void OnSize(wxSizeEvent &event);
  void OnPaint(wxPaintEvent &event);
  void OnToolLeftClick(wxCommandEvent &event);
  void OnEraseBackground(wxEraseEvent &event);
  void onCanvasOptionsAnimationTimerEvent(wxTimerEvent &event);

  void SetBestPosition(void);
  void UpdateDynamicValues();
  int GetOrientation() { return m_orientation; }
  void ResetCanvasOptions();
  void SetFollowButtonState(int state);
  CanvasOptions *GetCanvasOptions() { return m_canvasOptions; }
  void SetColorScheme(ColorScheme cs);
  void SetCanvasENCAvailable(bool avail);
  void OnScaleSelected(wxMouseEvent &event);
  void DrawGL(ocpnDC &gldc, double displayScale);

private:
  void Init(void);
  void CreateControls();
  void PullCanvasOptions();
  void PushCanvasOptions();

  void CaptureCanvasOptionsBitmap();
  void CaptureCanvasOptionsBitmapChain(wxTimerEvent &event);

  ChartCanvas *m_parentCanvas;
  int m_orientation;
  float m_scaleFactor;

  MUIButton *m_zinButton;
  MUIButton *m_zoutButton;
  MUIButton *m_menuButton;
  MUIButton *m_followButton;
  MUITextButton *m_scaleButton;

  CanvasOptions *m_canvasOptions;
  wxPoint m_targetCOPos;
  wxPoint m_currentCOPos;
  wxPoint m_startCOPos;
  int m_COTopOffset;

  wxSize m_canvasOptionsFullSize;

  wxTimer m_canvasOptionsAnimationTimer;
  int m_animateStep;
  int m_animateSteps;
  int m_animationType;
  int m_animationTotalTime;
  int m_pushPull;

  wxString m_backcolorString;
  wxBitmap m_animateBitmap;
  wxBitmap m_backingBitmap;
  wxTimer CanvasOptionTimer;
  int m_coSequence;
  int m_capture_size_y;
  wxPoint m_capturePoint;
  wxPoint m_backingPoint;
  bool m_coAnimateByBitmaps;
  ColorScheme m_cs;
  bool m_CanvasENCAvail;
  bool m_bEffects;

  uint32_t m_texture;
  int m_end_margin;

  DECLARE_EVENT_TABLE()
};

#endif
