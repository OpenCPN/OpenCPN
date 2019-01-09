//////////////////////////////////////////////////////////////////////////////
// Name:        SVGCanvasTextCairo.h
// Purpose:     Cairo canvas text
// Author:      Alex Thuering
// Created:     2011/06/23
// RCS-ID:      $Id: SVGCanvasTextCairo.cpp,v 1.13 2014/06/16 19:40:46 ntalex Exp $
// Copyright:   (c) 2011 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGCanvasTextCairo.h"
#include "SVGCanvasPathCairo.h"
#include <wx/log.h>
#include <wx/font.h>
#include <wx/tokenzr.h>
#include <cairo/cairo.h>
#if defined(__WXMSW__)
#include <cairo/cairo-win32.h>
#elif defined(__WXMAC__) 
#include <cairo/cairo-quartz.h>
#else
#include <pango/pangocairo.h>
#include <glib-object.h>
#endif

wxSVGCanvasTextCairo::wxSVGCanvasTextCairo(wxSVGCanvas* canvas): wxSVGCanvasText(canvas) {
}

wxSVGCanvasTextCairo::~wxSVGCanvasTextCairo() {
}

void wxSVGCanvasTextCairo::InitText(const wxString& text, const wxCSSStyleDeclaration& style, wxSVGMatrix* matrix) {
	BeginChar(matrix);
	
	// create path from text
	cairo_t* cr = ((wxSVGCanvasPathCairo*) m_char->path)->GetCr();
	
#if defined(__WXMSW__) || defined(__WXMAC__)
	int size = (int) style.GetFontSize();
	int fstyle = style.GetFontStyle() == wxCSS_VALUE_ITALIC ? wxFONTSTYLE_ITALIC
			: (style.GetFontStyle() == wxCSS_VALUE_OBLIQUE ? wxFONTSTYLE_SLANT : wxFONTSTYLE_NORMAL);
	wxFontWeight weight = style.GetFontWeight() == wxCSS_VALUE_BOLD ? wxFONTWEIGHT_BOLD
			: style.GetFontWeight() == wxCSS_VALUE_BOLDER ? wxFONTWEIGHT_MAX
			: style.GetFontWeight() == wxCSS_VALUE_LIGHTER ? wxFONTWEIGHT_LIGHT : wxFONTWEIGHT_NORMAL;
	wxFont fnt(size, wxFONTFAMILY_DEFAULT, fstyle, weight, false, style.GetFontFamily());
#ifdef __WXMSW__
	HFONT hfont = (HFONT) fnt.GetResourceHandle();
	cairo_set_font_face(cr, cairo_win32_font_face_create_for_hfont(hfont));
#else
	CGFontRef cgFont = fnt.OSXGetCGFont();
	cairo_set_font_face(cr, cairo_quartz_font_face_create_for_cgfont(cgFont));
#endif	
	cairo_set_font_size(cr, style.GetFontSize());
	
	cairo_font_extents_t fextents;
	cairo_font_extents(cr, &fextents);
	
	double maxWidth = 0;
	if (style.GetTextAnchor() == wxCSS_VALUE_MIDDLE || style.GetTextAnchor() == wxCSS_VALUE_END) {
		wxStringTokenizer tokenzr(text, wxT("\n"));
		while (tokenzr.HasMoreTokens()) {
			wxString token = tokenzr.GetNextToken();
			cairo_text_extents_t extents;
			cairo_text_extents(cr, (const char*) token.utf8_str(), &extents);
			if (maxWidth < extents.width)
				maxWidth = extents.width;
		}
	}
	
	wxStringTokenizer tokenzr(text, wxT("\n"));
	double x_advance = 0;
	double width = 0;
	double height = 0;
	double y = 0;
	while (tokenzr.HasMoreTokens()) {
		wxString token = tokenzr.GetNextToken();
		
		// get text extents
		cairo_text_extents_t extents;
		cairo_text_extents(cr, (const char*) token.utf8_str(), &extents);
		double x = style.GetTextAnchor() == wxCSS_VALUE_END ? maxWidth - extents.width
				: style.GetTextAnchor() == wxCSS_VALUE_MIDDLE ? (maxWidth - extents.width) / 2 : 0;
		
		m_char->path->MoveTo(m_tx + x, m_ty + y);
		cairo_text_path(cr, (const char*) token.utf8_str());
		
		if (x_advance < extents.x_advance)
			x_advance = extents.x_advance;
		if (width < extents.width)
			width = extents.width;
		height += fextents.height;
		if (tokenzr.HasMoreTokens())
			y += fextents.height;
	}
	
	// set bbox
	m_char->bbox = wxSVGRect(m_tx, m_ty, width, height);
	
	// increase current position (m_tx)
	if (style.GetTextAnchor() == wxCSS_VALUE_MIDDLE || style.GetTextAnchor() == wxCSS_VALUE_END) {
		wxSVGRect bbox = m_char->path->GetResultBBox(style);
		m_tx += x_advance > bbox.GetWidth() ? x_advance : bbox.GetWidth();
	} else
		m_tx += x_advance;
#else
	PangoLayout* layout = pango_cairo_create_layout(cr);
	PangoFontDescription* font = pango_font_description_new();
	pango_font_description_set_family(font, style.GetFontFamily().ToAscii());
	pango_font_description_set_weight(font, style.GetFontWeight() == wxCSS_VALUE_BOLD ? PANGO_WEIGHT_BOLD
			: PANGO_WEIGHT_NORMAL);
	pango_font_description_set_style(font, style.GetFontStyle() == wxCSS_VALUE_ITALIC ? PANGO_STYLE_ITALIC
			: (style.GetFontStyle() == wxCSS_VALUE_OBLIQUE ? PANGO_STYLE_OBLIQUE : PANGO_STYLE_NORMAL));
	pango_font_description_set_absolute_size(font, style.GetFontSize() * PANGO_SCALE);

	PangoContext* ctx = pango_layout_get_context(layout);
	PangoFont* f = pango_context_load_font(ctx, font);
	if (f == NULL)
		pango_font_description_set_style(font, PANGO_STYLE_NORMAL);
	pango_layout_set_font_description(layout, font);
	
	if (style.GetTextAnchor() != wxCSS_VALUE_START)
		pango_layout_set_alignment(layout, style.GetTextAnchor() == wxCSS_VALUE_MIDDLE ? PANGO_ALIGN_CENTER : PANGO_ALIGN_RIGHT);
	pango_layout_set_text(layout, (const char*) text.utf8_str(), -1);
	
	int baseline = pango_layout_get_baseline(layout);
	m_char->path->MoveTo(m_tx, m_ty - ((double)baseline / PANGO_SCALE));
	pango_cairo_layout_path(cr, layout);
	
	// set bbox and increase current position (m_tx)
	int lwidth, lheight;
	pango_layout_get_size(layout, &lwidth, &lheight);
	double width = ((double)lwidth / PANGO_SCALE);
	double height = ((double)lheight / PANGO_SCALE);
	m_char->bbox = wxSVGRect(m_tx, m_ty, width, height);
	if (style.GetTextAnchor() == wxCSS_VALUE_MIDDLE || style.GetTextAnchor() == wxCSS_VALUE_END) {
		wxSVGRect bbox = m_char->path->GetResultBBox(style);
		m_tx += width > bbox.GetWidth() ? width : bbox.GetWidth();
	} else
		m_tx += width;
		
	g_object_unref(layout);
	pango_font_description_free(font);
#endif
}
