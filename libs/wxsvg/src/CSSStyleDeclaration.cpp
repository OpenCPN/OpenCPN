//////////////////////////////////////////////////////////////////////////////
// Name:        CSSStyleDeclaration.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/05/03
// RCS-ID:      $Id: CSSStyleDeclaration.cpp,v 1.13 2015/03/21 16:28:23 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "CSSStyleDeclaration.h"
#include <wx/tokenzr.h>
#include <wx/log.h>

wxCSSPrimitiveValue* wxCSSStyleDeclaration::s_emptyCSSValue = new wxCSSPrimitiveValue;
wxSVGColor* wxCSSStyleDeclaration::s_emptySVGColor = new wxSVGColor;
wxSVGPaint* wxCSSStyleDeclaration::s_emptySVGPaint = new wxSVGPaint;
wxSVGPaint* wxCSSStyleDeclaration::s_blackSVGPaint = new wxSVGPaint(0,0,0);
wxCSSValueList* wxCSSStyleDeclaration::s_emptyValueList = new wxCSSValueList;

wxCSSStyleDeclaration::~wxCSSStyleDeclaration() {
	for (iterator it = begin(); it != end(); ++it)
		delete it->second;
}

wxCSSStyleDeclaration& wxCSSStyleDeclaration::operator=(
		const wxCSSStyleDeclaration& src) {
	for (iterator it = begin(); it != end(); ++it)
		delete it->second;
	clear();
	Add(src);
	return *this;
}

void wxCSSStyleDeclaration::Add(const wxCSSStyleDeclaration& style) {
	const_iterator it;
	for (it = style.begin(); it != style.end(); ++it) {
		iterator it2 = find(it->first);
		if (it2 != end()) { // replace old value with new one
			delete it2->second;
			it2->second = it->second->Clone();
		} else
			(*this)[it->first] = it->second->Clone();
	}
}

wxString wxCSSStyleDeclaration::GetCSSText() const {
	wxString text;
	const_iterator it;
	for (it = begin(); it != end(); ++it) 
		text = text + GetPropertyName(it->first) + wxT(":") + it->second->GetCSSText() + wxT(";");
	return text;
}

void wxCSSStyleDeclaration::SetCSSText(const wxString& text) {
	wxStringTokenizer tkz(text, wxT(";"));
	while (tkz.HasMoreTokens()) {
		wxString token = tkz.GetNextToken().Strip(wxString::both);
		int pos = token.find(wxT(':'));
		if (pos <= 0)
			continue;
		SetProperty(token.substr(0, pos), token.substr(pos + 1));
	}
}

static wxSortedArrayString* s_cssProperties = NULL;
#include "css_properties.cpp"
inline void FillCSSProperties() {
	if (s_cssProperties == NULL) {
		s_cssProperties = new wxSortedArrayString;
		for (unsigned int i = 0; i < sizeof(s_cssPropertyStrings) / sizeof(s_cssPropertyStrings[0]); i++)
			s_cssProperties->Add(s_cssPropertyStrings[i]);
	}
}

wxCSS_PROPERTY wxCSSStyleDeclaration::GetPropertyId(const wxString& propertyName) {
	FillCSSProperties();
	int id = s_cssProperties->Index(propertyName);
	if (id >= 0)
		return wxCSS_PROPERTY(id + 1);
	return wxCSS_PROPERTY_UNKNOWN;
}

wxString wxCSSStyleDeclaration::GetPropertyName(wxCSS_PROPERTY propertyId) {
	FillCSSProperties();
	if (propertyId == wxCSS_PROPERTY_UNKNOWN)
		return wxT("");
	return (*s_cssProperties)[int(propertyId) - 1];
}

void wxCSSStyleDeclaration::SetProperty(wxCSS_PROPERTY propertyId, const wxString& svalue) {
	if (propertyId == wxCSS_PROPERTY_UNKNOWN)
		return;
	wxString value = svalue.Strip(wxString::both);
	wxCSSValue* cssValue = NULL;
	iterator it = find(propertyId);
	if (it != end())
		cssValue = it->second;
	if (value == wxT("inherit")) {
		if (cssValue != NULL)
			erase(propertyId);
		return;
	}
	switch (propertyId) {
	case wxCSS_PROPERTY_UNKNOWN:
		break;
	// only idents
	case wxCSS_PROPERTY_ALIGNMENT_BASELINE:
	case wxCSS_PROPERTY_BASELINE_SHIFT:
	case wxCSS_PROPERTY_CLIP_RULE:
	case wxCSS_PROPERTY_COLOR_INTERPOLATION:
	case wxCSS_PROPERTY_COLOR_INTERPOLATION_FILTERS:
	case wxCSS_PROPERTY_COLOR_RENDERING:
	case wxCSS_PROPERTY_DIRECTION:
	case wxCSS_PROPERTY_DISPLAY:
	case wxCSS_PROPERTY_DOMINANT_BASELINE:
	case wxCSS_PROPERTY_ENABLE_BACKGROUND:
	case wxCSS_PROPERTY_FILL_RULE:
	case wxCSS_PROPERTY_FONT_STRETCH:
	case wxCSS_PROPERTY_FONT_STYLE:
	case wxCSS_PROPERTY_FONT_VARIANT:
	case wxCSS_PROPERTY_FONT_WEIGHT:
	case wxCSS_PROPERTY_IMAGE_RENDERING:
	case wxCSS_PROPERTY_OVERFLOW:
	case wxCSS_PROPERTY_POINTER_EVENTS:
	case wxCSS_PROPERTY_SHAPE_RENDERING:
	case wxCSS_PROPERTY_STROKE_LINECAP:
	case wxCSS_PROPERTY_STROKE_LINEJOIN:
	case wxCSS_PROPERTY_TEXT_ANCHOR:
	case wxCSS_PROPERTY_TEXT_DECORATION:
	case wxCSS_PROPERTY_TEXT_RENDERING:
	case wxCSS_PROPERTY_UNICODE_BIDI:
	case wxCSS_PROPERTY_VISIBILITY:
	case wxCSS_PROPERTY_WRITING_MODE:
		if (!cssValue)
			cssValue = new wxCSSPrimitiveValue;
		((wxCSSPrimitiveValue*) cssValue)->SetIdentValue(wxCSSValue::GetValueId(value));
		break;
	case wxCSS_PROPERTY_CLIP:
		if (!cssValue)
			cssValue = new wxCSSPrimitiveValue;
		if (value == wxT("none"))
			((wxCSSPrimitiveValue*) cssValue)->SetIdentValue(wxCSS_VALUE_NONE);
		else
			((wxCSSPrimitiveValue*) cssValue)->SetStringValue(wxCSS_STRING, value);
		break;
	// url or ident
	case wxCSS_PROPERTY_CLIP_PATH:
	case wxCSS_PROPERTY_CURSOR:
	case wxCSS_PROPERTY_FILTER:
	case wxCSS_PROPERTY_MARKER_END:
	case wxCSS_PROPERTY_MARKER_MID:
	case wxCSS_PROPERTY_MARKER_START:
	case wxCSS_PROPERTY_MASK:
		if (!cssValue)
			cssValue = new wxCSSPrimitiveValue;
		if (value.Left(3) == wxT("url"))
			((wxCSSPrimitiveValue*) cssValue)->SetStringValue(wxCSS_URI,
					value.AfterFirst(wxT('(')).BeforeFirst(wxT(')')));
		else
			((wxCSSPrimitiveValue*) cssValue)->SetIdentValue(wxCSSValue::GetValueId(value));
		break;
	case wxCSS_PROPERTY_COLOR:
		if (!cssValue)
			cssValue = new wxCSSPrimitiveValue;
		((wxCSSPrimitiveValue*) cssValue)->SetRGBColorValue(ParseColor(value));
		break;
	case wxCSS_PROPERTY_COLOR_PROFILE:
		if (!cssValue)
			cssValue = new wxCSSPrimitiveValue;
		if (value == wxT("auto"))
			((wxCSSPrimitiveValue*) cssValue)->SetIdentValue(wxCSS_VALUE_AUTO);
		else if (value == wxT("sRGB"))
			((wxCSSPrimitiveValue*) cssValue)->SetIdentValue(wxCSS_VALUE_SRGB);
		else if (value.Left(3) == wxT("url"))
			((wxCSSPrimitiveValue*) cssValue)->SetStringValue(wxCSS_URI,
					value.AfterFirst(wxT('(')).BeforeFirst(wxT(')')));
		else
			((wxCSSPrimitiveValue*) cssValue)->SetStringValue(wxCSS_STRING, value);
		break;
	// number
	case wxCSS_PROPERTY_FILL_OPACITY:
	case wxCSS_PROPERTY_FLOOD_OPACITY:
	case wxCSS_PROPERTY_FONT_SIZE:
	case wxCSS_PROPERTY_GLYPH_ORIENTATION_HORIZONTAL:
	case wxCSS_PROPERTY_STROKE_DASHOFFSET:
	case wxCSS_PROPERTY_STROKE_MITERLIMIT:
	case wxCSS_PROPERTY_STROKE_OPACITY:
	case wxCSS_PROPERTY_STROKE_WIDTH:
	case wxCSS_PROPERTY_OPACITY:
	case wxCSS_PROPERTY_STOP_OPACITY:
		if (!cssValue)
			cssValue = new wxCSSPrimitiveValue;
		((wxCSSPrimitiveValue*) cssValue)->SetFloatValue(wxCSS_NUMBER, ParseNumber(value));
		break;
	// 'none' or number
	case wxCSS_PROPERTY_FONT_SIZE_ADJUST:
		if (!cssValue)
			cssValue = new wxCSSPrimitiveValue;
		if (value == wxT("none"))
			((wxCSSPrimitiveValue*) cssValue)->SetIdentValue(wxCSS_VALUE_NONE);
		else
			((wxCSSPrimitiveValue*) cssValue)->SetFloatValue(wxCSS_NUMBER, ParseNumber(value));
		break;
	// 'auto' or number
	case wxCSS_PROPERTY_GLYPH_ORIENTATION_VERTICAL:
	case wxCSS_PROPERTY_KERNING:
	case wxCSS_PROPERTY_LETTER_SPACING:
		if (!cssValue)
			cssValue = new wxCSSPrimitiveValue;
		if (value == wxT("auto"))
			((wxCSSPrimitiveValue*) cssValue)->SetIdentValue(wxCSS_VALUE_AUTO);
		else
			((wxCSSPrimitiveValue*) cssValue)->SetFloatValue(wxCSS_NUMBER, ParseNumber(value));
		break;
		// 'normal' or number
	case wxCSS_PROPERTY_WORD_SPACING:
		if (!cssValue)
			cssValue = new wxCSSPrimitiveValue;
		if (value == wxT("normal"))
			((wxCSSPrimitiveValue*) cssValue)->SetIdentValue(wxCSS_VALUE_AUTO);
		else
			((wxCSSPrimitiveValue*) cssValue)->SetFloatValue(wxCSS_NUMBER, ParseNumber(value));
		break;
		// string
	case wxCSS_PROPERTY_FONT_FAMILY:
		if (!cssValue)
			cssValue = new wxCSSPrimitiveValue;
		((wxCSSPrimitiveValue*) cssValue)->SetStringValue(wxCSS_STRING, value);
		break;
		// <color>
	case wxCSS_PROPERTY_FLOOD_COLOR:
	case wxCSS_PROPERTY_LIGHTING_COLOR:
	case wxCSS_PROPERTY_STOP_COLOR:
		if (!cssValue)
			cssValue = new wxSVGColor;
		((wxSVGColor*) cssValue)->SetRGBColor(ParseColor(value));
		break;
		// <paint>
	case wxCSS_PROPERTY_FILL:
	case wxCSS_PROPERTY_STROKE:
		if (!cssValue)
			cssValue = new wxSVGPaint;
		ParseSVGPaint(*(wxSVGPaint*) cssValue, value);
		break;
		// <dasharray>
	case wxCSS_PROPERTY_STROKE_DASHARRAY:
		if (!cssValue)
			cssValue = new wxCSSValueList;
		((wxCSSValueList*) cssValue)->SetCSSText(value);
		break;
	}
	if (it == end())
		(*this)[propertyId] = cssValue;
}

void wxCSSStyleDeclaration::SetProperty(wxCSS_PROPERTY propertyId, const wxSVGAnimatedType& value) {
	if (value.GetPropertyType() != wxSVG_ANIMATED_LENGTH && value.GetPropertyType() != wxSVG_ANIMATED_COLOR) {
		SetProperty(propertyId, value.GetString());
		return;
	}
	if (propertyId == wxCSS_PROPERTY_UNKNOWN)
		return;
	wxCSSValue* cssValue = NULL;
	iterator it = find(propertyId);
	if (it != end())
		cssValue = it->second;
	if (value.GetPropertyType() == wxSVG_ANIMATED_LENGTH) {
		switch (propertyId) {
		// number
		case wxCSS_PROPERTY_FILL_OPACITY:
		case wxCSS_PROPERTY_FLOOD_OPACITY:
		case wxCSS_PROPERTY_FONT_SIZE:
		case wxCSS_PROPERTY_GLYPH_ORIENTATION_HORIZONTAL:
		case wxCSS_PROPERTY_STROKE_DASHOFFSET:
		case wxCSS_PROPERTY_STROKE_MITERLIMIT:
		case wxCSS_PROPERTY_STROKE_OPACITY:
		case wxCSS_PROPERTY_STROKE_WIDTH:
		case wxCSS_PROPERTY_OPACITY:
		case wxCSS_PROPERTY_STOP_OPACITY:
		case wxCSS_PROPERTY_GLYPH_ORIENTATION_VERTICAL:
		case wxCSS_PROPERTY_KERNING:
		case wxCSS_PROPERTY_LETTER_SPACING:
		case wxCSS_PROPERTY_WORD_SPACING:
			if (!cssValue)
				cssValue = new wxCSSPrimitiveValue;
			((wxCSSPrimitiveValue*) cssValue)->SetFloatValue(wxCSS_NUMBER, value.GetLength());
			break;
		default:
			break;
		}
	} else if (value.GetPropertyType() == wxSVG_ANIMATED_COLOR) {
		switch (propertyId) {
		case wxCSS_PROPERTY_COLOR:
			if (!cssValue)
				cssValue = new wxCSSPrimitiveValue;
			((wxCSSPrimitiveValue*) cssValue)->SetRGBColorValue(value.GetColor());
			break;
		// <color>
		case wxCSS_PROPERTY_FLOOD_COLOR:
		case wxCSS_PROPERTY_LIGHTING_COLOR:
		case wxCSS_PROPERTY_STOP_COLOR:
			if (!cssValue)
				cssValue = new wxSVGColor;
			((wxSVGColor*) cssValue)->SetRGBColor(value.GetColor());
			break;
		// <paint>
		case wxCSS_PROPERTY_FILL:
		case wxCSS_PROPERTY_STROKE:
			if (!cssValue)
				cssValue = new wxSVGPaint;
			((wxSVGPaint*) cssValue)->SetRGBColor(value.GetColor());
			break;
		default:
			break;
		}
	}
	if (it == end())
		(*this)[propertyId] = cssValue;
}

double wxCSSStyleDeclaration::ParseNumber(const wxString& value) {
	double val = 0;
	value.ToDouble(&val);
	return val;
}

static wxSortedArrayString* s_cssColors = NULL;
#include "css_colors.cpp"
inline void FillCSSColors() {
	if (s_cssColors == NULL) {
		s_cssColors = new wxSortedArrayString;
		for (unsigned int i = 0;
				i < sizeof(s_cssNamedColors) / sizeof(s_cssNamedColors[0]); i++)
			s_cssColors->Add(s_cssNamedColors[i].name);
	}
}

wxRGBColor wxCSSStyleDeclaration::ParseColor(const wxString& value) {
	if (!value.length() || value == wxT("none"))
		return wxRGBColor();
	else if (value.GetChar(0) == wxT('#')) {
		long r = 0, g = 0, b = 0, test;
		if (!value.Mid(4, 1).ToLong(&test, 16)) {
			value.Mid(1, 1).ToLong(&r, 16);
			value.Mid(2, 1).ToLong(&g, 16);
			value.Mid(3, 1).ToLong(&b, 16);
			return wxRGBColor((r << 4) | r, (g << 4) | g, (b << 4) | b);
		} else {
			value.Mid(1, 2).ToLong(&r, 16);
			value.Mid(3, 2).ToLong(&g, 16);
			value.Mid(5, 2).ToLong(&b, 16);
			return wxRGBColor(r, g, b);
		}
	} else if (value.Left(3) == wxT("rgb")) {
		wxStringTokenizer tkz(value.Mid(3), wxT(",()"));
		long rgb[3] = { 0, 0, 0 };
		for (int i = 0; tkz.HasMoreTokens() && i < 3;) {
			wxString token = tkz.GetNextToken().Strip(wxString::both);
			if (token.length())
				token.ToLong(&rgb[i++]);
		}
		return wxRGBColor(rgb[0], rgb[1], rgb[2]);
	} else {
		FillCSSColors();
		int num = s_cssColors->Index(value);
		if (num >= 0)
			return s_cssNamedColors[num].colour;
	}
	return wxRGBColor();
}

void wxCSSStyleDeclaration::ParseSVGPaint(wxSVGPaint& cssValue, const wxString& value) {
	wxString val = value;
	if (val.Left(3) == wxT("url")) {
		cssValue.SetUri(value.AfterFirst(wxT('(')).BeforeFirst(wxT(')')));
		val = value.AfterFirst(wxT(')')).Strip(wxString::both);
	}
	cssValue.SetRGBColor(ParseColor(val));
}

//////////////////////////////////////////////////////////////////////////////
///////////////////////// wxCSSStyleRef //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
wxCSSStyleRef::~wxCSSStyleRef() {
	while (size())
		erase(begin());
}

void wxCSSStyleRef::Add(const wxCSSStyleDeclaration& style) {
	const_iterator it;
	for (it = style.begin(); it != style.end(); ++it)
		(*this)[it->first] = it->second;
}

