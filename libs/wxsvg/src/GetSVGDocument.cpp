//////////////////////////////////////////////////////////////////////////////
// Name:        GetSVGDocument.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/05/10
// RCS-ID:      $Id: GetSVGDocument.cpp,v 1.1.1.1 2005/05/10 17:51:39 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "GetSVGDocument.h"

static wxSVGDocument* s_svgDocument = NULL;

wxSVGDocument* wxGetSVGDocument::GetSVGDocument()
{
  if (!s_svgDocument)
    s_svgDocument = new wxSVGDocument();
  return s_svgDocument;
}
