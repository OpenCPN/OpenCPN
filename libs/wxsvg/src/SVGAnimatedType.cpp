//////////////////////////////////////////////////////////////////////////////
// Name:        SVGAnimatedType.cpp
// Purpose:     Implementation of SVGAnimatedType
// Author:      Alex Thuering
// Created:     2014/02/24
// RCS-ID:      $Id: SVGAnimatedType.cpp,v 1.2 2014/03/27 08:42:16 ntalex Exp $
// Copyright:   (c) 2014 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGAnimatedType.h"
#include "CSSStyleDeclaration.h"

wxSVGAnimatedType::~wxSVGAnimatedType() {
	// nothing to do
}

wxString wxSVGAnimatedType::GetValueAsString() const {
	return m_string;
}

void wxSVGAnimatedType::SetValueAsString(const wxString& value) {
	m_string = value;
	wxString val = value.Strip(wxString::both);
	if (val.length() && (val[0] < wxT('0') || val[0] > wxT('9')) && val[0] != wxT('+')
			&& val[0] != wxT('-') && val[0] != wxT('.') && val[0] != wxT('E') && val[0] != wxT('e')) {
		m_color = wxCSSStyleDeclaration::ParseColor(value);
		m_propertyType = m_color.Ok() || val == wxT("none") ? wxSVG_ANIMATED_COLOR : wxSVG_ANIMATED_STRING;
	} else if ((int) val.Index(wxT(',')) != -1) {
		m_propertyType = wxSVG_ANIMATED_LENGTH_LIST;
		m_lengthList.SetValueAsString(val);
	} else {
		m_propertyType = wxSVG_ANIMATED_LENGTH;
		m_length.SetValueAsString(val);
	}
}
