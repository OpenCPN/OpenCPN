//////////////////////////////////////////////////////////////////////////////
// Name:        SVGPaint.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/05/10
// RCS-ID:      $Id: SVGPaint.cpp,v 1.4 2006/01/08 12:21:13 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGPaint.h"

wxString wxSVGPaint::GetCSSText() const
{
  switch (m_paintType)
  {
    case wxSVG_PAINTTYPE_UNKNOWN:
      break;
    case wxSVG_PAINTTYPE_RGBCOLOR:
    case wxSVG_PAINTTYPE_RGBCOLOR_ICCCOLOR:
      return wxSVGColor::GetCSSText();
    case wxSVG_PAINTTYPE_NONE:
      return wxT("none");
    case wxSVG_PAINTTYPE_CURRENTCOLOR:
      return wxT("currentColor");
    case wxSVG_PAINTTYPE_URI_NONE:
      return wxT("url(") + m_uri + wxT(") none");
    case wxSVG_PAINTTYPE_URI_CURRENTCOLOR:
      return wxT("url(") + m_uri + wxT(") currentColor");
    case wxSVG_PAINTTYPE_URI_RGBCOLOR:
    case wxSVG_PAINTTYPE_URI_RGBCOLOR_ICCCOLOR:
      return wxT("url(") + m_uri + wxT(") ") + wxSVGColor::GetCSSText();
    case wxSVG_PAINTTYPE_URI:
      return wxT("url(") + m_uri + wxT(")");
  }
  return wxT("");
}

void wxSVGPaint::SetUri(const wxString& uri)
{
  m_uri = uri;
  m_paintType = wxSVG_PAINTTYPE_URI;
}

void wxSVGPaint::SetRGBColor(const wxRGBColor& rgbColor)
{
  wxSVGColor::SetRGBColor(rgbColor);
  if (rgbColor.Ok())
    m_paintType = m_uri.length() ? wxSVG_PAINTTYPE_URI_RGBCOLOR : wxSVG_PAINTTYPE_RGBCOLOR;
  else
    m_paintType = m_uri.length() ? wxSVG_PAINTTYPE_URI_NONE : wxSVG_PAINTTYPE_NONE;
}

void wxSVGPaint::SetICCColor(const wxSVGICCColor& iccColor)
{
  wxSVGColor::SetICCColor(iccColor);
  if (m_iccColor.GetColors().Count())
    m_paintType = m_uri.length() ? wxSVG_PAINTTYPE_URI_RGBCOLOR_ICCCOLOR : wxSVG_PAINTTYPE_RGBCOLOR_ICCCOLOR;
  else
    SetRGBColor(GetRGBColor());
}

void wxSVGPaint::SetPaint(wxSVG_PAINTTYPE paintType, const wxString& uri,
   const wxString& rgbColor, const wxString& iccColor)
{

}
