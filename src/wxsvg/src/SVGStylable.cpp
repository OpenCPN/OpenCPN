//////////////////////////////////////////////////////////////////////////////
// Name:        SVGStylable.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/04/29
// RCS-ID:      $Id: SVGStylable.cpp,v 1.8 2014/03/27 08:42:16 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGStylable.h"
#include "svg.h"
#include <wx/log.h>

const wxCSSValue& wxSVGStylable::GetPresentationAttribute(const wxString& name) {
	return m_style.GetPropertyCSSValue(name);
}

bool wxSVGStylable::HasCustomAttribute(const wxString& name) const {
	return wxCSSStyleDeclaration::GetPropertyId(name) != wxCSS_PROPERTY_UNKNOWN;
}

wxString wxSVGStylable::GetCustomAttribute(const wxString& name) const {
	return m_style.GetPropertyValue(name);
}

bool wxSVGStylable::SetCustomAttribute(const wxString& name, const wxString& value) {
	wxCSS_PROPERTY id = wxCSSStyleDeclaration::GetPropertyId(name);

	if (id == wxCSS_PROPERTY_UNKNOWN)
		return false;

	m_style.SetProperty(name, value);
	return true;
}

wxSvgXmlAttrHash wxSVGStylable::GetCustomAttributes() const {
	wxSvgXmlAttrHash attrs;
	return attrs;
}

bool wxSVGStylable::SetCustomAnimatedValue(const wxString& name, const wxSVGAnimatedType& value) {
	wxCSS_PROPERTY id = wxCSSStyleDeclaration::GetPropertyId(name);
	
	if (id == wxCSS_PROPERTY_UNKNOWN)
		return false;
	
	m_style.SetProperty(name, value);
	return true;
}

wxCSSStyleDeclaration wxSVGStylable::GetResultStyle(const wxSVGElement& element) {
	wxCSSStyleDeclaration style;
	if (element.GetParent())
		style = GetResultStyle(*(wxSVGElement*) element.GetParent());
	const wxSVGStylable* stylable = wxSVGStylable::GetSVGStylable(element);
	if (stylable) {
		style.Add(stylable->GetStyle());
		style.Add(stylable->GetAnimStyle());
	}
	return style;
}

#define GET_STYLABLE(the_dtd, the_class)\
case the_dtd:\
  return &((the_class&) element);
  
wxSVGStylable* wxSVGStylable::GetSVGStylable(wxSVGElement& element) {
  if (element.GetType() != wxSVGXML_ELEMENT_NODE)
    return NULL;
  switch (element.GetDtd()) {
    GET_STYLABLE(wxSVG_A_ELEMENT, wxSVGAElement)
    GET_STYLABLE(wxSVG_ALTGLYPH_ELEMENT, wxSVGAltGlyphElement)
    GET_STYLABLE(wxSVG_CIRCLE_ELEMENT, wxSVGCircleElement)
    GET_STYLABLE(wxSVG_CLIPPATH_ELEMENT, wxSVGClipPathElement)
    GET_STYLABLE(wxSVG_DEFS_ELEMENT, wxSVGDefsElement)
    GET_STYLABLE(wxSVG_DESC_ELEMENT, wxSVGDescElement)
    GET_STYLABLE(wxSVG_ELLIPSE_ELEMENT, wxSVGEllipseElement)
    GET_STYLABLE(wxSVG_FILTER_ELEMENT, wxSVGFilterElement)
    GET_STYLABLE(wxSVG_FONT_ELEMENT, wxSVGFontElement)
    GET_STYLABLE(wxSVG_FOREIGNOBJECT_ELEMENT, wxSVGForeignObjectElement)
    GET_STYLABLE(wxSVG_G_ELEMENT, wxSVGGElement)
    GET_STYLABLE(wxSVG_GLYPH_ELEMENT, wxSVGGlyphElement)
    GET_STYLABLE(wxSVG_GLYPHREF_ELEMENT, wxSVGGlyphRefElement)
    GET_STYLABLE(wxSVG_LINEARGRADIENT_ELEMENT, wxSVGLinearGradientElement)
    GET_STYLABLE(wxSVG_RADIALGRADIENT_ELEMENT, wxSVGRadialGradientElement)
    GET_STYLABLE(wxSVG_IMAGE_ELEMENT, wxSVGImageElement)
    GET_STYLABLE(wxSVG_LINE_ELEMENT, wxSVGLineElement)
    GET_STYLABLE(wxSVG_MARKER_ELEMENT, wxSVGMarkerElement)
    GET_STYLABLE(wxSVG_MASK_ELEMENT, wxSVGMaskElement)
    GET_STYLABLE(wxSVG_MISSING_GLYPH_ELEMENT, wxSVGMissingGlyphElement)
    GET_STYLABLE(wxSVG_PATH_ELEMENT, wxSVGPathElement)
    GET_STYLABLE(wxSVG_PATTERN_ELEMENT, wxSVGPatternElement)
    GET_STYLABLE(wxSVG_POLYGON_ELEMENT, wxSVGPolygonElement)
    GET_STYLABLE(wxSVG_POLYLINE_ELEMENT, wxSVGPolylineElement)
    GET_STYLABLE(wxSVG_RECT_ELEMENT, wxSVGRectElement)
    GET_STYLABLE(wxSVG_SVG_ELEMENT, wxSVGSVGElement)
    GET_STYLABLE(wxSVG_STOP_ELEMENT, wxSVGStopElement)
    GET_STYLABLE(wxSVG_SWITCH_ELEMENT, wxSVGSwitchElement)
    GET_STYLABLE(wxSVG_SYMBOL_ELEMENT, wxSVGSymbolElement)
    GET_STYLABLE(wxSVG_TITLE_ELEMENT, wxSVGTitleElement)
    GET_STYLABLE(wxSVG_TEXT_ELEMENT, wxSVGTextElement)
    GET_STYLABLE(wxSVG_TEXTPATH_ELEMENT, wxSVGTextPathElement)
    GET_STYLABLE(wxSVG_TREF_ELEMENT, wxSVGTRefElement)
    GET_STYLABLE(wxSVG_TSPAN_ELEMENT, wxSVGTSpanElement)
    GET_STYLABLE(wxSVG_USE_ELEMENT, wxSVGUseElement)
    GET_STYLABLE(wxSVG_FEBLEND_ELEMENT, wxSVGFEBlendElement)
    GET_STYLABLE(wxSVG_FECOLORMATRIX_ELEMENT, wxSVGFEColorMatrixElement)
    GET_STYLABLE(wxSVG_FECOMPONENTTRANSFER_ELEMENT, wxSVGFEComponentTransferElement)
    GET_STYLABLE(wxSVG_FECOMPOSITE_ELEMENT, wxSVGFECompositeElement)
    GET_STYLABLE(wxSVG_FECONVOLVEMATRIX_ELEMENT, wxSVGFEConvolveMatrixElement)
    GET_STYLABLE(wxSVG_FEDIFFUSELIGHTING_ELEMENT, wxSVGFEDiffuseLightingElement)
    GET_STYLABLE(wxSVG_FEDISPLACEMENTMAP_ELEMENT, wxSVGFEDisplacementMapElement)
    GET_STYLABLE(wxSVG_FEFLOOD_ELEMENT, wxSVGFEFloodElement)
    GET_STYLABLE(wxSVG_FEGAUSSIANBLUR_ELEMENT, wxSVGFEGaussianBlurElement)
    GET_STYLABLE(wxSVG_FEMERGE_ELEMENT, wxSVGFEMergeElement)
    GET_STYLABLE(wxSVG_FEMORPHOLOGY_ELEMENT, wxSVGFEMorphologyElement)
    GET_STYLABLE(wxSVG_FEOFFSET_ELEMENT, wxSVGFEOffsetElement)
    GET_STYLABLE(wxSVG_FESPECULARLIGHTING_ELEMENT, wxSVGFESpecularLightingElement)
    GET_STYLABLE(wxSVG_FETILE_ELEMENT, wxSVGFETileElement)
    GET_STYLABLE(wxSVG_FETURBULENCE_ELEMENT, wxSVGFETurbulenceElement)
    default:
      break;
  }
  return NULL;
}

const wxSVGStylable* wxSVGStylable::GetSVGStylable(const wxSVGElement& element) {
	return GetSVGStylable((wxSVGElement&) element);
}

const wxCSSStyleDeclaration& wxSVGStylable::GetElementStyle(const wxSVGElement& element) {
	static wxCSSStyleDeclaration emptyStyle;
	const wxSVGStylable* stylable = GetSVGStylable(element);
	if (!stylable)
		return emptyStyle;
	return stylable->GetStyle();
}
