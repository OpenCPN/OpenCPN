//////////////////////////////////////////////////////////////////////////////
// Name:        SVGRectElement.cpp
// Purpose:     Implementation of wxSVGRectElement
// Author:      Alex Thuering
// Created:     2005/05/10
// RCS-ID:      $Id: SVGRectElement.cpp,v 1.6 2014/03/24 21:16:35 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGRectElement.h"
#include "SVGCanvas.h"
#include "SVGAnimatedPoints.h"

#include "math.h"

const double pi = 3.1415926;

wxSVGRect wxSVGRectElement::GetBBox(wxSVG_COORDINATES coordinates) {
	WX_SVG_CREATE_M_CANVAS_ITEM
	wxSVGRect bbox = coordinates == wxSVG_COORDINATES_USER ? m_canvasItem->GetBBox() :
			m_canvasItem->GetBBox(GetMatrix(coordinates));
	WX_SVG_CLEAR_M_CANVAS_ITEM
	return bbox;
}

wxSVGRect wxSVGRectElement::GetResultBBox(wxSVG_COORDINATES coordinates) {
	wxCSSStyleDeclaration style = GetResultStyle(*this);
	if (style.GetStroke().GetPaintType() == wxSVG_PAINTTYPE_NONE)
		return GetBBox(coordinates);
	WX_SVG_CREATE_M_CANVAS_ITEM
	wxSVGRect bbox = coordinates == wxSVG_COORDINATES_USER ? m_canvasItem->GetResultBBox(style) :
			m_canvasItem->GetResultBBox(style, GetMatrix(coordinates));
	WX_SVG_CLEAR_M_CANVAS_ITEM
	return bbox;
}

void wxSVGRectElement::SetCanvasItem(wxSVGCanvasItem* canvasItem) {
	if (m_canvasItem)
		delete m_canvasItem;
	m_canvasItem = canvasItem;
}
