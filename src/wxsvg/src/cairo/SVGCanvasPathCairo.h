//////////////////////////////////////////////////////////////////////////////
// Name:        SVGCanvasPathCairo.h
// Purpose:     Cairo canvas path
// Author:      Alex Thuering
// Created:     2005/05/12
// RCS-ID:      $Id: SVGCanvasPathCairo.h,v 1.6 2013/01/19 18:26:28 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#ifndef WX_SVG_CANVAS_PATH_CAIRO_H
#define WX_SVG_CANVAS_PATH_CAIRO_H

#include "SVGCanvas.h"
#include <cairo/cairo.h>

class wxSVGCanvasPathCairo: public wxSVGCanvasPath {
protected:
	cairo_t* m_cr;
	double m_curx, m_cury, m_cubicx, m_cubicy, m_quadx, m_quady;

public:
	wxSVGCanvasPathCairo(cairo_surface_t* surface, wxSVGMatrix* matrix = NULL);
	virtual ~wxSVGCanvasPathCairo();
	
	void End();
	wxSVGRect GetBBox(const wxSVGMatrix& matrix = *(wxSVGMatrix*) NULL);
	wxSVGRect GetResultBBox(const wxCSSStyleDeclaration& style, const wxSVGMatrix& matrix = *(wxSVGMatrix*) NULL);
	
	cairo_t* GetCr() { return m_cr; }
	cairo_path_t* GetPath();
	
	void MoveToImpl(double x, double y);
	void LineToImpl(double x, double y);
	void CurveToCubicImpl(double x1, double y1, double x2, double y2, double x, double y);
	bool ClosePathImpl();
	
	static void ApplyStrokeStyle(cairo_t* cr, const wxCSSStyleDeclaration& style);
};

#endif // WX_SVG_CANVAS_PATH_CAIRO_H
