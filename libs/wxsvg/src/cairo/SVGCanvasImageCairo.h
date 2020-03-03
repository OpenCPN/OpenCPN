//////////////////////////////////////////////////////////////////////////////
// Name:        SVGCanvasImageCairo.h
// Purpose:     Cairo canvas image
// Author:      Alex Thuering
// Created:     2011/06/23
// RCS-ID:      $Id: SVGCanvasImageCairo.h,v 1.4 2013/09/12 08:46:01 ntalex Exp $
// Copyright:   (c) 2011 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#ifndef WX_SVG_CANVAS_IMAGE_CAIRO_H
#define WX_SVG_CANVAS_IMAGE_CAIRO_H

#include "SVGCanvasItem.h"
#include <cairo/cairo.h>

class wxSVGCanvasImageCairoData {
public:
	wxSVGCanvasImageCairoData(wxImage image);
	~wxSVGCanvasImageCairoData();
	
	void IncRef() { m_count++; }
	int DecRef() { return (--m_count); }
	
	inline cairo_surface_t* GetSurface() { return m_surface; }
	
private:
    int m_count;
	unsigned char* m_buffer;
	cairo_surface_t* m_surface;
};

class wxSVGCanvasImageCairo: public wxSVGCanvasImage {
public:
	wxSVGCanvasImageCairo();
	wxSVGCanvasImageCairo(wxSVGCanvasItemType type);
	virtual ~wxSVGCanvasImageCairo();
	virtual void Init(wxSVGImageElement& element, const wxCSSStyleDeclaration& style, wxProgressDialog* progressDlg);
	virtual cairo_surface_t* GetCairoSurface() { return m_data != NULL ? m_data->GetSurface() : NULL; }
	
private:
	wxSVGCanvasImageCairoData* m_data;
};

class wxSVGCanvasVideoCairo: public wxSVGCanvasVideo {
public:
	wxSVGCanvasVideoCairo();
	virtual ~wxSVGCanvasVideoCairo();
	virtual void Init(wxSVGVideoElement& element, const wxCSSStyleDeclaration& style, wxProgressDialog* progressDlg);
	virtual cairo_surface_t* GetCairoSurface() { return m_data != NULL ? m_data->GetSurface() : NULL; }
	
private:
	wxSVGCanvasImageCairoData* m_data;
};

#endif // WX_SVG_CANVAS_IMAGE_CAIRO_H
