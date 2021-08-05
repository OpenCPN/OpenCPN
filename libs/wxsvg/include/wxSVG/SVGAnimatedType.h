//////////////////////////////////////////////////////////////////////////////
// Name:        SVGAnimatedType.h
// Author:      Alex Thuering
// Created:     2014/02/24
// RCS-ID:      $Id: SVGAnimatedType.h,v 1.2 2014/03/27 08:39:32 ntalex Exp $
// Copyright:   (c) 2014 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#ifndef WX_SVG_ANIMATED_TYPE_H
#define WX_SVG_ANIMATED_TYPE_H

#include "RGBColor.h"
#include "SVGLengthList.h"
#include "SVGNumberList.h"
#include "SVGTransformList.h"
#include "SVGRect.h"

enum wxSVG_ANIMATED_PROPERTY_TYPE {
	wxSVG_ANIMATED_COLOR,
	wxSVG_ANIMATED_LENGTH,
	wxSVG_ANIMATED_LENGTH_LIST,
	wxSVG_ANIMATED_NUMBER_LIST,
	wxSVG_ANIMATED_STRING,
	wxSVG_ANIMATED_TRANSFORM_LIST,
	wxSVG_ANIMATED_UNKNOWN
};

class wxSVGAnimatedType {
public:
	wxSVGAnimatedType(): m_propertyType(wxSVG_ANIMATED_UNKNOWN) {}
	wxSVGAnimatedType(const wxRGBColor& value) { m_propertyType = wxSVG_ANIMATED_COLOR; m_color = value; }
	wxSVGAnimatedType(const wxSVGLength& value) { m_propertyType = wxSVG_ANIMATED_LENGTH; m_length = value; }
	wxSVGAnimatedType(const wxSVGLengthList& value) { m_propertyType = wxSVG_ANIMATED_LENGTH_LIST; m_lengthList = value; }
	wxSVGAnimatedType(const wxSVGNumberList& value) { m_propertyType = wxSVG_ANIMATED_NUMBER_LIST; m_numberList = value; }
	wxSVGAnimatedType(const wxString& value) { m_propertyType = wxSVG_ANIMATED_STRING; m_string = value; }
	wxSVGAnimatedType(const wxSVGTransformList& value) { m_propertyType = wxSVG_ANIMATED_TRANSFORM_LIST; m_transformList = value; }
	~wxSVGAnimatedType();

	inline wxSVG_ANIMATED_PROPERTY_TYPE GetPropertyType() const { return m_propertyType; }
	inline const wxRGBColor& GetColor() const { return m_color; }
	inline const wxSVGLength& GetLength() const { return m_length; }
	inline const wxSVGLengthList& GetLengthList() const { return m_lengthList; }
	inline const wxSVGNumberList& GetNumberList() const { return m_numberList; }
	inline const wxString& GetString() const { return m_string; }
	inline const wxSVGTransformList& GetTransformList() const { return m_transformList; }

    wxString GetValueAsString() const;
    void SetValueAsString(const wxString& value);

private:
	wxSVG_ANIMATED_PROPERTY_TYPE m_propertyType;
	wxString m_string;
	wxRGBColor m_color;
	wxSVGLength m_length;
	wxSVGLengthList m_lengthList;
	wxSVGNumberList m_numberList;
	wxSVGTransformList m_transformList;
};

#endif // WX_SVG_ANIMATED_TYPE_H
