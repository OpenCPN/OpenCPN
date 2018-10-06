//////////////////////////////////////////////////////////////////////////////
// Name:        CSSValue.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/05/03
// RCS-ID:      $Id: CSSValue.cpp,v 1.13 2014/06/30 19:06:12 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "CSSValue.h"
#include <wx/tokenzr.h>

static wxSortedArrayString* s_cssValues = NULL;
#include "css_values.cpp"
inline void FillCSSValues()
{
  if (s_cssValues == NULL)
  {
	s_cssValues = new wxSortedArrayString;
	for (unsigned int i=0; i<sizeof(s_cssValueStrings)/sizeof(s_cssValueStrings[0]); i++)
	  s_cssValues->Add(s_cssValueStrings[i]);
  }
}

wxCSS_VALUE wxCSSValue::GetValueId(wxString value)
{
  FillCSSValues();
  int id = s_cssValues->Index(value);
  if (id >= 0)
	return wxCSS_VALUE(id+1);
  return wxCSS_VALUE_UNKNOWN;
}

wxString wxCSSValue::GetValueString(wxCSS_VALUE id)
{
  FillCSSValues();
  if (id == wxCSS_VALUE_UNKNOWN)
	return wxT("");
  return (*s_cssValues)[int(id)-1];
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////  wxCSSPrimitiveValue //////////////////////////////
//////////////////////////////////////////////////////////////////////////////

wxCSSPrimitiveValue::wxCSSPrimitiveValue(const wxCSSPrimitiveValue& src):
  wxCSSValue(wxCSS_PRIMITIVE_VALUE)
{
  m_primitiveType = src.m_primitiveType;
  if (m_primitiveType == wxCSS_UNKNOWN);
  else if (int(m_primitiveType) >= int(wxCSS_NUMBER) &&
		   int(m_primitiveType) <= int(wxCSS_DIMENSION))
	m_number = src.m_number;
  else if (m_primitiveType == wxCSS_IDENT)
	m_ident = src.m_ident;
  else if (m_primitiveType == wxCSS_STRING ||
		   m_primitiveType == wxCSS_URI ||
  		   m_primitiveType == wxCSS_ATTR)
	m_string = new wxString(*src.m_string);
  else if (m_primitiveType == wxCSS_RECT)
	m_rect = new wxRect(*src.m_rect);
  else if (m_primitiveType == wxCSS_RGBCOLOR)
	m_color = new wxRGBColor(*src.m_color);
}

wxString wxCSSPrimitiveValue::GetCSSText() const
{
  if (m_primitiveType == wxCSS_URI)
	return wxT("url(") + *m_string + wxT(")");
  return GetStringValue();
}

void wxCSSPrimitiveValue::SetStringValue(wxCSS_PRIMITIVE_TYPE stringType, const wxString& stringValue)
{
  if (m_primitiveType != wxCSS_STRING &&
	  m_primitiveType != wxCSS_URI &&
	  m_primitiveType != wxCSS_ATTR)
  {
	CleanUp();
	m_string = new wxString;
  }
  m_primitiveType = stringType == wxCSS_URI || stringType == wxCSS_ATTR ? stringType : wxCSS_STRING;
  *m_string = stringValue;
}

wxString wxCSSPrimitiveValue::GetStringValue() const
{
  switch (m_primitiveType)
  {
	case wxCSS_STRING:
	case wxCSS_URI:
	case wxCSS_ATTR:
	  return *m_string;
    case wxCSS_IDENT:
	  return GetValueString(m_ident);
	case wxCSS_COUNTER:
	  break;
	case wxCSS_RECT:
	  break;
	case wxCSS_RGBCOLOR:
	  if (m_color->Ok())
		return wxString::Format(_T("#%02x%02x%02x"),
		  m_color->Red(), m_color->Green(), m_color->Blue());
	  break;
	case wxCSS_UNKNOWN:
	  break;
	default:
	  return wxString::Format(wxT("%g"), m_number);
  }
  return wxT("");
}

void wxCSSPrimitiveValue::SetFloatValue(wxCSS_PRIMITIVE_TYPE unitType, double doubleValue)
{
  CleanUp();
  m_primitiveType =
	int(unitType) >= int(wxCSS_NUMBER) &&
	int(unitType) <= int(wxCSS_DIMENSION) ? unitType : wxCSS_NUMBER;
  m_number = doubleValue;
}

double wxCSSPrimitiveValue::GetFloatValue(wxCSS_PRIMITIVE_TYPE unitType) const
{
  if (int(m_primitiveType) >= int(wxCSS_NUMBER) &&
	  int(m_primitiveType) <= int(wxCSS_DIMENSION))
	return m_number;
  return 0;
}

void wxCSSPrimitiveValue::SetRectValue(const wxRect& rect)
{
  if (m_primitiveType != wxCSS_RECT)
  {
	CleanUp();
	m_rect = new wxRect;
  }
  m_primitiveType = wxCSS_RECT;
  *m_rect = rect;
}

wxRect wxCSSPrimitiveValue::GetRectValue() const
{
  if (m_primitiveType == wxCSS_RECT)
	return *m_rect;
  return wxRect();
}

void wxCSSPrimitiveValue::SetRGBColorValue(const wxRGBColor& color)
{
  if (m_primitiveType != wxCSS_RGBCOLOR)
  {
	CleanUp();
	m_color = new wxRGBColor;
  }
  m_primitiveType = wxCSS_RGBCOLOR;
  *m_color = color;
}

wxRGBColor wxCSSPrimitiveValue::GetRGBColorValue() const
{
  if (m_primitiveType == wxCSS_RGBCOLOR)
	return *m_color;
  return wxRGBColor();
}

void wxCSSPrimitiveValue::SetIdentValue(wxCSS_VALUE ident)
{
  if (m_primitiveType != wxCSS_IDENT)
	CleanUp();
  m_primitiveType = wxCSS_IDENT;
  m_ident = ident;
}

wxCSS_VALUE wxCSSPrimitiveValue::GetIdentValue() const
{
  if (m_primitiveType == wxCSS_IDENT)
	return m_ident;
  return wxCSS_VALUE_UNKNOWN;
}

void wxCSSPrimitiveValue::CleanUp()
{
  if (m_primitiveType == wxCSS_STRING ||
	  m_primitiveType == wxCSS_URI ||
	  m_primitiveType == wxCSS_ATTR)
	delete m_string;
  else if (m_primitiveType == wxCSS_RECT)
	delete m_rect;
  else if (m_primitiveType == wxCSS_RGBCOLOR)
	delete m_color;
  m_primitiveType = wxCSS_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////////////
/////////////////////////////  wxCSSValueList ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

wxCSSValueList::wxCSSValueList(const wxCSSValueList& src) {
	m_values = src.m_values;
}

wxString wxCSSValueList::GetCSSText() const {
	wxString result;
	for (std::vector<double>::const_iterator it = m_values.begin(); it != m_values.end(); ++it) {
		if (result.length())
			result += wxT(", ");
		result += wxString::Format(wxT("%g"), *it);
	}
	return result;
}

void wxCSSValueList::SetCSSText(const wxString& value) {
	double val;
	wxStringTokenizer tkz(value, wxT(", \t"));
	while (tkz.HasMoreTokens()) {
		wxString token = tkz.GetNextToken();
		if (token.length() && token.ToDouble(&val))
			m_values.push_back(val);
	}
}
