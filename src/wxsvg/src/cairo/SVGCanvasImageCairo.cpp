//////////////////////////////////////////////////////////////////////////////
// Name:        SVGCanvasImageCairo.cpp
// Purpose:     Cairo canvas image
// Author:      Alex Thuering
// Created:     2011/06/23
// RCS-ID:      $Id: SVGCanvasImageCairo.cpp,v 1.6 2013/09/12 08:46:01 ntalex Exp $
// Copyright:   (c) 2011 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGCanvasImageCairo.h"
#include <wx/log.h>

wxSVGCanvasImageCairoData::wxSVGCanvasImageCairoData(wxImage image) {
	m_count = 1;
	
	int bw = image.GetWidth();
	int bh = image.GetHeight();
	m_buffer = new unsigned char[bw*bh*4];
	wxUint32* data = (wxUint32*) m_buffer;
	
	// Create a surface object and copy the bitmap pixel data to it
	unsigned char* srcPix = image.GetData();
	if (image.HasAlpha()) { // alpha 
		m_surface = cairo_image_surface_create_for_data(m_buffer, CAIRO_FORMAT_ARGB32, bw, bh, bw*4);
		unsigned char* srcAlpha = image.GetAlpha();
		for (int y = 0; y < bh; y++) {
			for (int x = 0; x < bw; x++) {
				// Each pixel in CAIRO_FORMAT_ARGB32 is a 32-bit quantity, with alpha in the upper 8 bits,
				// then red, then green, then blue. The 32-bit quantities are stored native-endian.
				// Pre-multiplied alpha is used.
				unsigned char alpha = srcAlpha != NULL ? (*srcAlpha) : 255;
				if (alpha == 0)
					*data = 0;
				else
					*data = (alpha << 24 | (srcPix[0] * alpha / 255) << 16 | (srcPix[1] * alpha / 255) << 8
							| (srcPix[2] * alpha / 255));
				data++;
				srcPix += 3;
				if (srcAlpha)
					srcAlpha++;
			}
		}
	} else { // no alpha
		m_surface = cairo_image_surface_create_for_data(m_buffer, CAIRO_FORMAT_RGB24, bw, bh, bw * 4);
		for (int y = 0; y < bh; y++) {
			for (int x = 0; x < bw; x++) {
				// Each pixel in CAIRO_FORMAT_RGB24 is a 32-bit quantity, with the upper 8 bits unused.
				// Red, Green, and Blue are stored in the remaining 24 bits in that order.
				// The 32-bit quantities are stored native-endian.
				*data = (srcPix[0] << 16 | srcPix[1] << 8 | srcPix[2]);
				data++;
				srcPix += 3;
			}
		}
	}
}

wxSVGCanvasImageCairoData::~wxSVGCanvasImageCairoData() {
	cairo_surface_destroy(m_surface);
	delete[] m_buffer;
}

wxSVGCanvasImageCairo::wxSVGCanvasImageCairo(): wxSVGCanvasImage() {
	m_data = NULL;
}

wxSVGCanvasImageCairo::wxSVGCanvasImageCairo(wxSVGCanvasItemType type): wxSVGCanvasImage(type) {
	m_data = NULL;
}

wxSVGCanvasImageCairo::~wxSVGCanvasImageCairo() {
	if (m_data != NULL && m_data->DecRef() == 0)
		delete m_data;
}

void wxSVGCanvasImageCairo::Init(wxSVGImageElement& element, const wxCSSStyleDeclaration& style,
		wxProgressDialog* progressDlg) {
	wxSVGCanvasImage::Init(element, style, progressDlg);
	
	wxSVGCanvasImageCairo* prevItem = (wxSVGCanvasImageCairo*) element.GetCanvasItem();
	if (prevItem != NULL && prevItem->m_href == m_href && prevItem->m_data != NULL) {
		m_data = prevItem->m_data;
		m_data->IncRef();
		return;
	}
	
	if (!m_image.Ok())
		return;
	
	m_data = new wxSVGCanvasImageCairoData(m_image); 
}


wxSVGCanvasVideoCairo::wxSVGCanvasVideoCairo(): wxSVGCanvasVideo() {
	m_data = NULL;
}

wxSVGCanvasVideoCairo::~wxSVGCanvasVideoCairo() {
	if (m_data != NULL && m_data->DecRef() == 0)
		delete m_data;
}

void wxSVGCanvasVideoCairo::Init(wxSVGVideoElement& element, const wxCSSStyleDeclaration& style,
		wxProgressDialog* progressDlg) {
	wxSVGCanvasVideo::Init(element, style, progressDlg);
	
	wxSVGCanvasVideoCairo* prevItem = (wxSVGCanvasVideoCairo*) element.GetCanvasItem();
	if (prevItem != NULL && prevItem->m_href == m_href && prevItem->m_time == m_time && prevItem->m_data != NULL) {
		m_data = prevItem->m_data;
		m_data->IncRef();
		return;
	}
	
	if (!m_image.Ok())
		return;
	
	m_data = new wxSVGCanvasImageCairoData(m_image); 
}
