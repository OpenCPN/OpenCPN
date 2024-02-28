/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Bar Window
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#ifndef __statwin_H__
#define __statwin_H__


//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

#define PIANO_EVENT_TIMER 73566
#define DEFERRED_KEY_CLICK_DOWN 1
#define DEFERRED_KEY_CLICK_UP 2
#define INFOWIN_TIMEOUT 3

// Class declarations
WX_DECLARE_OBJARRAY(wxRect, RectArray);

class MyFrame;
class ChartCanvas;

enum {
  PIANO_MODE_COMPOSITE = 0,
  PIANO_MODE_LEGACY
};

//----------------------------------------------------------------------------
// PianoKeyElement
//----------------------------------------------------------------------------
class PianoKeyElement {
public:
  PianoKeyElement() {};
  PianoKeyElement(int scale);
  ~PianoKeyElement() {};

  int chart_scale;
  ChartTypeEnum chart_type;
  ChartFamilyEnum chart_family;
  std::vector<int> dbindex_list;
};



//----------------------------------------------------------------------------
// Piano
//----------------------------------------------------------------------------
class Piano : public wxEvtHandler {
public:
  Piano(ChartCanvas *parent);
  ~Piano();

  void Paint(int y, wxDC &dc, wxDC *shapeDC = NULL);
  void Paint(int y, ocpnDC &dc, wxDC *shapeDC = NULL);
  void DrawGL(int y);
  void FormatKeys(void);
  bool MouseEvent(wxMouseEvent &event);
  void SetColorScheme(ColorScheme cs);
  void SetKeyArray(std::vector<int> &center_array, std::vector<int> &full_array);
  void SetActiveKey(int iactive) { m_iactive = iactive; }
  void SetActiveKeyArray(std::vector<int> array);
  void SetNoshowIndexArray(std::vector<int> array);
  void AddNoshowIndexArray(std::vector<int> array);
  void SetEclipsedIndexArray(std::vector<int> array);
  void SetSkewIndexArray(std::vector<int> array);
  void SetTmercIndexArray(std::vector<int> array);
  void SetPolyIndexArray(std::vector<int> array);
  int GetPianoMode() { return m_piano_mode;}

  std::vector<int> GetActiveKeyArray() { return m_active_index_array; }

  void SetVizIcon(wxBitmap *picon_bmp) {
    if (m_pVizIconBmp) delete m_pVizIconBmp;
    m_pVizIconBmp = picon_bmp;
  }
  void SetInVizIcon(wxBitmap *picon_bmp) {
    if (m_pInVizIconBmp) delete m_pInVizIconBmp;
    m_pInVizIconBmp = picon_bmp;
  }
  void SetSkewIcon(wxBitmap *picon_bmp) {
    if (m_pSkewIconBmp) delete m_pSkewIconBmp;
    m_pSkewIconBmp = picon_bmp;
  }
  void SetTMercIcon(wxBitmap *picon_bmp) {
    if (m_pTmercIconBmp) delete m_pTmercIconBmp;
    m_pTmercIconBmp = picon_bmp;
  }
  void SetPolyIcon(wxBitmap *picon_bmp) {
    if (m_pPolyIconBmp) delete m_pPolyIconBmp;
    m_pPolyIconBmp = picon_bmp;
  }
  void ShowBusy(bool busy);
  void onTimerEvent(wxTimerEvent &event);

  wxPoint GetKeyOrigin(int key_index);
  void ResetRollover(void);
  void SetRoundedRectangles(bool val) {
    m_brounded = val;
    m_hash.Clear();
  }

  int GetHeight();
  int GetWidth();

  wxString &GenerateAndStoreNewHash();
  wxString &GetStoredHash();

  int GetnKeys() { return m_nRegions; }

private:
  void SetPianoMode(int new_mode) {m_piano_mode = new_mode;}

  void DrawGLSL(int y);
  void BuildGLTexture();
  bool InArray(std::vector<int> &array, int key);
  bool IsAnyActiveChartInPianoKeyElement(PianoKeyElement &pke);
  bool IsAllEclipsedChartInPianoKeyElement(PianoKeyElement &pke);

  wxString GetStateHash();
  wxString m_hash;

  ChartCanvas *m_parentCanvas;
  int m_piano_mode;

  std::vector<PianoKeyElement> m_composite_array;

  int m_nRegions;
  int m_index_last;
  int m_hover_icon_last;
  int m_hover_last;
  bool m_gotPianoDown;

  wxBrush m_backBrush;
  wxBrush m_srBrush, m_rBrush;
  wxBrush m_svBrush, m_vBrush;
  wxBrush m_unavailableBrush;
  wxBrush m_utileBrush, m_tileBrush;

  wxBrush m_cBrush;
  wxBrush m_scBrush;

  std::vector<int> m_key_array;
  std::vector<int> m_noshow_index_array;
  std::vector<int> m_active_index_array;
  std::vector<int> m_eclipsed_index_array;
  std::vector<int> m_skew_index_array;
  std::vector<int> m_tmerc_index_array;
  std::vector<int> m_poly_index_array;
  bool m_bBusy;
  wxTimer m_eventTimer;
  int m_click_sel_index;
  int m_action;

  std::vector<wxRect> KeyRect;

  wxBitmap *m_pVizIconBmp;
  wxBitmap *m_pInVizIconBmp;
  wxBitmap *m_pTmercIconBmp;
  wxBitmap *m_pSkewIconBmp;
  wxBitmap *m_pPolyIconBmp;

  int m_iactive;
  bool m_brounded;
  bool m_bleaving;

  unsigned int m_tex, m_texw, m_texh, m_tex_piano_height;
  int m_ref, m_pad, m_radius, m_texPitch;

  int m_width;
  int m_width_avail;

  DECLARE_EVENT_TABLE()
};

//----------------------------------------------------------------------------
// ChartBarWin
//----------------------------------------------------------------------------
class ChartBarWin : public wxDialog {
public:
  ChartBarWin(wxWindow *win);
  ~ChartBarWin();
  void OnSize(wxSizeEvent &event);
  void OnPaint(wxPaintEvent &event);
  void MouseEvent(wxMouseEvent &event);
  int GetFontHeight();
  void RePosition();
  void ReSize();

  DECLARE_EVENT_TABLE()
};

#endif
