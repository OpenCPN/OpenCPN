//////////////////////////////////////////////////////////////////////////////
// Name:        SVGTextElement.cpp
// Purpose:     svg text element
// Author:      Alex Thuering
// Created:     2005/05/10
// RCS-ID:      $Id: SVGTextElement.cpp,v 1.5 2008/04/14 15:44:55 etisserant Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGTextElement.h"
#include "SVGCanvas.h"
#include <math.h>

wxSVGRect wxSVGTextElement::GetBBox(wxSVG_COORDINATES coordinates)
{
  wxSVGMatrix matrix = wxSVGLocatable::GetCTM(this);
  WX_SVG_CREATE_M_CANVAS_ITEM
  wxSVGRect bbox = coordinates == wxSVG_COORDINATES_USER ? 
    m_canvasItem->GetBBox() : m_canvasItem->GetBBox(GetMatrix(coordinates));
  WX_SVG_CLEAR_M_CANVAS_ITEM
  return bbox.MatrixTransform(matrix);
  //return bbox;
}

wxSVGRect wxSVGTextElement::GetResultBBox(wxSVG_COORDINATES coordinates)
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

void wxSVGTextElement::SetCanvasItem(wxSVGCanvasItem* canvasItem)
{
  if (m_canvasItem)
    delete m_canvasItem;
  m_canvasItem = canvasItem;
}

long wxSVGTextElement::GetNumberOfChars()
{
  WX_SVG_CREATE_M_CANVAS_ITEM
  long number = ((wxSVGCanvasText*)m_canvasItem)->GetNumberOfChars();
  WX_SVG_CLEAR_M_CANVAS_ITEM
  return number;
}

double wxSVGTextElement::GetComputedTextLength()
{
  wxSVGMatrix matrix = wxSVGLocatable::GetCTM(this);
  WX_SVG_CREATE_M_CANVAS_ITEM
  double length = ((wxSVGCanvasText*)m_canvasItem)->GetComputedTextLength();
  WX_SVG_CLEAR_M_CANVAS_ITEM
  return length * sqrt(matrix.GetA() * matrix.GetA() + matrix.GetB() * matrix.GetB());
}

double wxSVGTextElement::GetSubStringLength(unsigned long charnum, unsigned long nchars)
{
  wxSVGMatrix matrix = wxSVGLocatable::GetCTM(this);
  WX_SVG_CREATE_M_CANVAS_ITEM
  double length = ((wxSVGCanvasText*)m_canvasItem)->GetSubStringLength(charnum, nchars);
  WX_SVG_CLEAR_M_CANVAS_ITEM
  return length * sqrt(matrix.GetA() * matrix.GetA() + matrix.GetB() * matrix.GetB());
}

wxSVGPoint wxSVGTextElement::GetStartPositionOfChar(unsigned long charnum)
{
  wxSVGPoint real_position;  
  wxSVGMatrix matrix = wxSVGLocatable::GetCTM(this);
  WX_SVG_CREATE_M_CANVAS_ITEM
  wxSVGPoint position = ((wxSVGCanvasText*)m_canvasItem)->GetStartPositionOfChar(charnum);
  WX_SVG_CLEAR_M_CANVAS_ITEM
  real_position.SetX(matrix.GetA() * position.GetX() + matrix.GetB() * position.GetY() + matrix.GetE());
  real_position.SetY(matrix.GetB() * position.GetX() + matrix.GetD() * position.GetY() + matrix.GetF());
  return real_position;
}

wxSVGPoint wxSVGTextElement::GetEndPositionOfChar(unsigned long charnum)
{
  wxSVGPoint real_position;  
  wxSVGMatrix matrix = wxSVGLocatable::GetCTM(this);
  WX_SVG_CREATE_M_CANVAS_ITEM
  wxSVGPoint position = ((wxSVGCanvasText*)m_canvasItem)->GetEndPositionOfChar(charnum);
  WX_SVG_CLEAR_M_CANVAS_ITEM
  real_position.SetX(matrix.GetA() * position.GetX() + matrix.GetB() * position.GetY() + matrix.GetE());
  real_position.SetY(matrix.GetB() * position.GetX() + matrix.GetD() * position.GetY() + matrix.GetF());
  return real_position;
}

wxSVGRect wxSVGTextElement::GetExtentOfChar(unsigned long charnum)
{
  wxSVGMatrix matrix = wxSVGLocatable::GetCTM(this);
  WX_SVG_CREATE_M_CANVAS_ITEM
  wxSVGRect extent = ((wxSVGCanvasText*)m_canvasItem)->GetExtentOfChar(charnum);
  WX_SVG_CLEAR_M_CANVAS_ITEM
  return extent.MatrixTransform(matrix);
}

double wxSVGTextElement::GetRotationOfChar(unsigned long charnum)
{
  WX_SVG_CREATE_M_CANVAS_ITEM
  double rotation = ((wxSVGCanvasText*)m_canvasItem)->GetRotationOfChar(charnum);
  WX_SVG_CLEAR_M_CANVAS_ITEM
  return rotation;
}

long wxSVGTextElement::GetCharNumAtPosition(const wxSVGPoint& point)
{
  WX_SVG_CREATE_M_CANVAS_ITEM
  long charnum = ((wxSVGCanvasText*)m_canvasItem)->GetCharNumAtPosition(point);
  WX_SVG_CLEAR_M_CANVAS_ITEM
  return charnum;
}
