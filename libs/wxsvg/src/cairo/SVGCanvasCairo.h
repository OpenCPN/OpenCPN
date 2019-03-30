//////////////////////////////////////////////////////////////////////////////
// Name:        SVGCanvasCairo.h
// Purpose:     Cairo render
// Author:      Alex Thuering
// Created:     2005/05/12
// RCS-ID:      $Id: SVGCanvasCairo.h,v 1.15 2014/08/09 11:16:21 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#ifndef WX_SVG_CANVAS_CAIRO_H
#define WX_SVG_CANVAS_CAIRO_H

#include "SVGCanvas.h"
#include <cairo/cairo.h>

class wxSVGCanvasPathCairo;
class wxSVGCanvasTextCairo;
class wxSVGCanvasImageCairo;

class wxSVGCanvasCairo: public wxSVGCanvas {
public:
	wxSVGCanvasCairo(): m_cr(NULL), m_surface(NULL), m_pattern(NULL), m_alpha(false) { }
	virtual ~wxSVGCanvasCairo();
	
	void Init(int width, int height, bool alpha = false);
    int GetWidth();
    int GetHeight();
    wxImage GetImage();
	void Clear(wxRGBColor color = wxRGBColor(0xFF,0xFF,0xFF));
	
	wxSVGCanvasPath* CreateCanvasPath(wxSVGMatrix* matrix = NULL);
    wxSVGCanvasItem* CreateItem(wxSVGTextElement* element, const wxCSSStyleDeclaration* style = NULL,
    		wxSVGMatrix* matrix = NULL);
	wxSVGCanvasItem* CreateItem(wxSVGImageElement* element, const wxCSSStyleDeclaration* style = NULL,
			wxProgressDialog* progressDlg = NULL);
	wxSVGCanvasItem* CreateItem(wxSVGVideoElement* element, const wxCSSStyleDeclaration* style = NULL,
			wxProgressDialog* progressDlg = NULL);
    
	void DrawItem(wxSVGCanvasItem& item, wxSVGMatrix& matrix, const wxCSSStyleDeclaration& style,
			wxSVGSVGElement& svgElem);
  
protected:
	void DrawCanvasPath(wxSVGCanvasPathCairo& canvasPath, wxSVGMatrix& matrix, const wxCSSStyleDeclaration& style,
			wxSVGSVGElement& svgElem);
    void SetStopValue(unsigned int index, float offset, float opacity, const wxRGBColor& rgbColor);
	void AllocateGradientStops(unsigned int stop_count);
	void DrawCanvasImage(wxSVGCanvasImage& canvasImage, cairo_surface_t* cairoSurface,
			wxSVGMatrix& matrix, const wxCSSStyleDeclaration& style, wxSVGSVGElement& svgElem);

private:
	cairo_t* m_cr;
	cairo_surface_t* m_surface;
	cairo_pattern_t* m_pattern;
    bool m_alpha;
    void Destroy();
    void SetPaint(cairo_t* cr,const wxSVGPaint & paint, float opacity, wxSVGCanvasPathCairo& canvasPath,
    		wxSVGSVGElement& svgElem, const wxSVGMatrix& matrix);
	void DrawPath(cairo_t* cr, wxSVGCanvasPathCairo& canvasPath, const wxSVGMatrix& matrix,
			const wxCSSStyleDeclaration& style, wxSVGSVGElement& svgElem);
	void SetMatrix(cairo_t* cr, const wxSVGMatrix& matrix);
	void SetClipPath(wxSVGElement* clipPathElem, wxSVGMatrix matrix);
	void DrawMask(cairo_t* cr, wxSVGElement* maskElem, const wxSVGMatrix& matrix, const wxCSSStyleDeclaration& style,
			wxSVGSVGElement& svgElem);
	void DrawMarker(const wxString& uri, wxSVGMark::Type type, wxSVGCanvasPathCairo& canvasPath,
			const wxSVGMatrix& matrix, const wxCSSStyleDeclaration& style, wxSVGSVGElement& svgElem);
};

#endif // WX_SVG_CANVAS_CAIRO_H
