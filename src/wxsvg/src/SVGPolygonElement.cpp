//////////////////////////////////////////////////////////////////////////////
// Name:        SVGPolygonElement.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/05/10
// RCS-ID:      $Id: SVGPolygonElement.cpp,v 1.4 2006/01/08 12:44:30 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGPolygonElement.h"
#include "SVGCanvas.h"

wxSVGRect wxSVGPolygonElement::GetBBox(wxSVG_COORDINATES coordinates)
{
  const wxSVGPointList& points = GetPoints();
  if (points.Count() == 0)
	return wxSVGRect();
  
  wxSVGPoint p0 = points[0];
  wxSVGMatrix matrix;
  if (coordinates != wxSVG_COORDINATES_USER)
  {
    matrix = GetMatrix(coordinates);
    p0 = p0.MatrixTransform(matrix);
  }
  wxSVGRect bbox(p0.GetX(), p0.GetY(), 0, 0);
  
  wxSVGPoint pi = wxSVGPoint();
  for (int i = 1; i<(int)points.Count(); i++)
  {
  	pi = coordinates == wxSVG_COORDINATES_USER ?
      points[i] : points[i].MatrixTransform(matrix);
	if (bbox.GetX() > pi.GetX())
	{
	  bbox.SetWidth(bbox.GetWidth() + bbox.GetX() - pi.GetX());
	  bbox.SetX(pi.GetX());
	}
	if (bbox.GetY() > pi.GetY())
	{
	  bbox.SetHeight(bbox.GetHeight() + bbox.GetY() - pi.GetY());
	  bbox.SetY(pi.GetY());
	}
	
	if (bbox.GetX() + bbox.GetWidth() < pi.GetX())
	  bbox.SetWidth(pi.GetX() - bbox.GetX());
	if (bbox.GetY() + bbox.GetHeight() < pi.GetY())
	  bbox.SetHeight(pi.GetY() - bbox.GetY());
  }
  
  return bbox;
}

wxSVGRect wxSVGPolygonElement::GetResultBBox(wxSVG_COORDINATES coordinates)
{
  wxCSSStyleDeclaration style = GetResultStyle(*this);
  if (style.GetStroke().GetPaintType() == wxSVG_PAINTTYPE_NONE)
    return GetBBox(coordinates);
  WX_SVG_CREATE_M_CANVAS_ITEM
  wxSVGRect bbox = coordinates == wxSVG_COORDINATES_USER ?
    m_canvasItem->GetResultBBox(style) :
    m_canvasItem->GetResultBBox(style, GetMatrix(coordinates));
  WX_SVG_CLEAR_M_CANVAS_ITEM
  return bbox;
}

void wxSVGPolygonElement::SetCanvasItem(wxSVGCanvasItem* canvasItem)
{
  if (m_canvasItem)
    delete m_canvasItem;
  m_canvasItem = canvasItem;
}
