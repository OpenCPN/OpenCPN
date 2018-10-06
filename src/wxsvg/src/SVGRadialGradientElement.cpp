//////////////////////////////////////////////////////////////////////////////
// Name:        SVGRadialGradientElement.cpp
// Author:      Alex Thuering
// Created:     2012/04/01
// RCS-ID:      $Id: SVGRadialGradientElement.cpp,v 1.1 2012/04/01 20:36:08 ntalex Exp $
// Copyright:   (c) 2012 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGRadialGradientElement.h"

double wxSVGRadialGradientElement::GetQualifiedR() const {
	return m_r.GetAnimVal().GetUnitType() != wxSVG_LENGTHTYPE_UNKNOWN ? m_r.GetAnimVal().GetValue() : 0.5;
}

double wxSVGRadialGradientElement::GetQualifiedCx() const {
	return m_cx.GetAnimVal().GetUnitType() != wxSVG_LENGTHTYPE_UNKNOWN ? m_cx.GetAnimVal().GetValue() : 0.5;
}

double wxSVGRadialGradientElement::GetQualifiedCy() const {
	return m_cy.GetAnimVal().GetUnitType() != wxSVG_LENGTHTYPE_UNKNOWN ? m_cy.GetAnimVal().GetValue() : 0.5;
}

double wxSVGRadialGradientElement::GetQualifiedFx() const {
	return m_fx.GetAnimVal().GetUnitType() != wxSVG_LENGTHTYPE_UNKNOWN ? m_fx.GetAnimVal().GetValue() : GetQualifiedCx();
}

double wxSVGRadialGradientElement::GetQualifiedFy() const {
	return m_fy.GetAnimVal().GetUnitType() != wxSVG_LENGTHTYPE_UNKNOWN ? m_fy.GetAnimVal().GetValue() : GetQualifiedCy();
}
