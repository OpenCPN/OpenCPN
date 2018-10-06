//////////////////////////////////////////////////////////////////////////////
// Name:        SVGFilterElement.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/05/10
// RCS-ID:      $Id: SVGFilterElement.cpp,v 1.1.1.1 2005/05/10 17:51:39 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGFilterElement.h"

void wxSVGFilterElement::SetFilterRes(unsigned long filterResX, unsigned long filterResY)
{
  SetFilterResX(filterResX);
  SetFilterResY(filterResY);
}

