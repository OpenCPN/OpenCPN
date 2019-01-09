/////////////////////////////////////////////////////////////////////////////
// Author:      Vaclav Slavik
// Created:     2000/05/05
// RCS-ID:      Id: xmlhelpr.cpp,v 1.5 2001/06/09 22:16:41 VS Exp
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
    #pragma implementation "svgxmlhelpr.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/wx.h"
#include "wx/tokenzr.h"
#include "svgxmlhelpr.h"



wxSvgXmlNode *XmlFindNodeSimple(wxSvgXmlNode *parent, const wxString& param)
{
    if (param.IsEmpty()) return parent;

    wxSvgXmlNode *n = parent->GetChildren();
    
    while (n)
    {
        if (n->GetType() == wxSVGXML_ELEMENT_NODE && n->GetName() == param)
            return n;
        n = n->GetNext();
    }
    return NULL;
}



wxSvgXmlNode *XmlFindNode(wxSvgXmlNode *parent, const wxString& path)
{
    wxSvgXmlNode *n = parent;
    wxStringTokenizer tkn(path, _T("/"));
    while (tkn.HasMoreTokens())
    {
        n = XmlFindNodeSimple(n, tkn.GetNextToken());
        if (n == NULL) break;
    }
    return n;
}



wxSvgXmlNode *XmlCreateNode(wxSvgXmlNode *parent, const wxString& name)
{
    wxSvgXmlNode *n;
    wxString nm;

    wxStringTokenizer tkn(name, _T("/"));
    n = parent;
    while (tkn.HasMoreTokens())
    {
        parent = n;
        nm = tkn.GetNextToken();
        n = XmlFindNodeSimple(parent, nm);
        if (n) continue;
        
        // n == NULL:
        n = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, nm);
        parent->AddChild(n);        
    }
    n->AddChild(new wxSvgXmlNode(wxSVGXML_TEXT_NODE, wxEmptyString));
    
    return n;
}



void XmlWriteValue(wxSvgXmlNode *parent, const wxString& name, const wxString& value)
{
    wxSvgXmlNode *n = XmlFindNode(parent, name);
    if (n == NULL)
        n = XmlCreateNode(parent, name);
    
    n = n->GetChildren();
    
    while (n)
    {
        if (n->GetType() == wxSVGXML_TEXT_NODE || 
            n->GetType() == wxSVGXML_CDATA_SECTION_NODE)
        {
            n->SetContent(value);
            break;
        }
        n = n->GetNext();
    }
}



wxString XmlReadValue(wxSvgXmlNode *parent, const wxString& name)
{
    wxSvgXmlNode *n = XmlFindNode(parent, name);
    if (n == NULL) return wxEmptyString;
    n = n->GetChildren();
    
    while (n)
    {
        if (n->GetType() == wxSVGXML_TEXT_NODE || 
            n->GetType() == wxSVGXML_CDATA_SECTION_NODE)
            return n->GetContent();
        n = n->GetNext();
    }
    return wxEmptyString;
}



wxString XmlGetClass(wxSvgXmlNode *parent)
{
    return parent->GetPropVal(_T("class"), wxEmptyString);
}



void XmlSetClass(wxSvgXmlNode *parent, const wxString& classname)
{
    parent->DeleteProperty(_T("class"));
    parent->AddProperty(_T("class"), classname);
}





