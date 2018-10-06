//////////////////////////////////////////////////////////////////////////////
// Name:        SVGCanvas.cpp
// Purpose:     wxSVGCanvas - Base class for SVG renders (backends)
// Author:      Alex Thuering
// Created:     2005/05/04
// RCS-ID:      $Id: SVGCanvas.cpp,v 1.27 2016/01/09 23:31:14 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGCanvas.h"
#include <math.h>
#include <wx/log.h>

//////////////////////////////////////////////////////////////////////////////
////////////////////////// Create item functions /////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define WX_SVG_CREATE_USING_PATH(elem_name)\
wxSVGCanvasItem* wxSVGCanvas::CreateItem(wxSVG##elem_name##Element* element) {\
	wxSVGCanvasPath* canvasItem = CreateCanvasPath();\
	canvasItem->Init(*element);\
	return canvasItem;\
}

WX_SVG_CREATE_USING_PATH(Line)
WX_SVG_CREATE_USING_PATH(Polyline)
WX_SVG_CREATE_USING_PATH(Polygon)
WX_SVG_CREATE_USING_PATH(Rect)
WX_SVG_CREATE_USING_PATH(Circle)
WX_SVG_CREATE_USING_PATH(Ellipse)
WX_SVG_CREATE_USING_PATH(Path)

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// Draw functions ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define WX_SVG_DRAW(elem_name)\
void wxSVGCanvas::Draw##elem_name(wxSVG##elem_name##Element* element, wxSVGMatrix* matrix,\
		const wxCSSStyleDeclaration* style) {\
	wxSVGCanvasItem* canvasItem = CreateItem(element);\
	if (style == NULL)\
		style = &element->GetStyle();\
	if (style->GetDisplay() == wxCSS_VALUE_INLINE)\
		DrawItem(*canvasItem, *matrix, *style, *element->GetOwnerSVGElement());\
	if (IsItemsCached())\
		element->SetCanvasItem(canvasItem);\
	else\
		delete canvasItem;\
}

WX_SVG_DRAW(Line)
WX_SVG_DRAW(Polyline)
WX_SVG_DRAW(Polygon)
WX_SVG_DRAW(Rect)
WX_SVG_DRAW(Circle)
WX_SVG_DRAW(Ellipse)
WX_SVG_DRAW(Path)

void wxSVGCanvas::DrawImage(wxSVGImageElement* element, wxSVGMatrix* matrix, const wxCSSStyleDeclaration* style,
		const wxSVGRect* rect, wxProgressDialog* progressDlg) {
	wxSVGCanvasImage* canvasItem = (wxSVGCanvasImage*) CreateItem(element, NULL, progressDlg);
	if (style == NULL)
		style = &element->GetStyle();
	if (style->GetDisplay() == wxCSS_VALUE_INLINE) {
		if (canvasItem->GetSvgImage() != NULL) {
			wxSVGGElement* gElem = new wxSVGGElement();
			gElem->SetOwnerDocument(element->GetOwnerDocument());
			gElem->SetOwnerSVGElement(element->GetOwnerSVGElement());
			gElem->Translate(canvasItem->m_x, canvasItem->m_y);
			wxSVGSVGElement* svgElem = canvasItem->GetSvgImage((wxSVGDocument*) element->GetOwnerDocument());
			svgElem->SetWidth(canvasItem->m_width);
			svgElem->SetHeight(canvasItem->m_height);
			gElem->AddChild(svgElem);
			RenderElement(gElem, rect, matrix, style, element->GetOwnerSVGElement(), element->GetViewportElement(),
					progressDlg);
			gElem->RemoveChild(gElem->GetFirstChild());
			delete gElem;
		} else
			DrawItem(*canvasItem, *matrix, *style, *element->GetOwnerSVGElement());
	}
	if (IsItemsCached())
		element->SetCanvasItem(canvasItem);
	else
		delete canvasItem;
}

void wxSVGCanvas::DrawVideo(wxSVGVideoElement* element, wxSVGMatrix* matrix, const wxCSSStyleDeclaration* style,
		wxProgressDialog* progressDlg) {
	wxSVGCanvasItem* canvasItem = CreateItem(element, style, progressDlg);
	if (style == NULL)
		style = &element->GetStyle();
	if (style->GetDisplay() == wxCSS_VALUE_INLINE)
		DrawItem(*canvasItem, *matrix, *style, *element->GetOwnerSVGElement());
	if (IsItemsCached())
		element->SetCanvasItem(canvasItem);
	else
		delete canvasItem;
}


void wxSVGCanvas::DrawText(wxSVGTextElement* element,
  wxSVGMatrix* matrix, const wxCSSStyleDeclaration* style)
{
  if (style == NULL)
	style = &element->GetStyle();
  wxSVGCanvasItem* canvasItem = CreateItem(element, style, matrix);
  if (style->GetDisplay() == wxCSS_VALUE_INLINE)
  	DrawItem(*canvasItem, *matrix, *style, *element->GetOwnerSVGElement());
  if (IsItemsCached())
	element->SetCanvasItem(canvasItem);
  else
	delete canvasItem;
}

void wxSVGCanvas::DrawCanvasText(wxSVGCanvasText& canvasText,
  wxSVGMatrix& matrix, const wxCSSStyleDeclaration& style, wxSVGSVGElement& svgElem)
{
  for (int i=0; i<(int)canvasText.m_chunks.Count(); i++)
  {
	wxSVGCanvasTextChunk& chunk = canvasText.m_chunks[i];
	chunk.style.SetFillRule(wxCSS_VALUE_EVENODD);
	wxSVGMatrix pathMatrix = matrix.Multiply(chunk.matrix);
	for (unsigned int j=0; j<chunk.chars.Count(); j++)
		DrawItem(*chunk.chars[j].path, pathMatrix, chunk.style, svgElem);
  }
}

wxSVGPatternElement* wxSVGCanvas::GetPatternElement(const wxSVGSVGElement& svgElem, const wxString& href) {
	if (href.length() == 0 || href[0] != wxT('#') || &svgElem == NULL)
	    return NULL;
	wxSVGElement* elem = (wxSVGElement*) svgElem.GetElementById(href.substr(1));
	return elem != NULL && elem->GetDtd() == wxSVG_PATTERN_ELEMENT ? (wxSVGPatternElement*) elem : NULL;
}

wxSVGMarkerElement* wxSVGCanvas::GetMarkerElement(const wxSVGSVGElement& svgElem, const wxString& href) {
	if (href.length() == 0 || href[0] != wxT('#') || &svgElem == NULL)
		return NULL;
	wxSVGElement* elem = (wxSVGElement*) svgElem.GetElementById(href.substr(1));
	return elem != NULL && elem->GetDtd() == wxSVG_MARKER_ELEMENT ? (wxSVGMarkerElement*) elem : NULL;
}

wxSVGGradientElement* wxSVGCanvas::GetGradientElement(const wxSVGSVGElement& svgElem, const wxString& href) {
	if (href.length() == 0 || href[0] != wxT('#') || &svgElem == NULL)
	    return NULL;
	wxSVGGradientElement* elem = (wxSVGGradientElement*) svgElem.GetElementById(href.substr(1));
	return elem != NULL && (elem->GetDtd() == wxSVG_LINEARGRADIENT_ELEMENT
			|| elem->GetDtd() == wxSVG_RADIALGRADIENT_ELEMENT) ? elem : NULL;
}

unsigned int wxSVGCanvas::GetGradientStops(const wxSVGSVGElement& svgElem, wxSVGGradientElement* gradElem, float opacity) {
	if (gradElem == NULL)
		return 0;

	// Search for the most referenced gradient (we assume that this is the one that contain stops)
	while (true) {
		wxString grad_href = gradElem->GetHref();
		if (grad_href.length() <= 1 || grad_href.GetChar(0) != wxT('#'))
			break;
		wxSVGElement* tmp = (wxSVGSVGElement*) svgElem.GetElementById(grad_href.substr(1));
		if (tmp == NULL || (tmp->GetDtd() != wxSVG_LINEARGRADIENT_ELEMENT
				&& tmp->GetDtd() != wxSVG_RADIALGRADIENT_ELEMENT))
			break;
		gradElem = (wxSVGGradientElement*) tmp;
	}

	// Count number of stop element
	wxSVGStopElement* stop_elem = (wxSVGStopElement*) gradElem->GetChildren();
	unsigned int stop_count = 0;
	while (stop_elem) {
		if (stop_elem->GetDtd() == wxSVG_STOP_ELEMENT)
			stop_count++;
		stop_elem = (wxSVGStopElement*) stop_elem->GetNext();
	}
	if (stop_count == 0)
		return 0;

	// Allocate enough stops
	AllocateGradientStops(stop_count);
	
	// Fill the stops
	stop_elem = (wxSVGStopElement*) gradElem->GetChildren();
	int i = 0;
	while (stop_elem) {
		if (stop_elem->GetDtd() == wxSVG_STOP_ELEMENT) {
			wxSVGColor color = stop_elem->GetStopColor();
 			// no color, default is black
 			if (color.GetColorType() == wxSVG_COLORTYPE_UNKNOWN)
				color = wxSVGColor(0,0,0);
			SetStopValue(i++, stop_elem->GetOffset(), stop_elem->GetStopOpacity() * opacity,
					color.GetRGBColor());
		}
		stop_elem = (wxSVGStopElement*) stop_elem->GetNext();
	}
	return stop_count;
}

void wxSVGCanvas::GetLinearGradientVector(wxSVGPoint& p1, wxSVGPoint& p2, const wxSVGLinearGradientElement& gradElem,
		wxSVGCanvasPath& path) {
    p1.SetX(gradElem.GetX1().GetAnimVal());
	p1.SetY(gradElem.GetY1().GetAnimVal());
	p2.SetX(gradElem.GetX2().GetAnimVal());
	p2.SetY(gradElem.GetY2().GetAnimVal());
    
    if (gradElem.GetGradientUnits().GetAnimVal() == wxSVG_UNIT_TYPE_UNKNOWN ||
        gradElem.GetGradientUnits().GetAnimVal() == wxSVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    {
        wxSVGRect bbox = path.GetBBox();
        p1.SetX(bbox.GetX() + p1.GetX()*bbox.GetWidth());
        p1.SetY(bbox.GetY() + p1.GetY()*bbox.GetHeight());
        p2.SetX(bbox.GetX() + p2.GetX()*bbox.GetWidth());
        p2.SetY(bbox.GetY() + p2.GetY()*bbox.GetHeight());
    }
    
	// Compute gradient transformation matrix
    wxSVGMatrix lg_matrix;
	const wxSVGTransformList& transforms =  gradElem.GetGradientTransform().GetAnimVal();
	for (int i=0; i<(int)transforms.Count(); i++)
        lg_matrix = lg_matrix.Multiply(transforms[i].GetMatrix());
	
	p1 = p1.MatrixTransform(lg_matrix);
	p2 = p2.MatrixTransform(lg_matrix);
}


void wxSVGCanvas::GetRadialGradientTransform(wxSVGPoint& focus,
  wxSVGMatrix& matrix, const wxSVGRadialGradientElement& gradElem,
  wxSVGCanvasPath& path, bool scale)
{
    double r, cx, cy, fx, fy;
	r = gradElem.GetR().GetAnimVal();
	cx = gradElem.GetCx().GetAnimVal();
	cy = gradElem.GetCy().GetAnimVal();
	fx = gradElem.GetFx().GetAnimVal();
	fy = gradElem.GetFy().GetAnimVal();
    
    if (gradElem.GetGradientUnits().GetAnimVal() == wxSVG_UNIT_TYPE_UNKNOWN ||
        gradElem.GetGradientUnits().GetAnimVal() == wxSVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    {
        wxSVGRect bbox = path.GetBBox();
        r = r*sqrt(bbox.GetWidth()*bbox.GetWidth() + bbox.GetHeight()*bbox.GetHeight());
        cx = bbox.GetX() + cx*bbox.GetWidth();
        cy = bbox.GetY() + cy*bbox.GetHeight();
        fx = bbox.GetX() + fx*bbox.GetWidth();
        fy = bbox.GetY() + fy*bbox.GetHeight();
    }

	// Compute gradient transformation matrix
	const wxSVGTransformList& transforms =  gradElem.GetGradientTransform().GetAnimVal();
	for (int i=0; i<(int)transforms.Count(); i++)
		matrix = matrix.Multiply(transforms[i].GetMatrix());
	
	// Apply Gradient parameters to transformation
	matrix = matrix.Translate(cx, cy); //.Scale(r);
    if (scale)
      matrix = matrix.Scale(r);
	
	// Change Focus reference to gradient reference
	focus.SetX((fx - cx) / r);
	focus.SetY((fy - cy) / r);
}
//////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Render /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void wxSVGCanvas::LoadImages(wxSVGElement* parent1, wxSVGElement* parent2, wxProgressDialog* progressDlg) {
	wxSVGElement* elem1 = (wxSVGElement*) parent1->GetChildren();
	wxSVGElement* elem2 = (wxSVGElement*) parent2->GetChildren();
	while (elem1 && elem2) {
		if (elem1->GetType() == wxSVGXML_ELEMENT_NODE && elem1->GetDtd() == wxSVG_IMAGE_ELEMENT
				&& elem2->GetType() == wxSVGXML_ELEMENT_NODE && elem2->GetDtd() == wxSVG_IMAGE_ELEMENT) {
			wxSVGImageElement* imgElem1 = (wxSVGImageElement*) elem1;
			if (imgElem1->GetHref().GetAnimVal().length()) {
				if (imgElem1->GetCanvasItem() == NULL
						|| ((wxSVGCanvasImage*) imgElem1->GetCanvasItem())->m_href != imgElem1->GetHref().GetAnimVal())
					imgElem1->SetCanvasItem(CreateItem(imgElem1, NULL, progressDlg));
				((wxSVGImageElement*) elem2)->SetCanvasItem(CreateItem(imgElem1, NULL, progressDlg));
			}
		} else if (elem1->GetType() == wxSVGXML_ELEMENT_NODE && elem1->GetDtd() == wxSVG_VIDEO_ELEMENT
				&& elem2->GetType() == wxSVGXML_ELEMENT_NODE && elem2->GetDtd() == wxSVG_VIDEO_ELEMENT) {
			wxSVGVideoElement* vElem1 = (wxSVGVideoElement*) elem1;
			if (vElem1->GetHref().GetAnimVal().length()) {
				if (vElem1->GetCanvasItem() == NULL
						|| ((wxSVGCanvasVideo*) vElem1->GetCanvasItem())->m_href != vElem1->GetHref().GetAnimVal())
					vElem1->SetCanvasItem(CreateItem(vElem1, NULL, progressDlg));
				((wxSVGVideoElement*) elem2)->SetCanvasItem(CreateItem(vElem1, NULL, progressDlg));
			}
		} else if (elem1->GetChildren())
			LoadImages(elem1, elem2, progressDlg);
		elem1 = (wxSVGElement*) elem1->GetNext();
		elem2 = (wxSVGElement*) elem2->GetNext();
	}
}

void wxSVGCanvas::RenderElement(wxSVGElement* elem, const wxSVGRect* rect, const wxSVGMatrix* parentMatrix,
		const wxCSSStyleDeclaration* parentStyle, wxSVGSVGElement* ownerSVGElement, wxSVGElement* viewportElement,
		wxProgressDialog* progressDlg) {
	wxSVGMatrix matrix(*parentMatrix);
	wxCSSStyleRef style(*parentStyle);
	elem->SetOwnerSVGElement(ownerSVGElement);
	elem->SetViewportElement(viewportElement);

	switch (elem->GetDtd()) {
	case wxSVG_SVG_ELEMENT: {
		wxSVGSVGElement* element = (wxSVGSVGElement*) elem;
		if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
			break;
		if (element->GetWidth().GetAnimVal().GetUnitType() == wxSVG_LENGTHTYPE_UNKNOWN)
			((wxSVGAnimatedLength&) element->GetWidth()).SetAnimVal( wxSVGLength(wxSVG_LENGTHTYPE_PERCENTAGE, 100));
		if (element->GetHeight().GetAnimVal().GetUnitType() == wxSVG_LENGTHTYPE_UNKNOWN)
			((wxSVGAnimatedLength&) element->GetHeight()).SetAnimVal( wxSVGLength(wxSVG_LENGTHTYPE_PERCENTAGE, 100));
		element->UpdateMatrix(matrix);
		style.Add(element->GetStyle());
		style.Add(element->GetAnimStyle());
		if (rect && element->GetParent()) {
			wxSVGRect rect2 = *rect;
			wxSVGElement* parent = (wxSVGElement*) element->GetParent();
			wxSVGTransformable* transformable = wxSVGTransformable::GetSVGTransformable(*parent);
			if (transformable)
				rect2 = rect2.MatrixTransform(transformable->GetCTM().Inverse());
			RenderChilds(elem, &rect2, &matrix, &style, element, element, progressDlg);
		} else
			RenderChilds(elem, rect, &matrix, &style, element, element, progressDlg);
		break;
	}
	case wxSVG_G_ELEMENT: {
		wxSVGGElement* element = (wxSVGGElement*) elem;
		if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
			break;
		element->UpdateMatrix(matrix);
		style.Add(element->GetStyle());
		style.Add(element->GetAnimStyle());
		RenderChilds(elem, rect, &matrix, &style, ownerSVGElement, viewportElement, progressDlg);
		break;
	}
	case wxSVG_LINE_ELEMENT: {
		wxSVGLineElement* element = (wxSVGLineElement*) elem;
		if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
			break;
		element->UpdateMatrix(matrix);
		style.Add(element->GetStyle());
		style.Add(element->GetAnimStyle());
		DrawLine(element, &matrix, &style);
		break;
	}
	case wxSVG_POLYLINE_ELEMENT: {
		wxSVGPolylineElement* element = (wxSVGPolylineElement*) elem;
		if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
			break;
		element->UpdateMatrix(matrix);
		style.Add(element->GetStyle());
		style.Add(element->GetAnimStyle());
		DrawPolyline(element, &matrix, &style);
		break;
	}
	case wxSVG_POLYGON_ELEMENT: {
		wxSVGPolygonElement* element = (wxSVGPolygonElement*) elem;
		if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
			break;
		element->UpdateMatrix(matrix);
		style.Add(element->GetStyle());
		style.Add(element->GetAnimStyle());
		DrawPolygon(element, &matrix, &style);
		break;
	}
	case wxSVG_RECT_ELEMENT: {
		wxSVGRectElement* element = (wxSVGRectElement*) elem;
		if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
			break;
		element->UpdateMatrix(matrix);
		style.Add(element->GetStyle());
		style.Add(element->GetAnimStyle());
		DrawRect(element, &matrix, &style);
		break;
	}
	case wxSVG_CIRCLE_ELEMENT: {
		wxSVGCircleElement* element = (wxSVGCircleElement*) elem;
		if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
			break;
		element->UpdateMatrix(matrix);
		style.Add(element->GetStyle());
		style.Add(element->GetAnimStyle());
		DrawCircle(element, &matrix, &style);
		break;
	}
	case wxSVG_ELLIPSE_ELEMENT: {
		wxSVGEllipseElement* element = (wxSVGEllipseElement*) elem;
		if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
			break;
		element->UpdateMatrix(matrix);
		style.Add(element->GetStyle());
		style.Add(element->GetAnimStyle());
		DrawEllipse(element, &matrix, &style);
		break;
	}
	case wxSVG_PATH_ELEMENT: {
		wxSVGPathElement* element = (wxSVGPathElement*) elem;
		if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
			break;
		element->UpdateMatrix(matrix);
		style.Add(element->GetStyle());
		style.Add(element->GetAnimStyle());
		DrawPath(element, &matrix, &style);
		break;
	}
	case wxSVG_TSPAN_ELEMENT:
		break;
	case wxSVG_TEXT_ELEMENT: {
		wxSVGTextElement* element = (wxSVGTextElement*) elem;
		if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
			break;
		element->UpdateMatrix(matrix);
		style.Add(element->GetStyle());
		style.Add(element->GetAnimStyle());
		DrawText(element, &matrix, &style);
		break;
	}
	case wxSVG_IMAGE_ELEMENT: {
		wxSVGImageElement* element = (wxSVGImageElement*) elem;
		if (element->GetVisibility() == wxCSS_VALUE_HIDDEN || element->GetOpacity() == 0)
			break;
		element->UpdateMatrix(matrix);
		style.Add(element->GetStyle());
		style.Add(element->GetAnimStyle());
		DrawImage(element, &matrix, &style, rect, progressDlg);
		break;
	}
	case wxSVG_VIDEO_ELEMENT: {
		wxSVGVideoElement* element = (wxSVGVideoElement*) elem;
		if (element->GetVisibility() == wxCSS_VALUE_HIDDEN || element->GetOpacity() == 0)
			break;
		element->UpdateMatrix(matrix);
		style.Add(element->GetStyle());
		style.Add(element->GetAnimStyle());
#ifdef USE_LIBAV
		DrawVideo(element, &matrix, &style);
#else
		wxSVGGElement* gElem = new wxSVGGElement();
		gElem->SetOwnerSVGElement(ownerSVGElement);
		gElem->SetViewportElement(viewportElement);
		gElem->SetStyle(element->GetStyle());
		wxSVGRectElement* rectElem = new wxSVGRectElement();
		rectElem->SetX(element->GetX().GetAnimVal());
		rectElem->SetY(element->GetY().GetAnimVal());
		rectElem->SetWidth(element->GetWidth().GetAnimVal());
		rectElem->SetHeight(element->GetHeight().GetAnimVal());
		rectElem->SetFill(wxSVGPaint(0,0,0));
		gElem->AppendChild(rectElem);
		wxSVGTextElement* textElem = new wxSVGTextElement;
		textElem->SetX((double)element->GetX().GetAnimVal());
		textElem->SetY(element->GetY().GetAnimVal()+(double)element->GetHeight().GetAnimVal()/10);
		textElem->SetFontSize((double)element->GetHeight().GetAnimVal()/15);
		textElem->SetFill(wxSVGPaint(255,255,255));
		textElem->SetStroke(wxSVGPaint(255,255,255));
		textElem->AddChild(new wxSvgXmlNode(wxSVGXML_TEXT_NODE, wxT(""), wxT(" [") + element->GetHref() + wxT("]")));
		gElem->AppendChild(textElem);

		// render
		RenderElement(gElem, rect, &matrix, &style, ownerSVGElement, viewportElement, progressDlg);
		// delete shadow tree
		delete gElem;
#endif
		break;
	}
	case wxSVG_USE_ELEMENT: {
		wxSVGUseElement* element = (wxSVGUseElement*) elem;
		if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
			break;
		element->UpdateMatrix(matrix);
		style.Add(element->GetStyle());
		style.Add(element->GetAnimStyle());
		// test if visible
		if (element->GetWidth().GetAnimVal().GetUnitType() != wxSVG_LENGTHTYPE_UNKNOWN
				&& element->GetHeight().GetAnimVal().GetUnitType() != wxSVG_LENGTHTYPE_UNKNOWN) {
			if (rect && !ownerSVGElement->CheckIntersection(*elem, *rect))
				break;
			wxSVGPoint point(element->GetX().GetAnimVal() + element->GetWidth().GetAnimVal(),
					element->GetY().GetAnimVal() + element->GetHeight().GetAnimVal());
			point = point.MatrixTransform(matrix);
			if (point.GetX() < 0 || point.GetY() < 0)
				break;
		}
		// get ref element
		wxString href = element->GetHref();
		if (href.length() == 0 || href.GetChar(0) != wxT('#'))
			break;
		href.Remove(0, 1);
		wxSVGElement* refElem = (wxSVGElement*) ownerSVGElement->GetElementById(href);
		if (!refElem)
			break;

		// create shadow tree
		wxSVGGElement* gElem = new wxSVGGElement();
		gElem->SetOwnerDocument(elem->GetOwnerDocument());
		gElem->SetOwnerSVGElement(ownerSVGElement);
		gElem->SetViewportElement(viewportElement);
		gElem->SetStyle(element->GetStyle());
		if (element->GetX().GetAnimVal().GetUnitType() != wxSVG_LENGTHTYPE_UNKNOWN)
			gElem->Translate(element->GetX().GetAnimVal(), element->GetY().GetAnimVal());
		if (refElem->GetDtd() == wxSVG_SYMBOL_ELEMENT || refElem->GetDtd() == wxSVG_SVG_ELEMENT) {
			wxSVGSVGElement* svgElem;
			if (refElem->GetDtd() == wxSVG_SVG_ELEMENT)
				svgElem = (wxSVGSVGElement*) refElem->CloneNode();
			else {
				svgElem = new wxSVGSVGElement();
				wxSvgXmlElement* child = refElem->GetChildren();
				while (child) {
					svgElem->AddChild(child->CloneNode());
					child = child->GetNext();
				}
				svgElem->SetViewBox(((wxSVGSymbolElement*) refElem)->GetViewBox());
				svgElem->SetPreserveAspectRatio(((wxSVGSymbolElement*) refElem)->GetPreserveAspectRatio());
			}
			if (element->GetWidth().GetAnimVal().GetUnitType() != wxSVG_LENGTHTYPE_UNKNOWN)
				svgElem->SetWidth(element->GetWidth().GetAnimVal());
			if (element->GetHeight().GetAnimVal().GetUnitType() != wxSVG_LENGTHTYPE_UNKNOWN)
				svgElem->SetHeight(element->GetHeight().GetAnimVal());
			gElem->AddChild(svgElem);
			LoadImages(refElem, svgElem, progressDlg);
		} else
			gElem->AddChild(refElem->CloneNode());
		// render
		RenderElement(gElem, rect, &matrix, &style, ownerSVGElement, viewportElement, progressDlg);
		// delete shadow tree
		delete gElem;
		break;
	}
	default:
	  break;
  }
}

void wxSVGCanvas::RenderChilds(wxSVGElement* parent, const wxSVGRect* rect, const wxSVGMatrix* parentMatrix,
		const wxCSSStyleDeclaration* parentStyle, wxSVGSVGElement* ownerSVGElement, wxSVGElement* viewportElement,
		wxProgressDialog* progressDlg) {
  if (parentStyle->GetDisplay() == wxCSS_VALUE_INLINE) {
		wxSVGElement* elem = (wxSVGElement*) parent->GetChildren();
		while (elem) {
			if (elem->GetType() == wxSVGXML_ELEMENT_NODE) {
				//if (!rect || ownerSVGElement->CheckIntersection(*elem, *rect))
				RenderElement(elem, rect, parentMatrix, parentStyle, ownerSVGElement, viewportElement, progressDlg);
			}
			elem = (wxSVGElement*) elem->GetNext();
		}
	}
}
