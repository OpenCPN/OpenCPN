//////////////////////////////////////////////////////////////////////////////
// Purpose:     XML resources editor
// Author:      Vaclav Slavik
// Created:     2000/05/05
// RCS-ID:      Id: svgxmlhelpr.h,v 1.4 2002/09/07 12:15:24 GD Exp
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
    #pragma interface "svgxmlhelpr.h"
#endif

#ifndef _SVGXMLHELPR_H_
#define _SVGXMLHELPR_H_

#include "svgxml.h"

// some helper functions:

void XmlWriteValue(wxSvgXmlNode *parent, const wxString& name, const wxString& value);
wxString XmlReadValue(wxSvgXmlNode *parent, const wxString& name);

// Finds a subnode of parent named <name>
// (may be recursive, e.g. "name1/name2" means
// <parent><name1><name2>value</name2></name1></parent>
wxSvgXmlNode *XmlFindNode(wxSvgXmlNode *parent, const wxString& name);
wxSvgXmlNode *XmlFindNodeSimple(wxSvgXmlNode *parent, const wxString& path);

wxString XmlGetClass(wxSvgXmlNode *parent);
void XmlSetClass(wxSvgXmlNode *parent, const wxString& classname);

#endif
