//////////////////////////////////////////////////////////////////////////////
// Name:        SVGAnimationElement.cpp
// Purpose:     Implementation of wxSVGAnimationElement
// Author:      Alex Thuering
// Created:     2005/05/10
// RCS-ID:      $Id: SVGAnimationElement.cpp,v 1.8 2016/01/29 16:40:01 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGAnimationElement.h"
#include "SVGDocument.h"
#include "SVGSVGElement.h"
#include <wx/log.h>
#include <vector>

using namespace std;

double wxSVGAnimationElement::GetStartTime() {
  return m_begin;
}

double wxSVGAnimationElement::GetCurrentTime() {
	wxSVGDocument* doc = (wxSVGDocument*) GetOwnerDocument();
	return doc != NULL ? doc->GetCurrentTime() : 0;
}

double wxSVGAnimationElement::GetSimpleDuration() {
	return m_dur;
}

bool wxSVGAnimationElement::HasCustomAttribute(const wxString& name) const {
	return name == wxT("repeatCount") || name == wxT("values");
}

wxString wxSVGAnimationElement::GetCustomAttribute(const wxString& name) const {
	if (name == wxT("repeatCount"))
		return m_repeatCount < 0 ? wxT("indefinite") : wxString::Format(wxT("%d"), m_repeatCount);
	else if (name == wxT("values"))
		return m_values.GetValueAsString();
	return wxT("");
}

bool wxSVGAnimationElement::SetCustomAttribute(const wxString& name, const wxString& value) {
	long lvalue;
	if (name == wxT("repeatCount")) {
		if (value == wxT("indefinite")) {
			m_repeatCount = -1;
		} else if (value.ToLong(&lvalue)) {
			m_repeatCount = lvalue;
		}
		return true;
	} else if (name == wxT("values")) {
		m_values.SetValueAsString(value, wxT(';'));
	}
	return false;
}

wxSvgXmlAttrHash wxSVGAnimationElement::GetCustomAttributes() const {
	wxSvgXmlAttrHash attrs;
	if (m_repeatCount != 0 && m_repeatCount != 1)
		attrs.Add(wxT("repeatCount"), GetCustomAttribute(wxT("repeatCount")));
	if (m_values.size() > 0)
		attrs.Add(wxT("values"), GetCustomAttribute(wxT("values")));
	return attrs;
}

wxSVGElement* wxSVGAnimationElement::GetTargetElement() const {
	if (m_href.length() && GetOwnerSVGElement()) {
		return (wxSVGElement*) GetOwnerSVGElement()->GetElementById(m_href);
	}
	return (wxSVGElement*) GetParent();
}

void wxSVGAnimationElement::ApplyAnimation() {
	wxSVGElement* targetElement = GetTargetElement();
	if (targetElement == NULL || GetDur() <= 0)
		return;
	double currTime = GetCurrentTime();
	if (m_repeatCount != 0 && m_repeatCount != 1 && currTime >= GetStartTime() + GetDur()) {
		int cnt = (currTime - GetStartTime())/ GetDur();
		if (m_repeatCount < 0 || m_repeatCount < cnt)
			currTime -= GetDur() * cnt;
	}
	
	vector<wxSVGAnimatedType> values;
	if (m_values.size() > 0) {
		for (unsigned int i = 0; i < m_values.size(); i++) {
			wxSVGAnimatedType val;
			val.SetValueAsString(m_values[i]);
			values.push_back(val);
		}
	} else {
		values.push_back(GetFrom());
		values.push_back(GetTo());
	}
	if (values.size() < 2)
		return;
	
	if (currTime >= GetStartTime() + GetDur()) {
		targetElement->SetAnimatedValue(GetAttributeName(), values[values.size() - 1]);
	} else if (currTime >= GetStartTime()) {
		double dur = GetDur() / (values.size() - 1);
		int idx = (currTime - GetStartTime())/dur;
		wxSVGAnimatedType from = values[idx];
		wxSVGAnimatedType to = values[idx + 1];
		double t = (currTime - GetStartTime())/dur - idx;
		
		if (from.GetPropertyType() == wxSVG_ANIMATED_LENGTH && to.GetPropertyType() == wxSVG_ANIMATED_LENGTH) {
			wxSVGAnimatedType value(wxSVGLength(to.GetLength().GetUnitType(), from.GetLength().GetValue()
					+ (to.GetLength().GetValue() - from.GetLength().GetValue()) * t));
			targetElement->SetAnimatedValue(GetAttributeName(), value);
		} else if (from.GetPropertyType() == wxSVG_ANIMATED_COLOR
				&& to.GetPropertyType() == wxSVG_ANIMATED_COLOR) {
			double r1 = from.GetColor().Red();
			double g1 = from.GetColor().Green();
			double b1 = from.GetColor().Blue();
			double r2 = to.GetColor().Red();
			double g2 = to.GetColor().Green();
			double b2 = to.GetColor().Blue();
			wxSVGAnimatedType value(wxRGBColor(r1 + (r2 - r1) * t, g1 + (g2 - g1) * t, b1 + (b2 - b1) * t));
			targetElement->SetAnimatedValue(GetAttributeName(), value);
		}
	}
}
