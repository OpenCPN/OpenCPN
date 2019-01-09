//////////////////////////////////////////////////////////////////////////////
// Name:        SVGAngle.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/05/10
// RCS-ID:      $Id: SVGAngle.cpp,v 1.4 2005/11/07 19:03:00 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGAngle.h"

void wxSVGAngle::SetValueInSpecifiedUnits(double n)
{
  m_valueInSpecifiedUnits = n;
}

wxString wxSVGAngle::GetValueAsString() const
{
  wxString value = wxString::Format(wxT("%g"), m_valueInSpecifiedUnits);
  switch (m_unitType)
  {
	case wxSVG_ANGLETYPE_UNKNOWN: break;
	case wxSVG_ANGLETYPE_UNSPECIFIED: break;
	case wxSVG_ANGLETYPE_DEG: break;
	case wxSVG_ANGLETYPE_RAD: break;
	case wxSVG_ANGLETYPE_GRAD: break;
  }
  return value;
}

void wxSVGAngle::SetValueAsString(const wxString& n)
{
  m_unitType = wxSVG_ANGLETYPE_DEG;
  double d;
  if (!n.ToDouble(&d))
	return;
  SetValueInSpecifiedUnits(d);
}

void wxSVGAngle::NewValueSpecifiedUnits(wxSVG_ANGLETYPE unitType, double valueInSpecifiedUnits)
{
  m_unitType = unitType;
  m_valueInSpecifiedUnits = valueInSpecifiedUnits;
}

void wxSVGAngle::ConvertToSpecifiedUnits(wxSVG_ANGLETYPE unitType)
{
  m_unitType = unitType;
}

