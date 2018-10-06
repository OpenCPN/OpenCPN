//////////////////////////////////////////////////////////////////////////////
// Name:        SVGEllipseElement.cpp
// Purpose:     Implementation of wxSVGEllipseElement
// Author:      Alex Thuering
// Created:     2005/05/10
// RCS-ID:      $Id: SVGEllipseElement.cpp,v 1.6 2014/03/24 21:16:35 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGEllipseElement.h"
#include "SVGCanvas.h"
#include "SVGAnimatedPoints.h"

#include "math.h"

const double pi = 3.1415926;

wxSVGRect wxSVGEllipseElement::GetBBox(wxSVG_COORDINATES coordinates) {
	if (coordinates == wxSVG_COORDINATES_USER)
		return wxSVGRect(GetCx().GetBaseVal() - GetRx().GetBaseVal(), GetCy().GetBaseVal() - GetRy().GetBaseVal(),
				2 * GetRx().GetBaseVal(), 2 * GetRy().GetBaseVal());

	wxSVGMatrix matrix = GetMatrix(coordinates);

	double angles[4];
	angles[0] = atan((GetRy().GetBaseVal() * matrix.GetC()) / (GetRx().GetBaseVal() * matrix.GetA()));
	angles[1] = atan((GetRy().GetBaseVal() * matrix.GetD()) / (GetRx().GetBaseVal() * matrix.GetB()));
	angles[2] = angles[0] + pi;
	angles[3] = angles[1] + pi;

	wxSVGPointList points = wxSVGPointList();
	for (int i = 0; i < 4; i++) {
		points.Add(wxSVGPoint(GetRx().GetBaseVal() * cos(angles[i]) + GetCx().GetBaseVal(),
				GetRy().GetBaseVal() * sin(angles[i]) + GetCy().GetBaseVal()));
	}

	wxSVGPoint p0 = points[0].MatrixTransform(matrix);
	wxSVGRect bbox(p0.GetX(), p0.GetY(), 0, 0);

	wxSVGPoint pi = wxSVGPoint();
	for (int i = 1; i < (int) points.Count(); i++) {
		pi = points[i].MatrixTransform(matrix);
		if (bbox.GetX() > pi.GetX()) {
			bbox.SetWidth(bbox.GetWidth() + bbox.GetX() - pi.GetX());
			bbox.SetX(pi.GetX());
		}
		if (bbox.GetY() > pi.GetY()) {
			bbox.SetHeight(bbox.GetHeight() + bbox.GetY() - pi.GetY());
			bbox.SetY(pi.GetY());
		}

		if (bbox.GetX() + bbox.GetWidth() < pi.GetX())
			bbox.SetWidth(pi.GetX() - bbox.GetX());
		if (bbox.GetY() + bbox.GetHeight() < pi.GetY())
			bbox.SetHeight(pi.GetY() - bbox.GetY());
	}

	return bbox;
}

wxSVGRect wxSVGEllipseElement::GetResultBBox(wxSVG_COORDINATES coordinates) {
	wxSVGRect bbox = GetBBox(coordinates);
	if (GetStroke().GetPaintType() == wxSVG_PAINTTYPE_NONE)
		return bbox;

	double strokeWidthX = GetStrokeWidth();
	double strokeWidthY = strokeWidthX;
	if (coordinates != wxSVG_COORDINATES_USER) {
		wxSVGMatrix matrix = GetMatrix(coordinates);
		strokeWidthX *= matrix.GetA();
		strokeWidthY *= matrix.GetD();
	}

	return wxSVGRect(bbox.GetX() - strokeWidthX / 2, bbox.GetY() - strokeWidthY / 2, bbox.GetWidth() + strokeWidthX,
			bbox.GetHeight() + strokeWidthY);
}

void wxSVGEllipseElement::SetCanvasItem(wxSVGCanvasItem* canvasItem) {
	if (m_canvasItem)
		delete m_canvasItem;
	m_canvasItem = canvasItem;
}
