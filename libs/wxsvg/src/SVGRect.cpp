//////////////////////////////////////////////////////////////////////////////
// Name:        SVGRect.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/09/27
// RCS-ID:      $Id: SVGRect.cpp,v 1.5 2006/02/26 14:50:01 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGRect.h"
#include "SVGPoint.h"
#include <wx/tokenzr.h>

wxString wxSVGRect::GetValueAsString() const
{
  return wxString::Format(wxT("%g %g %g %g"),
    GetX(), GetY(), GetWidth(), GetHeight());
}

void wxSVGRect::SetValueAsString(const wxString& value)
{
  double val;
  wxStringTokenizer tkz(value, wxT(", \\t"));
  int pi = 0;
  while (tkz.HasMoreTokens() && pi<4) 
  { 
    wxString token = tkz.GetNextToken(); 
    token.ToDouble(&val);
    switch (pi)
    {
      case 0: SetX(val); break;
      case 1: SetY(val); break;
      case 2: SetWidth(val); break;
      case 3: SetHeight(val); break;
    }
    pi++;
  }
}

wxSVGRect wxSVGRect::MatrixTransform(const wxSVGMatrix& matrix) const
{
  wxSVGPoint point1(GetX(), GetY());
  point1 = point1.MatrixTransform(matrix);
  wxSVGPoint point2(GetX() + GetWidth(), GetY() + GetHeight());
  point2 = point2.MatrixTransform(matrix);
  
  wxSVGRect rect;
  rect.SetX(point1.GetX() < point2.GetX() ? point1.GetX() : point2.GetX());
  rect.SetY(point1.GetY() < point2.GetY() ? point1.GetY() : point2.GetY());
  rect.SetWidth(point1.GetX() < point2.GetX() ? point2.GetX() - rect.GetX() : point1.GetX() - rect.GetX());
  rect.SetHeight(point1.GetY() < point2.GetY() ? point2.GetY() - rect.GetY() : point1.GetY() - rect.GetY());
  return rect;
}
