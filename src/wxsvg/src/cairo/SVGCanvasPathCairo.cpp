//////////////////////////////////////////////////////////////////////////////
// Name:        SVGCanvasPathCairo.cpp
// Purpose:     Cairo canvas path
// Author:      Alex Thuering
// Created:     2005/05/12
// RCS-ID:      $Id: SVGCanvasPathCairo.cpp,v 1.14 2014/11/23 11:36:16 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGCanvasPathCairo.h"
#include <wx/log.h>

wxSVGCanvasPathCairo::wxSVGCanvasPathCairo(cairo_surface_t* surface, wxSVGMatrix* matrix) {
	m_cr = cairo_create(surface);
#if 0 //!defined(__WXMAC__) && !defined(__WXMSW__)
	if (matrix) {
		 // for correct checking of surface size when text is rendered using PangaCairo
		cairo_matrix_t m;
		cairo_matrix_init(&m, matrix->GetA(), matrix->GetB(), matrix->GetC(), matrix->GetD(), matrix->GetE(), matrix->GetF());
		cairo_set_matrix(m_cr, &m);
	};
#endif
	m_curx = m_cury = m_cubicx = m_cubicy = m_quadx = m_quady = 0;
}

wxSVGCanvasPathCairo::~wxSVGCanvasPathCairo() {
	cairo_destroy(m_cr);
}

void wxSVGCanvasPathCairo::End() {
}

cairo_path_t* wxSVGCanvasPathCairo::GetPath() {
	return cairo_copy_path(m_cr);
}

wxSVGRect wxSVGCanvasPathCairo::GetBBox(const wxSVGMatrix& matrix) {
	if (&matrix) {
		cairo_matrix_t m;
		cairo_matrix_init(&m, matrix.GetA(), matrix.GetB(), matrix.GetC(), matrix.GetD(), matrix.GetE(), matrix.GetF());
		cairo_set_matrix(m_cr, &m);
	}
	double x1, y1, x2, y2;
	cairo_fill_extents(m_cr, &x1, &y1, &x2, &y2);
	if (&matrix) {
		cairo_matrix_t mat;
		cairo_matrix_init(&mat, 1, 0, 0, 1, 0, 0);
		cairo_set_matrix(m_cr, &mat);
	}
	return wxSVGRect(x1, y1, x2 - x1, y2 - y1);
}

wxSVGRect wxSVGCanvasPathCairo::GetResultBBox(const wxCSSStyleDeclaration& style, const wxSVGMatrix& matrix) {
	if (&matrix) {
		cairo_matrix_t m;
		cairo_matrix_init(&m, matrix.GetA(), matrix.GetB(), matrix.GetC(), matrix.GetD(), matrix.GetE(), matrix.GetF());
		cairo_set_matrix(m_cr, &m);
	}
	ApplyStrokeStyle(m_cr, style);
	double x1, y1, x2, y2;
	if (style.GetStrokeWidth() > 0)
		cairo_stroke_extents(m_cr, &x1, &y1, &x2, &y2);
	else
		cairo_fill_extents(m_cr, &x1, &y1, &x2, &y2);
	if (&matrix) {
		cairo_matrix_t mat;
		cairo_matrix_init(&mat, 1, 0, 0, 1, 0, 0);
		cairo_set_matrix(m_cr, &mat);
	}
	return wxSVGRect(x1, y1, x2 - x1, y2 - y1);
}

void wxSVGCanvasPathCairo::MoveToImpl(double x, double y) {
	cairo_move_to(m_cr, x, y);
}

void wxSVGCanvasPathCairo::LineToImpl(double x, double y) {
	cairo_line_to(m_cr, x, y);
}

void wxSVGCanvasPathCairo::CurveToCubicImpl(double x1, double y1, double x2,
		double y2, double x, double y) {
	cairo_curve_to(m_cr, x1, y1, x2, y2, x, y);
}

bool wxSVGCanvasPathCairo::ClosePathImpl() {
	cairo_close_path(m_cr);
	return true;
}

void wxSVGCanvasPathCairo::ApplyStrokeStyle(cairo_t* cr, const wxCSSStyleDeclaration& style) {
	cairo_set_line_width(cr, style.GetStrokeWidth());
	switch (style.GetStrokeLinecap()) {
	case wxCSS_VALUE_ROUND:
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
		break;
	case wxCSS_VALUE_SQUARE:
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
		break;
	case wxCSS_VALUE_BUTT:
	default:
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
		break;
	}
	switch (style.GetStrokeLinejoin()) {
	case wxCSS_VALUE_BEVEL:
		cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
		break;
	case wxCSS_VALUE_ROUND:
		cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
		break;
	case wxCSS_VALUE_MITER:
	default:
		cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);
		break;
	}
	if (style.GetStrokeDasharray().GetLength() > 0) {
		double* dashed = new double[style.GetStrokeDasharray().GetLength()];
		for (int i = 0; i < style.GetStrokeDasharray().GetLength(); i++) {
			dashed[i] = style.GetStrokeDasharray().Item(i).GetFloatValue();
		}
		cairo_set_dash(cr, dashed, style.GetStrokeDasharray().GetLength(), 0.0);
		delete[] dashed;
	} else
		cairo_set_dash(cr, NULL, 0, 0);
}
