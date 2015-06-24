/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Console Canvas
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 */

#ifndef __concanv_H__
#define __concanv_H__


//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

#include "chart1.h"             // for ColorScheme


#define ID_LEGROUTE 1000

// Class declarations
class Routeman;

//----------------------------------------------------------------------------
// CDI
//----------------------------------------------------------------------------

class CDI:public wxWindow
{
public:
      CDI(wxWindow* parent, wxWindowID id, long style, const wxString& name);

      void OnPaint(wxPaintEvent& event);
      void SetColorScheme(ColorScheme cs);
      void MouseEvent( wxMouseEvent& event );
      
      wxBrush *m_pbackBrush;
      wxBrush *m_proadBrush;
      wxPen   *m_proadPen;

DECLARE_EVENT_TABLE()

};

//----------------------------------------------------------------------------
// AnnunText
//----------------------------------------------------------------------------
class AnnunText : public wxWindow
{
public:
      AnnunText(wxWindow *parent, wxWindowID id, const wxString& LegendElement, const wxString& ValueElement);

      ~AnnunText();

      void SetALabel(const wxString &l);
      void SetAValue(const wxString &v);
      void OnPaint(wxPaintEvent& event);
      void RefreshFonts(void);
      void SetLegendElement(const wxString &element);
      void SetValueElement(const wxString &element);
      void SetColorScheme(ColorScheme cs);
      void MouseEvent( wxMouseEvent& event );
      
private:
      void CalculateMinSize(void);

      wxBrush     m_backBrush;
      wxColour    m_default_text_color;

      wxString    m_label;
      wxString    m_value;
      wxFont      *m_plabelFont;
      wxFont      *m_pvalueFont;

      wxString    m_LegendTextElement;
      wxString    m_ValueTextElement;
      wxColour    m_legend_color;
      wxColour    m_val_color;

DECLARE_EVENT_TABLE()

};




//----------------------------------------------------------------------------
// ConsoleCanvas
//----------------------------------------------------------------------------
class ConsoleCanvas: public wxDialog
{
public:
      ConsoleCanvas(wxWindow *frame);
      ~ConsoleCanvas();
      void UpdateRouteData();
      void ShowWithFreshFonts(void);
      void UpdateFonts(void);
      void SetColorScheme(ColorScheme cs);
      void LegRoute();
      void OnContextMenu( wxContextMenuEvent& event );
      void OnContextMenuSelection( wxCommandEvent& event );
      void RefreshConsoleData(void);
      
      wxWindow          *m_pParent;
      wxStaticText       *pThisLegText;
      wxBoxSizer        *m_pitemBoxSizerLeg;

      AnnunText         *pXTE;
      AnnunText         *pBRG;
      AnnunText         *pRNG;
      AnnunText         *pTTG;
      AnnunText         *pVMG;
      CDI               *pCDI;

      wxFont            *pThisLegFont;
      bool              m_bShowRouteTotal;
      bool              m_bNeedClear;
      wxBrush           *pbackBrush;

private:
      void OnPaint(wxPaintEvent& event);
      void OnShow(wxShowEvent& event);

DECLARE_EVENT_TABLE()
};

#endif
