//////////////////////////////////////////////////////////////////////////////
// Name:        SVGAnimateMotionElement.cpp
// Purpose:     Implementation of SVGAnimateMotionElement
// Author:      Alex Thuering
// Created:     2016-01-17
// RCS-ID:      $Id: SVGAnimateMotionElement.cpp,v 1.1 2016/01/29 16:40:01 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGAnimateMotionElement.h"
#include "SVGPointList.h"
#include <wx/log.h>

void wxSVGAnimateMotionElement::ApplyAnimation() {
	wxSVGElement* targetElement = GetTargetElement();
	if (targetElement == NULL || GetDur() <= 0 || m_values.size() < 2)
		return;
	wxSVGPointList points;
	for (unsigned int i = 0; i < m_values.size(); i++) {
		double x = 0, y = 0;
		wxString pointStr = m_values[i];
		if (pointStr.Find(wxT(',')) > 0
				&& pointStr.BeforeFirst(wxT(',')).Strip(wxString::both).ToDouble(&x)
				&& pointStr.AfterFirst(wxT(',')).Strip(wxString::both).ToDouble(&y))
			points.Add(wxSVGPoint(x, y));
	}
	if (points.size() < 2)
		return;
	wxString xAttr = GetTargetElement()->GetDtd() == wxSVG_CIRCLE_ELEMENT ? wxT("cx") : wxT("x");
	wxString yAttr = GetTargetElement()->GetDtd() == wxSVG_CIRCLE_ELEMENT ? wxT("cy") : wxT("y");
	double currTime = GetCurrentTime();
	if (m_repeatCount != 0 && m_repeatCount != 1 && currTime >= GetStartTime() + GetDur()) {
		int cnt = (currTime - GetStartTime())/ GetDur();
		if (m_repeatCount < 0 || m_repeatCount < cnt)
			currTime -= GetDur() * cnt;
	}
	if (currTime >= GetStartTime() + GetDur()) {
		targetElement->SetAnimatedValue(xAttr, wxSVGAnimatedType(wxSVGLength(points[points.size()-1].GetX())));
		targetElement->SetAnimatedValue(yAttr, wxSVGAnimatedType(wxSVGLength(points[points.size()-1].GetY())));
	} else if (currTime >= GetStartTime()) {
		double dur = GetDur() / (points.size() - 1);
		int idx = (currTime - GetStartTime())/dur;
		wxSVGPoint from = points[idx];
		wxSVGPoint to = points[idx + 1];
		double t = (currTime - GetStartTime())/dur - idx;
		targetElement->SetAnimatedValue(xAttr, wxSVGAnimatedType(wxSVGLength(from.GetX() + (to.GetX() - from.GetX())*t)));
		targetElement->SetAnimatedValue(yAttr, wxSVGAnimatedType(wxSVGLength(from.GetY() + (to.GetY() - from.GetY())*t)));
	}
}
