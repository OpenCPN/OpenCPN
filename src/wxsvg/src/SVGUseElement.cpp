//////////////////////////////////////////////////////////////////////////////
// Name:        SVGUseElement.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/09/21
// RCS-ID:      $Id: SVGUseElement.cpp,v 1.2 2006/01/08 12:44:30 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGUseElement.h"

wxSVGRect wxSVGUseElement::GetBBox(wxSVG_COORDINATES coordinates)
{
  wxSVGRect bbox = wxSVGRect(GetX().GetAnimVal(), GetY().GetAnimVal(),
	GetWidth().GetAnimVal(), GetHeight().GetAnimVal());
  if (coordinates != wxSVG_COORDINATES_USER)
    bbox.MatrixTransform(GetMatrix(coordinates));
  return bbox;
}

wxSVGRect wxSVGUseElement::GetResultBBox(wxSVG_COORDINATES coordinates)
{
  return GetBBox(coordinates);
}
