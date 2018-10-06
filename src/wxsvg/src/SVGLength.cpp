//////////////////////////////////////////////////////////////////////////////
// Name:        SVGLength.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/04/29
// RCS-ID:      $Id: SVGLength.cpp,v 1.5 2005/11/07 17:39:59 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGLength.h"

wxString wxSVGLength::GetValueAsString() const
{
  wxString value = wxString::Format(wxT("%g"), m_valueInSpecifiedUnits);
  switch (m_unitType)
  {
	case wxSVG_LENGTHTYPE_UNKNOWN: break;
	case wxSVG_LENGTHTYPE_NUMBER: break;
	case wxSVG_LENGTHTYPE_PX: value += wxT("px"); break;
	case wxSVG_LENGTHTYPE_PERCENTAGE: value += wxT("%"); break;
	case wxSVG_LENGTHTYPE_EMS: value += wxT("em"); break;
	case wxSVG_LENGTHTYPE_EXS: value += wxT("ex"); break;
	case wxSVG_LENGTHTYPE_CM: value += wxT("cm"); break;
	case wxSVG_LENGTHTYPE_MM: value += wxT("mm"); break;
	case wxSVG_LENGTHTYPE_IN: value += wxT("in"); break;
	case wxSVG_LENGTHTYPE_PT: value += wxT("pt"); break;
	case wxSVG_LENGTHTYPE_PC: value += wxT("pc"); break;
  }
  return value;
}

void wxSVGLength::SetValueAsString(const wxString& n)
{
  m_valueInSpecifiedUnits = 0;
  m_unitType = wxSVG_LENGTHTYPE_NUMBER;
  wxString value = n.Strip(wxString::both);
  wxString unit;
  if (value.length()>=2)
  {
	const wxString s_numeric = wxT("0123456789");
	const wxString s_numericFirst = wxT("+-.Ee") + s_numeric;
	if (!s_numeric.Contains(value.Right(1)))
	{
	  if (s_numericFirst.Contains(value.Mid(value.Length()-2,1)))
	  {
		unit = value.Right(1);
		value = value.Left(value.Length()-1);
	  }
	  else
	  {
		unit = value.Right(2);
		value = value.Left(value.Length()-2);
	  }
	}
  }
  
  double d;
  if (!value.ToDouble(&d))
	return;
  m_valueInSpecifiedUnits = d;
  
  if (unit.length() == 0);
  else if (unit == wxT("px"))
	m_unitType = wxSVG_LENGTHTYPE_PX;
  else if (unit.Right(1) == wxT("%"))
	m_unitType = wxSVG_LENGTHTYPE_PERCENTAGE;
  else if (unit == wxT("em"))
	m_unitType = wxSVG_LENGTHTYPE_EMS;
  else if (unit == wxT("ex"))
	m_unitType = wxSVG_LENGTHTYPE_EXS;
  else if (unit == wxT("cm"))
	m_unitType = wxSVG_LENGTHTYPE_CM;
  else if (unit == wxT("mm"))
	m_unitType = wxSVG_LENGTHTYPE_MM;
  else if (unit == wxT("in"))
	m_unitType = wxSVG_LENGTHTYPE_IN;
  else if (unit == wxT("pt"))
	m_unitType = wxSVG_LENGTHTYPE_PT;
  else if (unit == wxT("pc"))
	m_unitType = wxSVG_LENGTHTYPE_PC;
  SetValueInSpecifiedUnits(m_valueInSpecifiedUnits);
}

void wxSVGLength::NewValueSpecifiedUnits(wxSVG_LENGTHTYPE unitType, double valueInSpecifiedUnits)
{
  m_unitType = unitType;
  SetValueInSpecifiedUnits(valueInSpecifiedUnits);
}

void wxSVGLength::ConvertToSpecifiedUnits(wxSVG_LENGTHTYPE unitType)
{
  m_unitType = unitType;
}

void wxSVGLength::SetValueInSpecifiedUnits(double n)
{
  m_valueInSpecifiedUnits = n;
  m_value = n;
  switch (m_unitType)
  {
	case wxSVG_LENGTHTYPE_UNKNOWN: break;
	case wxSVG_LENGTHTYPE_NUMBER: break;
	case wxSVG_LENGTHTYPE_PX: break;
	case wxSVG_LENGTHTYPE_PERCENTAGE: break; // todo
	case wxSVG_LENGTHTYPE_EMS: break; // todo
	case wxSVG_LENGTHTYPE_EXS: break; // todo
	case wxSVG_LENGTHTYPE_CM: m_value *= 35.43307; break;
	case wxSVG_LENGTHTYPE_MM: m_value *= 3.543307; break;
	case wxSVG_LENGTHTYPE_IN: m_value *= 90; break;
	case wxSVG_LENGTHTYPE_PT: m_value *= 1.25; break;
	case wxSVG_LENGTHTYPE_PC: m_value *= 15; break;
  }
}
