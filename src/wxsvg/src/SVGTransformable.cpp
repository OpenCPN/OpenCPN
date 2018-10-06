//////////////////////////////////////////////////////////////////////////////
// Name:        SVGTransformable.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/05/05
// RCS-ID:      $Id: SVGTransformable.cpp,v 1.8 2014/03/27 21:20:10 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGTransformable.h"
#include "svg.h"
#include <wx/log.h>

void wxSVGTransformable::Transform(const wxSVGMatrix& matrix) {
  wxSVGTransformList& transforms = m_transform.GetBaseVal();
  transforms.Add(new wxSVGTransform(matrix));
  m_transform.ResetAnimVal();
}

void wxSVGTransformable::Translate(double tx, double ty) {
  wxSVGTransformList& transforms = m_transform.GetBaseVal();
  wxSVGTransform* t = new wxSVGTransform; t->SetTranslate(tx,ty); transforms.Add(t);
  m_transform.ResetAnimVal();
}

void wxSVGTransformable::Scale(double s) {
  wxSVGTransformList& transforms = m_transform.GetBaseVal();
  wxSVGTransform* t = new wxSVGTransform; t->SetScale(s,s); transforms.Add(t);
  m_transform.ResetAnimVal();
}

void wxSVGTransformable::Scale(double sx, double sy) {
  wxSVGTransformList& transforms = m_transform.GetBaseVal();
  wxSVGTransform* t = new wxSVGTransform; t->SetScale(sx,sy); transforms.Add(t);
  m_transform.ResetAnimVal();
}

void wxSVGTransformable::Rotate(double angle, double cx, double cy) {
  wxSVGTransformList& transforms = m_transform.GetBaseVal();
  wxSVGTransform* t = new wxSVGTransform; t->SetRotate(angle, cx,cy); transforms.Add(t);
  m_transform.ResetAnimVal();
}

void wxSVGTransformable::SkewX(double angle) {
  wxSVGTransformList& transforms = m_transform.GetBaseVal();
  wxSVGTransform* t = new wxSVGTransform; t->SetSkewX(angle); transforms.Add(t);
  m_transform.ResetAnimVal();
}

void wxSVGTransformable::SkewY(double angle) {
  wxSVGTransformList& transforms = m_transform.GetBaseVal();
  wxSVGTransform* t = new wxSVGTransform; t->SetSkewY(angle); transforms.Add(t);
  m_transform.ResetAnimVal();
}

#define GET_TRANSFORMABLE(the_dtd, the_class)\
case the_dtd:\
  return &((the_class&)element);

wxSVGTransformable* wxSVGTransformable::GetSVGTransformable(wxSVGElement& element) {
  if (&element == NULL || element.GetType() != wxSVGXML_ELEMENT_NODE) {
      return NULL;
  }
  switch (element.GetDtd()) {
    GET_TRANSFORMABLE(wxSVG_LINE_ELEMENT, wxSVGLineElement)
    GET_TRANSFORMABLE(wxSVG_POLYLINE_ELEMENT, wxSVGPolylineElement)
    GET_TRANSFORMABLE(wxSVG_POLYGON_ELEMENT, wxSVGPolygonElement)
    GET_TRANSFORMABLE(wxSVG_RECT_ELEMENT, wxSVGRectElement)
    GET_TRANSFORMABLE(wxSVG_CIRCLE_ELEMENT, wxSVGCircleElement)
    GET_TRANSFORMABLE(wxSVG_ELLIPSE_ELEMENT, wxSVGEllipseElement)
    GET_TRANSFORMABLE(wxSVG_PATH_ELEMENT, wxSVGPathElement)
    GET_TRANSFORMABLE(wxSVG_TEXT_ELEMENT, wxSVGTextElement)
    GET_TRANSFORMABLE(wxSVG_G_ELEMENT, wxSVGGElement)
    GET_TRANSFORMABLE(wxSVG_USE_ELEMENT, wxSVGUseElement)
    default:
      break;
  }
  return NULL;
}

const wxSVGTransformable* wxSVGTransformable::GetSVGTransformable(const wxSVGElement& element) {
  return GetSVGTransformable((wxSVGElement&)element);
}

void wxSVGTransformable::UpdateMatrix(wxSVGMatrix& matrix) const {
  const wxSVGTransformList& transforms = GetTransform().GetAnimVal();
	for (unsigned int i = 0; i < transforms.Count(); i++)
		matrix = matrix.Multiply(transforms[i].GetMatrix());
}
