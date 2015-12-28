//////////////////////////////////////////////////////////////////////////////
// Name:        SVGLineElement.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/05/10
// RCS-ID:      $Id: SVGLineElement.cpp,v 1.4 2006/01/08 12:44:30 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGLineElement.h"
#include "SVGCanvas.h"

wxSVGRect wxSVGLineElement::GetBBox(wxSVG_COORDINATES coordinates)
{
  wxSVGPoint p1 = wxSVGPoint(GetX1().GetAnimVal(), GetY1().GetAnimVal());
  wxSVGPoint p2 = wxSVGPoint(GetX2().GetAnimVal(), GetY2().GetAnimVal());
  if (coordinates != wxSVG_COORDINATES_USER)
  {
    wxSVGMatrix matrix = GetMatrix(coordinates);
    p1 = p1.MatrixTransform(matrix);
    p2 = p2.MatrixTransform(matrix);
  }
  
  double x1 = p1.GetX();
  double y1 = p1.GetY();
  double x2 = p2.GetX();
  double y2 = p2.GetY();

  wxSVGRect bbox(x1, y1, x2 - x1, y2 - y1);
  
  if (x1 > x2)
  {
	bbox.SetX(x2);
	bbox.SetWidth(x1 - x2);
  }
  
  if (y1 > y2)
  {
	bbox.SetY(y2);
	bbox.SetHeight(y1 - y2);
  }
  
  return bbox;
}

wxSVGRect wxSVGLineElement::GetResultBBox(wxSVG_COORDINATES coordinates)
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

void wxSVGLineElement::SetCanvasItem(wxSVGCanvasItem* canvasItem)
{
  if (m_canvasItem)
    delete m_canvasItem;
  m_canvasItem = canvasItem;
}
