/******************************************************************************
 *
 * Project:  OpenCP
 * Purpose:  Console Canvas
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   bdbcat@yahoo.com   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 * $Log: concanv.h,v $
 * Revision 1.8  2010/04/27 01:44:56  bdbcat
 * Build 426
 *
 * Revision 1.7  2009/08/22 01:21:44  bdbcat
 * Improved font support
 *
 * Revision 1.6  2009/06/28 03:08:41  bdbcat
 * Set minimum size for Console.
 *
 * Revision 1.5  2008/08/26 13:49:53  bdbcat
 * Better color scheme support
 *
 * Revision 1.4  2008/03/30 23:22:08  bdbcat
 * *** empty log message ***
 *
 * Revision 1.3  2008/01/12 06:18:30  bdbcat
 * Update for Mac OSX/Unicode
 *
 * Revision 1.2  2007/06/10 02:37:18  bdbcat
 * Cleanup
 *
 * Revision 1.1.1.1  2006/08/21 05:52:11  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.1.1.1  2006/04/19 03:23:28  dsr
 * Rename/Import to OpenCPN
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

private:
      void CalculateMinSize(void);

      wxBrush     *m_pbackBrush;
      wxColour    m_text_color;

      wxString    m_label;
      wxString    m_value;
      wxFont      *m_plabelFont;
      wxFont      *m_pvalueFont;

      wxString    m_LegendTextElement;
      wxString    m_ValueTextElement;

DECLARE_EVENT_TABLE()

};




//----------------------------------------------------------------------------
// ConsoleCanvas
//----------------------------------------------------------------------------
class ConsoleCanvas: public wxWindow
{
public:
      ConsoleCanvas(wxWindow *frame);
      ~ConsoleCanvas();
      void UpdateRouteData();
      void ShowWithFreshFonts(void);
      void UpdateFonts(void);
      void SetColorScheme(ColorScheme cs);
      void OnLegRouteButton(wxCommandEvent& event);
      void MouseEvent(wxMouseEvent& event);
      void MouseLostCaptureEvent(wxMouseCaptureLostEvent& event);

      wxWindow          *m_pParent;
      wxStaticBox       *pThisLegBox;
      wxStaticBoxSizer  *m_pitemStaticBoxSizerLeg;

      AnnunText         *pXTE;
      AnnunText         *pBRG;
      AnnunText         *pRNG;
      AnnunText         *pTTG;
      AnnunText         *pVMG;
      CDI               *pCDI;
      wxButton          *m_pLegRouteButton;

      wxFont            *pThisLegFont;

      bool              m_bRouteDataStale;
      bool              m_bNeedClear;
      bool              m_bShowRouteTotal;

      wxBrush           *pbackBrush;

private:
      void OnPaint(wxPaintEvent& event);


DECLARE_EVENT_TABLE()
};

#endif
