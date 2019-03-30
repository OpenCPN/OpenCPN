//////////////////////////////////////////////////////////////////////////////
// Name:        SVGTransform.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/04/29
// RCS-ID:      $Id: SVGTransform.cpp,v 1.8 2015/03/21 18:20:28 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGTransform.h"
#include <wx/math.h>
#include <wx/tokenzr.h>

void wxSVGTransform::SetTranslate(double tx, double ty) {
	m_type = wxSVG_TRANSFORM_TRANSLATE;
	m_angle = 0;
	m_matrix = wxSVGMatrix(1, 0, 0, 1, tx, ty);
}

void wxSVGTransform::SetScale(double sx, double sy) {
	m_type = wxSVG_TRANSFORM_SCALE;
	m_angle = 0;
	m_matrix = wxSVGMatrix(sx, 0, 0, sy, 0, 0);
}

void wxSVGTransform::SetRotate(double angle, double cx, double cy) {
	m_type = wxSVG_TRANSFORM_ROTATE;
	m_angle = angle;
	m_cx = cx;
	m_cy = cy;
	if (cx == 0 && cy == 0) {
		angle = angle * M_PI / 180;
		m_matrix = wxSVGMatrix(cos(angle), sin(angle), -sin(angle), cos(angle), 0, 0);
	} else {
		angle = angle * M_PI / 180;
		m_matrix = wxSVGMatrix(cos(angle), sin(angle), -sin(angle), cos(angle), cx, cy);
		m_matrix = m_matrix.Translate(-cx, -cy);
	}
}

void wxSVGTransform::SetSkewX(double angle) {
	m_type = wxSVG_TRANSFORM_SKEWX;
	m_angle = angle;
	m_matrix = wxSVGMatrix(1, 0, tan(angle * M_PI / 180), 1, 0, 0);
}

void wxSVGTransform::SetSkewY(double angle) {
	m_type = wxSVG_TRANSFORM_SKEWY;
	m_angle = angle;
	m_matrix = wxSVGMatrix(1, tan(angle * M_PI / 180), 0, 1, 0, 0);
}

wxString wxSVGTransform::GetValueAsString() const {
	wxString value;
	switch (m_type) {
	case wxSVG_TRANSFORM_UNKNOWN:
		break;
	case wxSVG_TRANSFORM_MATRIX:
		value = wxString::Format(wxT("matrix(%g,%g,%g,%g,%g,%g)"),
				m_matrix.GetA(), m_matrix.GetB(), m_matrix.GetC(),
				m_matrix.GetD(), m_matrix.GetE(), m_matrix.GetF());
		break;
	case wxSVG_TRANSFORM_TRANSLATE:
		value = wxString::Format(wxT("translate(%g,%g)"), m_matrix.GetE(), m_matrix.GetF());
		break;
	case wxSVG_TRANSFORM_SCALE:
		if (m_matrix.GetA() == m_matrix.GetD())
			value = wxString::Format(wxT("scale(%g)"), m_matrix.GetA());
		else
			value = wxString::Format(wxT("scale(%g,%g)"), m_matrix.GetA(), m_matrix.GetD());
		break;
	case wxSVG_TRANSFORM_ROTATE:
		if (m_cx == 0 && m_cy == 0)
			value = wxString::Format(wxT("rotate(%g)"), m_angle);
		else
//      double a = m_angle*M_PI/180;
//      double cosa = cos(a);
//      double sina = sin(a);
//      double e = m_matrix.GetE();
//      double f = m_matrix.GetF();
//      double cx = cosa != 1 ? (e*(1-cosa) - f*sina)/(1-cosa)/2 : 0;
//      double cy = cosa != 1 ? e*sina/(1-cosa)/2 + f/2 : 0;
			value = wxString::Format(wxT("rotate(%g,%g,%g)"), m_angle, m_cx, m_cy);
		break;
	case wxSVG_TRANSFORM_SKEWX:
		value = wxString::Format(wxT("skewX(%g)"), m_angle);
		break;
	case wxSVG_TRANSFORM_SKEWY:
		value = wxString::Format(wxT("skewY(%g)"), m_angle);
		break;
	}
	return value;
}

void wxSVGTransform::SetValueAsString(const wxString& value) {
	if (value.length() == 0)
		return;
	double params[6] = { 0, 0, 0, 0, 0, 0 };
	wxStringTokenizer tkz(value.AfterFirst(wxT('(')).BeforeLast(wxT(')')),
			wxT(","));
	int pi = 0;
	while (tkz.HasMoreTokens() && pi < 6) {
		tkz.GetNextToken().ToDouble(&params[pi]);
		pi++;
	}
	if (pi == 0)
		return;
	if (value.substr(0, 9) == wxT("translate"))
		SetTranslate(params[0], params[1]);
	else if (value.substr(0, 5) == wxT("scale"))
		SetScale(params[0], pi == 1 ? params[0] : params[1]);
	else if (value.substr(0, 6) == wxT("rotate"))
		SetRotate(params[0], params[1], params[2]);
	else if (value.substr(0, 5) == wxT("skewX"))
		SetSkewX(params[0]);
	else if (value.substr(0, 5) == wxT("skewY"))
		SetSkewY(params[0]);
	else if (value.substr(0, 6) == wxT("matrix"))
		SetMatrix(wxSVGMatrix(params[0], params[1], params[2], params[3], params[4], params[5]));
}
