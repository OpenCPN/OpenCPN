//////////////////////////////////////////////////////////////////////////////
// Name:        SVGColor.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/04/29
// RCS-ID:      $Id: SVGColor.cpp,v 1.3 2005/06/07 22:30:30 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGColor.h"

wxString wxSVGColor::GetCSSText() const
{
  if (m_colorType == wxSVG_COLORTYPE_UNKNOWN)
	return wxT("");
  return wxString::Format(_T("#%02x%02x%02x"),
	m_rgbColor.Red(), m_rgbColor.Green(), m_rgbColor.Blue());
}
	
void wxSVGColor::SetRGBColor(const wxRGBColor& rgbColor)
{
  m_rgbColor = rgbColor;
  m_colorType = wxSVG_COLORTYPE_RGBCOLOR;
}

void wxSVGColor::SetICCColor(const wxSVGICCColor& iccColor)
{
  m_iccColor = iccColor;
  m_colorType = wxSVG_COLORTYPE_RGBCOLOR_ICCCOLOR;
}

void wxSVGColor::SetRGBColor(const wxString& rgbColor)
{

}

void wxSVGColor::SetRGBColorICCColor(const wxString& rgbColor, const wxString& iccColor)
{

}

void wxSVGColor::SetColor(wxSVG_COLORTYPE colorType, const wxString& rgbColor, const wxString& iccColor)
{

}
