//////////////////////////////////////////////////////////////////////////////
// Name:        SVGTextContentElement.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/04/29
// RCS-ID:      $Id: SVGTextContentElement.cpp,v 1.2 2005/06/17 13:24:50 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGTextContentElement.h"

long wxSVGTextContentElement::GetNumberOfChars()
{
  return 0;
}

double wxSVGTextContentElement::GetComputedTextLength()
{
  return 0;
}

double wxSVGTextContentElement::GetSubStringLength(unsigned long charnum, unsigned long nchars)
{
  return 0;
}

wxSVGPoint wxSVGTextContentElement::GetStartPositionOfChar(unsigned long charnum)
{
  wxSVGPoint res;
  return res;
}

wxSVGPoint wxSVGTextContentElement::GetEndPositionOfChar(unsigned long charnum)
{
  wxSVGPoint res;
  return res;
}

wxSVGRect wxSVGTextContentElement::GetExtentOfChar(unsigned long charnum)
{
  wxSVGRect res;
  return res;
}

double wxSVGTextContentElement::GetRotationOfChar(unsigned long charnum)
{
  return 0;
}

long wxSVGTextContentElement::GetCharNumAtPosition(const wxSVGPoint& point)
{
  return 0;
}

void wxSVGTextContentElement::SelectSubString(unsigned long charnum, unsigned long nchars)
{

}

