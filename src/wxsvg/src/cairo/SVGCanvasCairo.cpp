//////////////////////////////////////////////////////////////////////////////
// Name:        SVGCanvasCairo.cpp
// Purpose:     Cairo render
// Author:      Alex Thuering
// Created:     2005/05/12
// RCS-ID:      $Id: SVGCanvasCairo.cpp,v 1.33 2015/09/19 17:18:23 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGCanvasCairo.h"
#include "SVGCanvasPathCairo.h"
#include "SVGCanvasTextCairo.h"
#include "SVGCanvasImageCairo.h"
#include <wx/log.h>
#include <wx/file.h>

#ifdef __WXMSW__
 inline long lround(double d) {
    return (long)(d>0 ? d+0.5 : ceil(d-0.5));
    }
#endif

wxSVGCanvasCairo::~wxSVGCanvasCairo() {
	Destroy();
}

void wxSVGCanvasCairo::Destroy() {
	if (m_pattern) {
		cairo_pattern_destroy(m_pattern);
		m_pattern = NULL;
	}
	if (m_cr)
		cairo_destroy(m_cr);
	if (m_surface)
		cairo_surface_destroy(m_surface);
}

void wxSVGCanvasCairo::Init(int width, int height, bool alpha) {
	if (m_surface != NULL && m_alpha == alpha
			&& cairo_image_surface_get_width(m_surface) == width
			&& cairo_image_surface_get_height(m_surface) == height) {
		if (alpha) {
			cairo_set_source_rgba(m_cr, 0.0, 0.0, 0.0, 0.0);
			cairo_rectangle(m_cr, 0, 0, GetWidth(), GetHeight());
			cairo_fill(m_cr);
		}
		return;
	}
	Destroy();
	m_alpha = alpha;
	m_surface = cairo_image_surface_create(alpha ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24, width, height);
	m_cr = cairo_create(m_surface);
}

int wxSVGCanvasCairo::GetWidth() {
	return cairo_image_surface_get_width(m_surface);
}

int wxSVGCanvasCairo::GetHeight() {
	return cairo_image_surface_get_height(m_surface);
}

wxImage wxSVGCanvasCairo::GetImage() {
	wxImage img;
	int width = GetWidth();
	int height = GetHeight();
	img.Create(width, height);
	if (m_alpha)
		img.InitAlpha();
	unsigned char* src = cairo_image_surface_get_data(m_surface);
	unsigned char* dst = img.GetData();
	unsigned char* dstAlpha = img.GetAlpha();
	for (int i = 0; i < width * height; i++) {
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
		if (dstAlpha != NULL) {
			*dstAlpha = src[3];
			dstAlpha++;
		}
		src += 4;
		dst += 3;
	}
	return img;
}

void wxSVGCanvasCairo::Clear(wxRGBColor color) {
	if (!m_cr || !m_surface)
		return;
	cairo_set_source_rgb(m_cr, color.Red() / 255.0, color.Green() / 255.0, color.Blue() / 255.0);
	cairo_rectangle(m_cr, 0, 0, GetWidth(), GetHeight());
	cairo_fill(m_cr);
}

wxSVGCanvasPath* wxSVGCanvasCairo::CreateCanvasPath(wxSVGMatrix* matrix) {
	if (m_surface == NULL)
		m_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
	return new wxSVGCanvasPathCairo(m_surface, matrix);
}

wxSVGCanvasItem* wxSVGCanvasCairo::CreateItem(wxSVGTextElement* element, const wxCSSStyleDeclaration* style,
		wxSVGMatrix* matrix) {
	wxSVGCanvasTextCairo* canvasText = new wxSVGCanvasTextCairo(this);
	canvasText->Init(*element, style != NULL ? *style : (wxCSSStyleDeclaration&) element->GetStyle(), matrix);
	return canvasText;
}

wxSVGCanvasItem* wxSVGCanvasCairo::CreateItem(wxSVGImageElement* element, const wxCSSStyleDeclaration* style,
		wxProgressDialog* progressDlg) {
	wxSVGCanvasImageCairo* canvasImage = new wxSVGCanvasImageCairo();
	canvasImage->Init(*element, style != NULL ? *style : (wxCSSStyleDeclaration&) element->GetStyle(), progressDlg);
	return canvasImage;
}

wxSVGCanvasItem* wxSVGCanvasCairo::CreateItem(wxSVGVideoElement* element, const wxCSSStyleDeclaration* style,
		wxProgressDialog* progressDlg) {
	wxSVGCanvasVideoCairo* canvasVideo = new wxSVGCanvasVideoCairo();
	canvasVideo->Init(*element, style != NULL ? *style : (wxCSSStyleDeclaration&) element->GetStyle(), progressDlg);
	return canvasVideo;
}

void wxSVGCanvasCairo::DrawItem(wxSVGCanvasItem& item, wxSVGMatrix& matrix,
		const wxCSSStyleDeclaration& style, wxSVGSVGElement& svgElem) {
	switch (item.GetType()) {
	case wxSVG_CANVAS_ITEM_PATH:
		DrawCanvasPath((wxSVGCanvasPathCairo&) item, matrix, style, svgElem);
		break;
	case wxSVG_CANVAS_ITEM_TEXT:
		DrawCanvasText((wxSVGCanvasText&) item, matrix, style, svgElem);
		break;
	case wxSVG_CANVAS_ITEM_IMAGE:
		DrawCanvasImage((wxSVGCanvasImage&) item, ((wxSVGCanvasImageCairo&) item).GetCairoSurface(),
				matrix, style, svgElem);
		break;
	case wxSVG_CANVAS_ITEM_VIDEO:
		DrawCanvasImage((wxSVGCanvasImage&) item, ((wxSVGCanvasVideoCairo&) item).GetCairoSurface(),
				matrix, style, svgElem);
		break;
	}
}

void wxSVGCanvasCairo::SetPaint(cairo_t* cr, const wxSVGPaint& paint, float opacity, wxSVGCanvasPathCairo& canvasPath,
		wxSVGSVGElement& svgElem, const wxSVGMatrix& matrix) {
	if (paint.GetPaintType() >= wxSVG_PAINTTYPE_URI_NONE && paint.GetPaintType() <= wxSVG_PAINTTYPE_URI) {
		wxSVGGradientElement* gradElem = GetGradientElement(svgElem, paint.GetUri());
		if (gradElem != NULL) {
			if (m_pattern != NULL) {
				cairo_pattern_destroy(m_pattern);
				m_pattern = NULL;
			}
			switch (gradElem->GetDtd()) {
			case wxSVG_LINEARGRADIENT_ELEMENT: {
				wxSVGPoint p1, p2;
				GetLinearGradientVector(p1, p2, (wxSVGLinearGradientElement&) *gradElem, canvasPath);
				m_pattern = cairo_pattern_create_linear(p1.GetX(), p1.GetY(), p2.GetX(), p2.GetY());
				break;
			}
			case wxSVG_RADIALGRADIENT_ELEMENT: {
				wxSVGRadialGradientElement* radialGradElem = (wxSVGRadialGradientElement*) gradElem;
				double r = radialGradElem->GetQualifiedR();
				double cx = radialGradElem->GetQualifiedCx();
				double cy = radialGradElem->GetQualifiedCy();
				double fx = radialGradElem->GetQualifiedFx();
				double fy = radialGradElem->GetQualifiedFy();
				
				if (gradElem->GetGradientUnits().GetAnimVal() == wxSVG_UNIT_TYPE_UNKNOWN
						|| gradElem->GetGradientUnits().GetAnimVal() == wxSVG_UNIT_TYPE_OBJECTBOUNDINGBOX) {
					wxSVGRect bbox = canvasPath.GetBBox();
					r = r * sqrt(bbox.GetWidth() * bbox.GetWidth() + bbox.GetHeight() * bbox.GetHeight());
					cx = bbox.GetX() + cx * bbox.GetWidth();
					cy = bbox.GetY() + cy * bbox.GetHeight();
					fx = bbox.GetX() + fx * bbox.GetWidth();
					fy = bbox.GetY() + fy * bbox.GetHeight();
				}
				m_pattern = cairo_pattern_create_radial(fx, fy, 0.0, cx, cy, r);
				const wxSVGTransformList& transforms =  gradElem->GetGradientTransform().GetAnimVal();
				if (transforms.GetCount() > 0) {
					wxSVGMatrix patMatrix;
					for (unsigned int i = 0; i < transforms.GetCount(); i++)
						patMatrix = patMatrix.Multiply(transforms[i].GetMatrix());
					patMatrix = patMatrix.Inverse();
					cairo_matrix_t mat;
					cairo_matrix_init(&mat, patMatrix.GetA(), patMatrix.GetB(), patMatrix.GetC(), patMatrix.GetD(), patMatrix.GetE(), patMatrix.GetF());
					cairo_pattern_set_matrix(m_pattern, &mat);
				}
				break;
			}
			default:
				break;
			}
			if (m_pattern != NULL) {
				int nstops = GetGradientStops(svgElem, gradElem, opacity);
				if (nstops) {
					cairo_set_source(cr, m_pattern);
				} else {
					cairo_pattern_destroy(m_pattern);
					m_pattern = NULL;
				}
			}
		}
		wxSVGPatternElement* patternElem = GetPatternElement(svgElem, paint.GetUri());
		if (patternElem != NULL && patternElem->GetWidth().GetAnimVal() > 0
				&& patternElem->GetHeight().GetAnimVal() > 0) {
			if (m_pattern != NULL) {
				cairo_pattern_destroy(m_pattern);
				m_pattern = NULL;
			}
			double scaleX = matrix.GetA();
			scaleX = lround(patternElem->GetWidth().GetAnimVal()*scaleX) / patternElem->GetWidth().GetAnimVal(); 
			double scaleY = matrix.GetD();
			scaleY = lround(patternElem->GetHeight().GetAnimVal()*scaleY) / patternElem->GetHeight().GetAnimVal(); 
			patternElem->SetOwnerSVGElement(&svgElem);
			patternElem->SetViewportElement(&svgElem);
			cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
					lround(patternElem->GetWidth().GetAnimVal()*scaleX),
					lround(patternElem->GetHeight().GetAnimVal()*scaleY));
			cairo_t* cr = cairo_create(surface);
			wxSVGMatrix patMatrix;
			patMatrix = patMatrix.ScaleNonUniform(scaleX, scaleY);
			wxCSSStyleDeclaration style;
			DrawMask(cr, patternElem, patMatrix, style, svgElem);
			m_pattern = cairo_pattern_create_for_surface(surface);
			
			if (patternElem->GetX().GetAnimVal() > 0 || patternElem->GetY().GetAnimVal() > 0) {
				patMatrix = patMatrix.Translate(patternElem->GetX().GetAnimVal(), patternElem->GetY().GetAnimVal());
			}
			if (patternElem->GetPatternTransform().GetAnimVal().size()) {
				const wxSVGTransformList& transforms = patternElem->GetPatternTransform().GetAnimVal();
				for (unsigned int i = 0; i < transforms.Count(); i++) {
					patMatrix = patMatrix.Multiply(transforms[i].GetMatrix().Inverse());
				}
			}
			cairo_matrix_t mat;
			cairo_matrix_init(&mat, patMatrix.GetA(), patMatrix.GetB(), patMatrix.GetC(), patMatrix.GetD(), patMatrix.GetE(), patMatrix.GetF());
			cairo_pattern_set_matrix(m_pattern, &mat);
			
			cairo_set_source(m_cr, m_pattern);
			cairo_pattern_set_extend(m_pattern, CAIRO_EXTEND_REPEAT);
			
			cairo_destroy(cr);
			cairo_surface_destroy(surface);
		}
	} else {
		wxRGBColor color = paint.GetRGBColor();
		cairo_set_source_rgba(cr, color.Red() / 255.0, color.Green() / 255.0, color.Blue() / 255.0, opacity);
	}
}


void wxSVGCanvasCairo::SetStopValue(unsigned int index, float offset,
		float opacity, const wxRGBColor& rgbColor) {
	cairo_pattern_add_color_stop_rgba(m_pattern, offset, rgbColor.Red() / 255.0, rgbColor.Green() / 255.0,
			rgbColor.Blue() / 255.0, opacity);
}

void wxSVGCanvasCairo::AllocateGradientStops(unsigned int stop_count) {
	// nothing to do
}

void boxBlurH(unsigned char *aInput, unsigned char *aOutput, int aStride, const wxRect &aRegion, unsigned int leftLobe,
		unsigned int rightLobe, const unsigned char *prediv) {
	int boxSize = leftLobe + rightLobe + 1;
	int posStart = aRegion.x - leftLobe;
	
	for (int y = aRegion.y; y < aRegion.height; y++) {
		unsigned int sums[4] = { 0, 0, 0, 0 };
		int lineIndex = aStride * y;
		for (int i = 0; i < boxSize; i++) {
			int pos = posStart + i;
			pos = wxMax(pos, aRegion.x);
			pos = wxMin(pos, aRegion.width - 1);
			int index = lineIndex + (pos << 2);
			sums[0] += aInput[index];
			sums[1] += aInput[index + 1];
			sums[2] += aInput[index + 2];
			sums[3] += aInput[index + 3];
		}
		for (int x = aRegion.x; x < aRegion.width; x++) {
			int index = lineIndex + (x << 2);
			aOutput[index] = prediv[sums[0]];
			aOutput[index + 1] = prediv[sums[1]];
			aOutput[index + 2] = prediv[sums[2]];
			aOutput[index + 3] = prediv[sums[3]];
			
			int tmp = x - leftLobe;
			int last = wxMax(tmp, aRegion.x);
			int next = wxMin(tmp + boxSize, aRegion.width - 1);
			int index2 = lineIndex + (next << 2);
			int index3 = lineIndex + (last << 2);
			sums[0] += aInput[index2] - aInput[index3];
			sums[1] += aInput[index2 + 1] - aInput[index3 + 1];
			sums[2] += aInput[index2 + 2] - aInput[index3 + 2];
			sums[3] += aInput[index2 + 3] - aInput[index3 + 3];
		}
	}
}

void boxBlurV(unsigned char *aInput, unsigned char *aOutput, int aStride, const wxRect &aRegion, unsigned int topLobe,
		unsigned int bottomLobe, const unsigned char *prediv) {
	int boxSize = topLobe + bottomLobe + 1;
	int posStart = aRegion.y - topLobe;
	
	for (int x = aRegion.x; x < aRegion.width; x++) {
		unsigned int sums[4] = { 0, 0, 0, 0 };
		int fourX = x << 2;
		for (int i = 0; i < boxSize; i++) {
			int pos = posStart + i;
			pos = wxMax(pos, aRegion.y);
			pos = wxMin(pos, aRegion.height - 1);
			int index = aStride * pos + fourX;
			sums[0] += aInput[index];
			sums[1] += aInput[index + 1];
			sums[2] += aInput[index + 2];
			sums[3] += aInput[index + 3];
		}
		for (int y = aRegion.y; y < aRegion.height; y++) {
			int index = aStride * y + fourX;
			aOutput[index] = prediv[sums[0]];
			aOutput[index + 1] = prediv[sums[1]];
			aOutput[index + 2] = prediv[sums[2]];
			aOutput[index + 3] = prediv[sums[3]];
			
			int tmp = y - topLobe;
			int last = wxMax(tmp, aRegion.y);
			int next = wxMin(tmp + boxSize, aRegion.height - 1);
			int index2 = aStride * next + fourX;
			int index3 = aStride * last + fourX;
			sums[0] += aInput[index2] - aInput[index3];
			sums[1] += aInput[index2 + 1] - aInput[index3 + 1];
			sums[2] += aInput[index2 + 2] - aInput[index3 + 2];
			sums[3] += aInput[index2 + 3] - aInput[index3 + 3];
		}
	}
}

unsigned char* setupPredivide(int size) {
	unsigned char *result = new unsigned char[size * 256];
    for (int i = 0; i < 256; i++)
      memset(result + i * size, i, size);
    return result;
}

/**
 * Draws gaussian blur filter.
 * Uses code from Mozilla (nsSVGFilters.cpp)
 */
void gaussianBlur(cairo_surface_t* surface, int dx, int dy) {
	unsigned char* buffer = cairo_image_surface_get_data(surface);
	
	int stride = cairo_image_surface_get_stride(surface);
	int width = cairo_image_surface_get_width(surface);
	int height = cairo_image_surface_get_height(surface);
	wxRect rect(0, 0, width, height);
	
	// Create temporary buffer
	unsigned char* tempBuffer = (unsigned char*) calloc((size_t)(stride * height), 1);
	if (tempBuffer == NULL)
		return;
	
	if (dx & 1) {
		// odd
		unsigned char* prediv = setupPredivide(2 * (dx / 2) + 1);
		boxBlurH(buffer, tempBuffer, stride, rect, dx / 2, dx / 2, prediv);
		boxBlurH(tempBuffer, buffer, stride, rect, dx / 2, dx / 2, prediv);
		boxBlurH(buffer, tempBuffer, stride, rect, dx / 2, dx / 2, prediv);
		delete[] prediv;
	} else {
		// even
		if (dx == 0) {
			memcpy(tempBuffer, buffer, (size_t)(stride * height));
		} else {
			unsigned char* prediv = setupPredivide(2 * (dx / 2) + 1);
			unsigned char* prediv2 = setupPredivide(2 * (dx / 2));
			boxBlurH(buffer, tempBuffer, stride, rect, dx / 2, dx / 2 - 1, prediv2);
			boxBlurH(tempBuffer, buffer, stride, rect, dx / 2 - 1, dx / 2, prediv2);
			boxBlurH(buffer, tempBuffer, stride, rect, dx / 2, dx / 2, prediv);
			delete[] prediv;
			delete[] prediv2;
		}
	}

	if (dy & 1) {
		// odd
		unsigned char* prediv = setupPredivide(2 * (dy / 2) + 1);
		boxBlurV(tempBuffer, buffer, stride, rect, dy / 2, dy / 2, prediv);
		boxBlurV(buffer, tempBuffer, stride, rect, dy / 2, dy / 2, prediv);
		boxBlurV(tempBuffer, buffer, stride, rect, dy / 2, dy / 2, prediv);
		delete[] prediv;
	} else {
		// even
		if (dy == 0) {
			memcpy(buffer, tempBuffer, (size_t)(stride * height));
		} else {
			unsigned char* prediv = setupPredivide(2 * (dy / 2) + 1);
			unsigned char* prediv2 = setupPredivide(2 * (dy / 2));
			boxBlurV(tempBuffer, buffer, stride, rect, dy / 2, dy / 2 - 1, prediv2);
			boxBlurV(buffer, tempBuffer, stride, rect, dy / 2 - 1, dy / 2, prediv2);
			boxBlurV(tempBuffer, buffer, stride, rect, dy / 2, dy / 2, prediv);
			delete[] prediv;
			delete[] prediv2;
		}
	}
	
	free(tempBuffer);
}

void wxSVGCanvasCairo::DrawPath(cairo_t* cr, wxSVGCanvasPathCairo& canvasPath, const wxSVGMatrix& matrix,
		const wxCSSStyleDeclaration& style, wxSVGSVGElement& svgElem) {
	SetMatrix(cr, matrix);
	
	// Filling
	if (canvasPath.GetFill() && style.GetFill().Ok() && style.GetFill().GetPaintType() != wxSVG_PAINTTYPE_NONE) {
		cairo_path_t* path = canvasPath.GetPath();
		cairo_append_path(cr, path);
		SetPaint(cr, style.GetFill(), style.GetOpacity()*style.GetFillOpacity(), canvasPath, svgElem, matrix);
		cairo_fill(cr);
		cairo_path_destroy(path);
	}
	
	// Stroking
	if (style.GetStroke().Ok() && style.GetStrokeWidth() > 0
			&& style.GetStroke().GetPaintType() != wxSVG_PAINTTYPE_NONE) {
		cairo_path_t* path = canvasPath.GetPath();
		cairo_append_path(cr, path);
		SetPaint(cr, style.GetStroke(), style.GetOpacity()*style.GetStrokeOpacity(), canvasPath, svgElem, matrix);
		wxSVGCanvasPathCairo::ApplyStrokeStyle(cr, style);
		cairo_stroke(cr);
		cairo_path_destroy(path);
	}
	
	// marker
	if (style.HasMarkerStart()) {
		DrawMarker(style.GetMarkerStart().GetStringValue(), wxSVGMark::START, canvasPath, matrix, style, svgElem);
	}
	if (style.HasMarkerMid()) {
		DrawMarker(style.GetMarkerMid().GetStringValue(), wxSVGMark::MID, canvasPath, matrix, style, svgElem);
	}
	if (style.HasMarkerEnd()) {
		DrawMarker(style.GetMarkerEnd().GetStringValue(), wxSVGMark::END, canvasPath, matrix, style, svgElem);
	}
}

void wxSVGCanvasCairo::DrawCanvasPath(wxSVGCanvasPathCairo& canvasPath, wxSVGMatrix& matrix,
		const wxCSSStyleDeclaration& style, wxSVGSVGElement& svgElem) {
	// check Filter
	if (style.GetFilter().GetCSSPrimitiveType() == wxCSS_URI && style.GetFilter().GetStringValue().length() > 1) {
		wxString filterId = style.GetFilter().GetStringValue().substr(1);
		wxSVGElement* filterElem = (wxSVGElement*) svgElem.GetElementById(filterId);
		// feGaussianBlur
		if (filterElem && filterElem->GetDtd() == wxSVG_FILTER_ELEMENT && filterElem->GetFirstChild() != NULL
				&& ((wxSVGSVGElement*) filterElem->GetFirstChild())->GetDtd() == wxSVG_FEGAUSSIANBLUR_ELEMENT) {
			float stdX = ((wxSVGFEGaussianBlurElement*) filterElem->GetFirstChild())->GetStdDeviationX().GetAnimVal();
			float stdY = ((wxSVGFEGaussianBlurElement*) filterElem->GetFirstChild())->GetStdDeviationY().GetAnimVal();
			if (stdX <= 0 || stdY <= 0)
				return;
			int dx = int(floor(stdX * 3 * sqrt(2 * M_PI) / 4 + 0.5));
			int dy = int(floor(stdY * 3 * sqrt(2 * M_PI) / 4 + 0.5));
			
			wxSVGRect rect = canvasPath.GetResultBBox(style, matrix.Inverse());
			rect.SetX(rect.GetX() - 2*dx);
			rect.SetY(rect.GetY() - 2*dy);
			rect.SetWidth(rect.GetWidth() + 4*dx);
			rect.SetHeight(rect.GetHeight() + 4*dy);
			
			int width = (int) rect.GetWidth();
			int height = (int) rect.GetHeight();
			cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
			cairo_t* cr = cairo_create(surface);
			wxSVGMatrix matrix2 = wxSVGMatrix(1, 0, 0, 1, - rect.GetX(), - rect.GetY()).Multiply(matrix);
			DrawPath(cr, canvasPath, matrix2, style, svgElem);
			gaussianBlur(surface, dx, dy);
			
			// draw surface
			cairo_save(m_cr);
			SetMatrix(m_cr, wxSVGMatrix(1, 0, 0, 1, rect.GetX(), rect.GetY()));
			cairo_set_source_surface(m_cr, surface, 0, 0);
			cairo_rectangle(m_cr, 0, 0, width, height);
			cairo_paint(m_cr); // fill the rectangle using the pattern
			cairo_new_path(m_cr);
			cairo_restore(m_cr);
			
			cairo_destroy(cr);
			cairo_surface_destroy(surface);
			return;
		}
	}
	DrawPath(m_cr, canvasPath, matrix, style, svgElem);
}

void wxSVGCanvasCairo::SetMatrix(cairo_t* cr, const wxSVGMatrix& matrix) {
	cairo_matrix_t mat;
	cairo_matrix_init(&mat, matrix.GetA(), matrix.GetB(), matrix.GetC(),
			matrix.GetD(), matrix.GetE(), matrix.GetF());
	cairo_set_matrix(cr, &mat);
}

void wxSVGCanvasCairo::SetClipPath(wxSVGElement* clipPathElem, wxSVGMatrix matrix) {
	SetMatrix(m_cr, matrix);
	wxSVGElement* elem = (wxSVGElement*) (clipPathElem->GetFirstChild());
	while (elem != NULL) {
		elem->SetOwnerSVGElement(clipPathElem->GetOwnerSVGElement());
		elem->SetViewportElement(clipPathElem->GetViewportElement());
		wxSVGDocument* doc = (wxSVGDocument*) elem->GetOwnerDocument();
		wxSVGCanvasItem* canvasItem = NULL;
		switch (elem->GetDtd()) {
		case wxSVG_G_ELEMENT: {
			wxSVGGElement* gElement = (wxSVGGElement*) elem;
			if (gElement->GetVisibility() == wxCSS_VALUE_HIDDEN)
				break;
			wxSVGMatrix gMatrix(matrix);
			gElement->UpdateMatrix(gMatrix);
			SetClipPath(elem, gMatrix);
			SetMatrix(m_cr, matrix);
			break;
		}
		case wxSVG_LINE_ELEMENT:
			canvasItem = doc->GetCanvas()->CreateItem((wxSVGLineElement*) elem);
			break;
		case wxSVG_POLYLINE_ELEMENT:
			canvasItem = doc->GetCanvas()->CreateItem((wxSVGPolylineElement*) elem);
			break;
		case wxSVG_POLYGON_ELEMENT:
			canvasItem = doc->GetCanvas()->CreateItem((wxSVGPolygonElement*) elem);
			break;
		case wxSVG_RECT_ELEMENT:
			canvasItem = doc->GetCanvas()->CreateItem((wxSVGRectElement*) elem);
			break;
		case wxSVG_CIRCLE_ELEMENT:
			canvasItem = doc->GetCanvas()->CreateItem((wxSVGCircleElement*) elem);
			break;
		case wxSVG_ELLIPSE_ELEMENT: {
			canvasItem = doc->GetCanvas()->CreateItem((wxSVGEllipseElement*) elem);
			break;
		}
		case wxSVG_PATH_ELEMENT:
			canvasItem = doc->GetCanvas()->CreateItem((wxSVGPathElement*) elem);
			break;
		default:
			break;
		}
		if (canvasItem != NULL) {
			cairo_path_t* path =
					((wxSVGCanvasPathCairo*) canvasItem)->GetPath();
			cairo_append_path(m_cr, path);
			cairo_clip(m_cr);
			//cairo_path_destroy(path);
			delete canvasItem;
		}
		elem = (wxSVGElement*) elem->GetNextSibling();
	}
}

void wxSVGCanvasCairo::DrawMask(cairo_t* cr, wxSVGElement* maskElem, const wxSVGMatrix& matrix,
		const wxCSSStyleDeclaration& style, wxSVGSVGElement& svgElem) {
	SetMatrix(cr, matrix);
	wxSVGElement* elem = (wxSVGElement*) (maskElem->GetFirstChild());
	while (elem != NULL) {
		elem->SetOwnerSVGElement(maskElem->GetOwnerSVGElement());
		elem->SetViewportElement(maskElem->GetViewportElement());
		wxSVGDocument* doc = (wxSVGDocument*) elem->GetOwnerDocument();
		wxSVGCanvasItem* canvasItem = NULL;
		wxCSSStyleDeclaration resStyle = style;
		switch (elem->GetDtd()) {
		case wxSVG_G_ELEMENT: {
			wxSVGGElement* gElement = (wxSVGGElement*) elem;
			if (gElement->GetVisibility() == wxCSS_VALUE_HIDDEN)
				break;
			resStyle.Add(((wxSVGGElement*) elem)->GetStyle());
			DrawMask(cr, elem, matrix, resStyle, svgElem);
			break;
		}
		case wxSVG_LINE_ELEMENT: {
			wxSVGLineElement* element = (wxSVGLineElement*) elem;
			if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
				break;
			canvasItem = doc->GetCanvas()->CreateItem(element);
			resStyle.Add(element->GetStyle());
			resStyle.Add(element->GetAnimStyle());
			break;
		}
		case wxSVG_POLYLINE_ELEMENT: {
			wxSVGPolylineElement* element = (wxSVGPolylineElement*) elem;
			if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
				break;
			canvasItem = doc->GetCanvas()->CreateItem(element);
			resStyle.Add(element->GetStyle());
			resStyle.Add(element->GetAnimStyle());
			break;
		}
		case wxSVG_POLYGON_ELEMENT: {
			wxSVGPolygonElement* element = (wxSVGPolygonElement*) elem;
			if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
				break;
			canvasItem = doc->GetCanvas()->CreateItem(element);
			resStyle.Add(element->GetStyle());
			resStyle.Add(element->GetAnimStyle());
			break;
		}
		case wxSVG_RECT_ELEMENT: {
			wxSVGRectElement* element = (wxSVGRectElement*) elem;
			if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
				break;
			canvasItem = doc->GetCanvas()->CreateItem(element);
			resStyle.Add(element->GetStyle());
			resStyle.Add(element->GetAnimStyle());
			break;
		}
		case wxSVG_CIRCLE_ELEMENT: {
			wxSVGCircleElement* element = (wxSVGCircleElement*) elem;
			if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
				break;
			canvasItem = doc->GetCanvas()->CreateItem(element);
			resStyle.Add(element->GetStyle());
			resStyle.Add(element->GetAnimStyle());
			break;
		}
		case wxSVG_ELLIPSE_ELEMENT: {
			wxSVGEllipseElement* element = (wxSVGEllipseElement*) elem;
			if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
				break;
			canvasItem = doc->GetCanvas()->CreateItem(element);
			resStyle.Add(element->GetStyle());
			resStyle.Add(element->GetAnimStyle());
			break;
		}
		case wxSVG_PATH_ELEMENT: {
			wxSVGPathElement* element = (wxSVGPathElement*) elem;
			if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
				break;
			canvasItem = doc->GetCanvas()->CreateItem(element);
			resStyle.Add(element->GetStyle());
			resStyle.Add(element->GetAnimStyle());
			break;
		}
		case wxSVG_USE_ELEMENT: {
			wxSVGUseElement* element = (wxSVGUseElement*) elem;
			if (element->GetVisibility() == wxCSS_VALUE_HIDDEN)
				break;
			resStyle.Add(element->GetStyle());
			resStyle.Add(element->GetAnimStyle());
			// get ref element
			wxString href = element->GetHref();
			if (href.length() == 0 || href.GetChar(0) != wxT('#'))
				break;
			href.Remove(0, 1);
			wxSVGElement* refElem = (wxSVGElement*) maskElem->GetOwnerSVGElement()->GetElementById(href);
			if (!refElem)
				break;

			// create shadow tree
			wxSVGGElement* gElem = new wxSVGGElement();
			gElem->SetOwnerDocument(elem->GetOwnerDocument());
			gElem->SetOwnerSVGElement(maskElem->GetOwnerSVGElement());
			gElem->SetViewportElement(maskElem->GetViewportElement());
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
			} else
				gElem->AddChild(refElem->CloneNode());
			// render
			DrawMask(cr, gElem, matrix, resStyle, svgElem);
			// delete shadow tree
			delete gElem;
			break;
		}
		default:
			break;
		}
		if (canvasItem != NULL) {
			DrawPath(cr, ((wxSVGCanvasPathCairo&) *canvasItem), matrix, resStyle, svgElem);
			delete canvasItem;
		}
		elem = (wxSVGElement*) elem->GetNextSibling();
	}
}

void wxSVGCanvasCairo::DrawCanvasImage(wxSVGCanvasImage& canvasImage, cairo_surface_t* cairoSurface,
		wxSVGMatrix& matrix, const wxCSSStyleDeclaration& style, wxSVGSVGElement& svgElem) {
	if (cairoSurface == NULL)
		return;
	
	cairo_save(m_cr);
	
	// ClipPath
	if (style.GetClipPath().GetCSSPrimitiveType() == wxCSS_URI && style.GetClipPath().GetStringValue().length() > 1) {
		wxString clipPathId = style.GetClipPath().GetStringValue().substr(1);
		wxSVGClipPathElement* clipPathElem = (wxSVGClipPathElement*) svgElem.GetElementById(clipPathId);
		if (clipPathElem && clipPathElem->GetDtd() == wxSVG_CLIPPATH_ELEMENT) {
			clipPathElem->SetOwnerSVGElement(&svgElem);
			clipPathElem->SetViewportElement(&svgElem);
			wxSVGMatrix clipMatrix(matrix);
			clipPathElem->UpdateMatrix(clipMatrix);
			SetClipPath(clipPathElem, clipMatrix);
		}
	}
	
	SetMatrix(m_cr, matrix);
	
	// scale context
	double x = canvasImage.m_x;
	double y = canvasImage.m_y;
	double scaleX = canvasImage.m_width / canvasImage.m_image.GetWidth();
	double scaleY = canvasImage.m_height / canvasImage.m_image.GetHeight();
	wxSVG_PRESERVEASPECTRATIO align = canvasImage.GetPreserveAspectRatio().GetAlign();
	bool alignX = false;
	if (align > wxSVG_PRESERVEASPECTRATIO_NONE) {
		scaleY = canvasImage.m_height / canvasImage.GetDefaultHeight();
		if (canvasImage.GetPreserveAspectRatio().GetMeetOrSlice() != wxSVG_MEETORSLICE_SLICE) {
			alignX = scaleX > scaleY;
		} else {
			cairo_rectangle(m_cr, x, y, canvasImage.m_width, canvasImage.m_height);
			cairo_clip(m_cr);
			alignX = scaleX < scaleY;
		}
		if (alignX) {
			scaleX = scaleY;
			if (align == wxSVG_PRESERVEASPECTRATIO_XMIDYMIN
					|| align == wxSVG_PRESERVEASPECTRATIO_XMIDYMID
					|| align == wxSVG_PRESERVEASPECTRATIO_XMIDYMAX)
				x += (canvasImage.m_width - canvasImage.GetDefaultWidth() * scaleX) / 2;
			else if (align == wxSVG_PRESERVEASPECTRATIO_XMAXYMIN
					|| align == wxSVG_PRESERVEASPECTRATIO_XMAXYMID
					|| align == wxSVG_PRESERVEASPECTRATIO_XMAXYMAX)
				x += canvasImage.m_width - canvasImage.GetDefaultWidth() * scaleX;
		} else {
			scaleY = scaleX;
			if (align == wxSVG_PRESERVEASPECTRATIO_XMINYMID
					|| align == wxSVG_PRESERVEASPECTRATIO_XMIDYMID
					|| align == wxSVG_PRESERVEASPECTRATIO_XMAXYMID)
				y += (canvasImage.m_height - canvasImage.GetDefaultHeight() * scaleY) / 2;
			else if (align == wxSVG_PRESERVEASPECTRATIO_XMINYMAX
					|| align == wxSVG_PRESERVEASPECTRATIO_XMIDYMAX
					|| align == wxSVG_PRESERVEASPECTRATIO_XMAXYMAX)
				y += canvasImage.m_height - canvasImage.GetDefaultHeight() * scaleY;
		}
		scaleY = scaleY * canvasImage.GetDefaultHeight() / canvasImage.m_image.GetHeight();
	}
	cairo_translate(m_cr, x, y);
	cairo_scale(m_cr, scaleX, scaleY);
	
	// prepare to draw the image
	cairo_set_source_surface(m_cr, cairoSurface, 0, 0);
	
	// use the original size here since the context is scaled already...
	cairo_rectangle(m_cr, 0, 0, canvasImage.m_image.GetWidth(), canvasImage.m_image.GetHeight());
	// paint
	if (style.GetMask().GetCSSPrimitiveType() == wxCSS_URI && style.GetMask().GetStringValue().length() > 1) {
		wxString maskId = style.GetMask().GetStringValue().substr(1);
		wxSVGMaskElement* maskElem = (wxSVGMaskElement*) svgElem.GetElementById(maskId);
		if (maskElem && maskElem->GetDtd() == wxSVG_MASK_ELEMENT) {
			maskElem->SetOwnerSVGElement(&svgElem);
			maskElem->SetViewportElement(&svgElem);
			cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
					svgElem.GetWidth().GetAnimVal()/scaleX, svgElem.GetHeight().GetAnimVal()/scaleY);
			cairo_t* cr = cairo_create(surface);
			wxSVGMatrix maskMatrix;
			maskMatrix = maskMatrix.Translate(x, y).ScaleNonUniform(scaleX, scaleY).Inverse();
			DrawMask(cr, maskElem, maskMatrix, style, svgElem);
			cairo_mask_surface(m_cr, surface, 0, 0);
			cairo_destroy(cr);
			cairo_surface_destroy(surface);
		}
	} else {
		cairo_paint_with_alpha(m_cr, style.GetOpacity());
	}
	cairo_new_path(m_cr);
	
	// clean up
	cairo_restore(m_cr);
}

void wxSVGCanvasCairo::DrawMarker(const wxString& uri, wxSVGMark::Type type, wxSVGCanvasPathCairo& canvasPath,
		const wxSVGMatrix& matrix, const wxCSSStyleDeclaration& style, wxSVGSVGElement& svgElem) {
	wxSVGMarkerElement* markerElem = GetMarkerElement(svgElem, uri);
	if (markerElem == NULL || markerElem->GetMarkerWidth().GetAnimVal() <= 0
				|| markerElem->GetMarkerHeight().GetAnimVal() <= 0)
		return;
	vector<wxSVGMark> markPoints = canvasPath.GetMarkPoints();
	for (vector<wxSVGMark>::iterator it = markPoints.begin(); it != markPoints.end(); it++) {
		if (it->type != type)
			continue;
		wxSVGMark& markPoint = *it;
		double scaleX = matrix.GetA() * style.GetStrokeWidth();
		double scaleY = matrix.GetD() * style.GetStrokeWidth();
		markerElem->SetOwnerSVGElement(&svgElem);
		markerElem->SetViewportElement(&svgElem);
		cairo_surface_t* surface = cairo_image_surface_create(
				CAIRO_FORMAT_ARGB32,
				lround(markerElem->GetMarkerWidth().GetAnimVal() * scaleX),
				lround(markerElem->GetMarkerHeight().GetAnimVal() * scaleY));
		cairo_t* cr = cairo_create(surface);
		wxSVGMatrix markerMatrix;
		markerMatrix = markerMatrix.ScaleNonUniform(scaleX, scaleY);
		wxCSSStyleDeclaration style;
		DrawMask(cr, markerElem, markerMatrix, style, svgElem);
		// draw surface
		cairo_save(m_cr);
		double refX = markerElem->GetRefX().GetAnimVal() * style.GetStrokeWidth();
		double refY = markerElem->GetRefY().GetAnimVal() * style.GetStrokeWidth();
		wxSVGPoint point(markPoint.x - refX, markPoint.y - refY);
		point = point.MatrixTransform(matrix);
		wxSVGMatrix m;
		m = m.Translate(point.GetX(), point.GetY());
		if (markPoint.angle != 0) {
			refX = markerElem->GetRefX().GetAnimVal() * scaleX;
			refY = markerElem->GetRefY().GetAnimVal() * scaleY;
			m = m.Translate(refX, refY).Rotate(markPoint.angle / M_PI * 180).Translate(-refX, -refY);
		}
		SetMatrix(m_cr, m);
		cairo_set_source_surface(m_cr, surface, 0, 0);
		cairo_paint(m_cr);
		cairo_restore(m_cr);
		cairo_destroy(cr);
		cairo_surface_destroy(surface);
	}
}
