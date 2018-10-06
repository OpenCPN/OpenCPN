/////////////////////////////////////////////////////////////////////////////
// Name:        xml.cpp
// Purpose:     wxSvgXmlDocument - XML parser & data holder class
// Author:      Vaclav Slavik
// Created:     2000/03/05
// RCS-ID:      $Id: svgxml.cpp,v 1.9 2014/03/27 19:24:49 ntalex Exp $
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "svgxml.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "svgxml.h"

#include <expat.h>

#include "wx/wfstream.h"
#include "wx/datstrm.h"
#include "wx/zstream.h"
#include "wx/log.h"
#include "wx/intl.h"
#include "wx/strconv.h"
#include "wx/string.h"

// DLL options compatibility check:
#include "wx/app.h"
//WX_CHECK_BUILD_OPTIONS("wxSVGXML")



//-----------------------------------------------------------------------------
//  wxSvgXmlNode
//-----------------------------------------------------------------------------

wxSvgXmlNode::wxSvgXmlNode(wxSvgXmlNode *parent,wxSvgXmlNodeType type,
                     const wxString& name, const wxString& content,
                     wxSvgXmlProperty *props, wxSvgXmlNode *next)
    : m_type(type), m_name(name), m_content(content),
      m_properties(props), m_parent(parent),
      m_children(NULL), m_next(next), m_ownerDocument(NULL)
{
    if (m_parent)
    {
        if (m_parent->m_children)
        {
            m_next = m_parent->m_children;
            m_parent->m_children = this;
        }
        else
            m_parent->m_children = this;
    }
}

wxSvgXmlNode::wxSvgXmlNode(wxSvgXmlNodeType type, const wxString& name,
                     const wxString& content)
    : m_type(type), m_name(name), m_content(content),
      m_properties(NULL), m_parent(NULL),
      m_children(NULL), m_next(NULL), m_ownerDocument(NULL)
{}

wxSvgXmlNode::wxSvgXmlNode(const wxSvgXmlNode& node)
{
    m_next = NULL;
    m_parent = NULL;
    DoCopy(node);
}

wxSvgXmlNode::~wxSvgXmlNode()
{
    wxSvgXmlNode *c, *c2;
    for (c = m_children; c; c = c2)
    {
        c2 = c->m_next;
        delete c;
    }

    wxSvgXmlProperty *p, *p2;
    for (p = m_properties; p; p = p2)
    {
        p2 = p->GetNext();
        delete p;
    }
}

wxSvgXmlNode& wxSvgXmlNode::operator=(const wxSvgXmlNode& node)
{
    wxDELETE(m_properties);
    wxDELETE(m_children);
    DoCopy(node);
    return *this;
}

void wxSvgXmlNode::DoCopy(const wxSvgXmlNode& node)
{
    m_type = node.m_type;
    m_name = node.m_name;
    m_content = node.m_content;
    m_children = NULL;

    wxSvgXmlNode *n = node.m_children;
    while (n)
    {
        AddChild(n->CloneNode());
        n = n->GetNext();
    }

    m_properties = NULL;
    wxSvgXmlProperty *p = node.m_properties;
    while (p)
    {
       AddProperty(p->GetName(), p->GetValue());
       p = p->GetNext();
    }
}

void wxSvgXmlNode::SetOwnerDocument(wxSvgXmlDocument* ownerDocument)
{
    m_ownerDocument = ownerDocument;
    wxSvgXmlNode *child = GetChildren();
    while (child)
    {
        child->SetOwnerDocument(ownerDocument);
        child = child->GetNext();
    }
}

bool wxSvgXmlNode::HasProp(const wxString& propName) const
{
    wxSvgXmlProperty *prop = GetProperties();

    while (prop)
    {
        if (prop->GetName() == propName) return TRUE;
        prop = prop->GetNext();
    }

    return FALSE;
}

bool wxSvgXmlNode::GetPropVal(const wxString& propName, wxString *value) const
{
    wxSvgXmlProperty *prop = GetProperties();

    while (prop)
    {
        if (prop->GetName() == propName)
        {
            *value = prop->GetValue();
            return TRUE;
        }
        prop = prop->GetNext();
    }

    return FALSE;
}

wxString wxSvgXmlNode::GetPropVal(const wxString& propName, const wxString& defaultVal) const
{
    wxString tmp;
    if (GetPropVal(propName, &tmp))
        return tmp;

    return defaultVal;
}

void wxSvgXmlNode::AddChild(wxSvgXmlNode *child)
{
    if (m_children == NULL)
        m_children = child;
    else
    {
        wxSvgXmlNode *ch = m_children;
        while (ch->m_next) ch = ch->m_next;
        ch->m_next = child;
    }
    child->m_next = NULL;
    child->m_parent = this;
    child->SetOwnerDocument(m_ownerDocument);
}

void wxSvgXmlNode::InsertChild(wxSvgXmlNode *child, wxSvgXmlNode *before_node)
{
    wxASSERT_MSG(before_node == NULL || before_node->GetParent() == this,
    		wxT("wxSvgXmlNode::InsertChild - the node has incorrect parent"));

    if (m_children == before_node)
       m_children = child;
    else
    {
        wxSvgXmlNode *ch = m_children;
        while (ch->m_next != before_node) ch = ch->m_next;
        ch->m_next = child;
    }

    child->m_parent = this;
    child->m_next = before_node;
    child->SetOwnerDocument(m_ownerDocument);
}

bool wxSvgXmlNode::RemoveChild(wxSvgXmlNode *child)
{
    if (m_children == NULL)
        return FALSE;
    else if (m_children == child)
    {
        m_children = child->m_next;
        child->m_parent = NULL;
        child->m_next = NULL;
        return TRUE;
    }
    else
    {
        wxSvgXmlNode *ch = m_children;
        while (ch->m_next)
        {
            if (ch->m_next == child)
            {
                ch->m_next = child->m_next;
                child->m_parent = NULL;
                child->m_next = NULL;
                return TRUE;
            }
            ch = ch->m_next;
        }
        return FALSE;
    }
}

wxSvgXmlNode* wxSvgXmlNode::GetLastChild() const
{
    wxSvgXmlNode* child = m_children;
    if (child)
    {
        while (child->m_next)
            child = child->m_next;
    }
    return child;
}

wxSvgXmlNode* wxSvgXmlNode::GetPreviousSibling() const
{
    if (!m_parent)
        return NULL;
    wxSvgXmlNode* node = m_parent->m_children;
    if (node == this || node == NULL)
        return NULL;
    while (node->m_next)
    {
        if (node->m_next == this)
          return node;
        node = node->m_next;
    }
    return NULL;
}

void wxSvgXmlNode::AddProperty(const wxString& name, const wxString& value)
{
    AddProperty(new wxSvgXmlProperty(name, value, NULL));
}

void wxSvgXmlNode::AddProperty(wxSvgXmlProperty *prop)
{
    if (m_properties == NULL)
        m_properties = prop;
    else
    {
        wxSvgXmlProperty *p = m_properties;
        while (p->GetNext()) p = p->GetNext();
        p->SetNext(prop);
    }
}

bool wxSvgXmlNode::DeleteProperty(const wxString& name)
{
    wxSvgXmlProperty *prop;

    if (m_properties == NULL)
        return FALSE;

    else if (m_properties->GetName() == name)
    {
        prop = m_properties;
        m_properties = prop->GetNext();
        prop->SetNext(NULL);
        delete prop;
        return TRUE;
    }

    else
    {
        wxSvgXmlProperty *p = m_properties;
        while (p->GetNext())
        {
            if (p->GetNext()->GetName() == name)
            {
                prop = p->GetNext();
                p->SetNext(prop->GetNext());
                prop->SetNext(NULL);
                delete prop;
                return TRUE;
            }
            p = p->GetNext();
        }
        return FALSE;
    }
}

wxString wxSvgXmlNode::GetAttribute(const wxString& name) const {
	wxString val;
	GetPropVal(name, &val);
	return val;
}

wxString wxSvgXmlNode::GetAttributeNS(const wxString& namespaceURI, const wxString& localName) const {
	return GetAttribute(localName);
}

bool wxSvgXmlNode::SetAttribute(const wxString& name, const wxString& value) {
	wxSvgXmlProperty *prop = GetProperties();
	while (prop) {
		if (prop->GetName() == name) {
			prop->SetValue(value);
			return true;
		}
		prop = prop->GetNext();
	}
	AddProperty(name, value);
	return true;
}

bool wxSvgXmlNode::SetAttributeNS(const wxString& namespaceURI, const wxString& qualifiedName, const wxString& value) {
	return SetAttribute(qualifiedName, value);
}

void wxSvgXmlNode::RemoveAttribute(const wxString& name) {
	DeleteProperty(name);
}

void wxSvgXmlNode::RemoveAttributeNS(const wxString& namespaceURI, const wxString& localName) {
	RemoveAttribute(localName);
}

bool wxSvgXmlNode::HasAttribute(const wxString& name) const {
  return HasProp(name);
}

bool wxSvgXmlNode::HasAttributeNS(const wxString& namespaceURI, const wxString& localName) const {
  return HasAttribute(localName);
}

wxSvgXmlAttrHash wxSvgXmlNode::GetAttributes() const {
  wxSvgXmlAttrHash attributes;
  wxSvgXmlProperty *prop = GetProperties();
  while (prop)
  {
    attributes.Add(prop->GetName(), prop->GetValue());
    prop = prop->GetNext();
  }
  return attributes;
}

//-----------------------------------------------------------------------------
//  wxSvgXmlDocument
//-----------------------------------------------------------------------------

wxSvgXmlDocument::wxSvgXmlDocument()
    : m_version(wxT("1.0")), m_fileEncoding(wxT("utf-8")), m_root(NULL)
{
#if !wxUSE_UNICODE
    m_encoding = wxT("UTF-8");
#endif
}

wxSvgXmlDocument::wxSvgXmlDocument(const wxString& filename, const wxString& encoding)
                          : wxObject(), m_root(NULL)
{
    if ( !Load(filename, encoding) )
    {
        wxDELETE(m_root);
    }
}

wxSvgXmlDocument::wxSvgXmlDocument(wxInputStream& stream, const wxString& encoding)
                          : wxObject(), m_root(NULL)
{
    if ( !Load(stream, encoding) )
    {
        wxDELETE(m_root);
    }
}

wxSvgXmlDocument::wxSvgXmlDocument(const wxSvgXmlDocument& doc)
{
    DoCopy(doc);
}

wxSvgXmlDocument& wxSvgXmlDocument::operator=(const wxSvgXmlDocument& doc)
{
    wxDELETE(m_root);
    DoCopy(doc);
    return *this;
}

void wxSvgXmlDocument::SetRoot(wxSvgXmlNode *node)
{
    delete m_root;
    m_root = node;
    if (m_root)
        m_root->SetOwnerDocument(this);
}

wxSvgXmlElement* wxSvgXmlDocument::CreateElement(const wxString& tagName)
{
  return new wxSvgXmlElement(wxSVGXML_ELEMENT_NODE, tagName);
}

wxSvgXmlElement* wxSvgXmlDocument::CreateElementNS(const wxString& namespaceURI,
											 const wxString& qualifiedName)
{
  return CreateElement(qualifiedName);
}

void wxSvgXmlDocument::DoCopy(const wxSvgXmlDocument& doc)
{
    m_version = doc.m_version;
#if !wxUSE_UNICODE
    m_encoding = doc.m_encoding;
#endif
    m_fileEncoding = doc.m_fileEncoding;
    m_root = new wxSvgXmlNode(*doc.m_root);
}

bool wxSvgXmlDocument::Load(const wxString& filename, const wxString& encoding) {
	if (!wxFile::Exists(filename)) {
		wxLogError(_("File %s doesn't exist."), filename.c_str());
		return false;
	}
    wxFileInputStream stream(filename);
    return Load(stream, encoding);
}

bool wxSvgXmlDocument::Save(const wxString& filename) const
{
    wxFileOutputStream stream(filename);
    return Save(stream);
}



//-----------------------------------------------------------------------------
//  wxSvgXmlDocument loading routines
//-----------------------------------------------------------------------------

/*
    FIXME:
       - process all elements, including CDATA
 */

// converts Expat-produced string in UTF-8 into wxString.
inline static wxString CharToString(wxMBConv *conv,
                                    const char *s, size_t len = wxString::npos)
{
#if wxUSE_UNICODE
    (void)conv;
    return wxString(s, wxConvUTF8, len);
#else
    if ( conv )
    {
        size_t nLen = (len != wxSTRING_MAXLEN) ? len :
                          wxConvUTF8.MB2WC((wchar_t*) NULL, s, 0);

        wchar_t *buf = new wchar_t[nLen+1];
        wxConvUTF8.MB2WC(buf, s, nLen);
        buf[nLen] = 0;
        wxString str(buf, *conv, len);
        delete[] buf;
        return str;
    }
    else
        return wxString(s, len != wxSTRING_MAXLEN ? len : strlen(s));
#endif
}

struct wxSvgXmlParsingContext
{
	wxSvgXmlDocument* doc;
    wxMBConv  *conv;
    wxSvgXmlNode *root;
    wxSvgXmlNode *node;
    wxSvgXmlNode *lastAsText;
    wxString   encoding;
    wxString   version;
};

static void StartElementHnd(void *userData, const char *name, const char **atts)
{
    wxSvgXmlParsingContext *ctx = (wxSvgXmlParsingContext*)userData;
	wxSvgXmlElement* node = ctx->doc->CreateElement(CharToString(ctx->conv, name));
    const char **a = atts;
    while (*a)
    {
        node->AddProperty(CharToString(ctx->conv, a[0]), CharToString(ctx->conv, a[1]));
        a += 2;
    }
    if (ctx->root == NULL)
        ctx->root = node;
    else
        ctx->node->AddChild(node);
    ctx->node = node;
    ctx->lastAsText = NULL;
}

static void EndElementHnd(void *userData, const char* WXUNUSED(name))
{
    wxSvgXmlParsingContext *ctx = (wxSvgXmlParsingContext*)userData;

    ctx->node = ctx->node->GetParent();
    ctx->lastAsText = NULL;
}

static void TextHnd(void *userData, const char *s, int len)
{
    wxSvgXmlParsingContext *ctx = (wxSvgXmlParsingContext*)userData;
    char *buf = new char[len + 1];

    buf[len] = '\0';
    memcpy(buf, s, (size_t)len);

    if (ctx->lastAsText)
    {
        ctx->lastAsText->SetContent(ctx->lastAsText->GetContent() +
                                    CharToString(ctx->conv, buf));
    }
    else
    {
        bool whiteOnly = TRUE;
        for (char *c = buf; *c != '\0'; c++)
            if (*c != ' ' && *c != '\t' && *c != '\n' && *c != '\r')
            {
                whiteOnly = FALSE;
                break;
            }
        if (!whiteOnly)
        {
            ctx->lastAsText = new wxSvgXmlNode(wxSVGXML_TEXT_NODE, wxT("text"),
                                            CharToString(ctx->conv, buf));
            ctx->node->AddChild(ctx->lastAsText);
        }
    }

    delete[] buf;
}

static void CommentHnd(void *userData, const char *data)
{
    wxSvgXmlParsingContext *ctx = (wxSvgXmlParsingContext*)userData;

    if (ctx->node)
    {
        // VS: ctx->node == NULL happens if there is a comment before
        //     the root element (e.g. wxDesigner's output). We ignore such
        //     comments, no big deal...
        ctx->node->AddChild(new wxSvgXmlNode(wxSVGXML_COMMENT_NODE,
                            wxT("comment"), CharToString(ctx->conv, data)));
    }
    ctx->lastAsText = NULL;
}

static void DefaultHnd(void *userData, const char *s, int len)
{
    // XML header:
    if (len > 6 && memcmp(s, "<?xml ", 6) == 0)
    {
        wxSvgXmlParsingContext *ctx = (wxSvgXmlParsingContext*)userData;

        wxString buf = CharToString(ctx->conv, s, (size_t)len);
        int pos;
        pos = buf.Find(wxT("encoding="));
        if (pos != wxNOT_FOUND)
            ctx->encoding = buf.Mid(pos + 10).BeforeFirst(buf[(size_t)pos+9]);
        pos = buf.Find(wxT("version="));
        if (pos != wxNOT_FOUND)
            ctx->version = buf.Mid(pos + 9).BeforeFirst(buf[(size_t)pos+8]);
    }
}

static int UnknownEncodingHnd(void * WXUNUSED(encodingHandlerData),
                              const XML_Char *name, XML_Encoding *info)
{
    // We must build conversion table for expat. The easiest way to do so
    // is to let wxCSConv convert as string containing all characters to
    // wide character representation:
    wxString str(name, wxConvLibc);
    wxCSConv conv(str);
    char mbBuf[2];
    wchar_t wcBuf[10];
    size_t i;

    mbBuf[1] = 0;
    info->map[0] = 0;
    for (i = 0; i < 255; i++)
    {
        mbBuf[0] = (char)(i+1);
        if (conv.MB2WC(wcBuf, mbBuf, 2) == (size_t)-1)
        {
            // invalid/undefined byte in the encoding:
            info->map[i+1] = -1;
        }
        info->map[i+1] = (int)wcBuf[0];
    }

    info->data = NULL;
    info->convert = NULL;
    info->release = NULL;

    return 1;
}

bool wxSvgXmlDocument::Load(wxInputStream& stream, const wxString& encoding)
{
#if wxUSE_UNICODE
    (void)encoding;
#else
    m_encoding = encoding;
#endif

    const size_t BUFSIZE = 1024;
    char buf[BUFSIZE];
    wxSvgXmlParsingContext ctx;
    bool done;
    XML_Parser parser = XML_ParserCreate(NULL);

	ctx.doc = this;
    ctx.root = ctx.node = NULL;
    ctx.encoding = wxT("UTF-8"); // default in absence of encoding=""
    ctx.conv = NULL;
#if !wxUSE_UNICODE
    if ( encoding != wxT("UTF-8") && encoding != wxT("utf-8") )
        ctx.conv = new wxCSConv(encoding);
#endif

    XML_SetUserData(parser, (void*)&ctx);
    XML_SetElementHandler(parser, StartElementHnd, EndElementHnd);
    XML_SetCharacterDataHandler(parser, TextHnd);
    XML_SetCommentHandler(parser, CommentHnd);
    XML_SetDefaultHandler(parser, DefaultHnd);
    XML_SetUnknownEncodingHandler(parser, UnknownEncodingHnd, NULL);

    bool ok = true;
    do
    {
        size_t len = stream.Read(buf, BUFSIZE).LastRead();
        done = (len < BUFSIZE);
        if (!XML_Parse(parser, buf, len, done))
        {
            wxString error(XML_ErrorString(XML_GetErrorCode(parser)),
                           *wxConvCurrent);
            wxLogError(_("XML parsing error: '%s' at line %d"),
                       error.c_str(),
                       XML_GetCurrentLineNumber(parser));
            ok = false;
            break;
        }
    } while (!done);

    if (ok)
    {
        if (!ctx.version.IsEmpty())
            SetVersion(ctx.version);
        if (!ctx.encoding.IsEmpty())
            SetFileEncoding(ctx.encoding);
        SetRoot(ctx.root);
    }
    else
    {
        delete ctx.root;
    }

    XML_ParserFree(parser);
#if !wxUSE_UNICODE
    if ( ctx.conv )
        delete ctx.conv;
#endif

    return ok;

}



//-----------------------------------------------------------------------------
//  wxSvgXmlDocument saving routines
//-----------------------------------------------------------------------------

// write string to output:
inline static void OutputString(wxOutputStream& stream, const wxString& str,
#if wxUSE_UNICODE
    wxMBConv * WXUNUSED(convMem),
#else
    wxMBConv *convMem,
#endif
    wxMBConv *convFile)
{
    if (str.IsEmpty()) return;
#if wxUSE_UNICODE
    const wxWX2MBbuf buf(str.mb_str(*(convFile ? convFile : &wxConvUTF8)));
    stream.Write((const char*)buf, strlen((const char*)buf));
#else
    if ( convFile == NULL )
        stream.Write(str.mb_str(), str.Len());
    else
    {
        wxString str2(str.wc_str(*convMem), *convFile);
        stream.Write(str2.mb_str(), str2.Len());
    }
#endif
}

// Same as above, but create entities first.
// Translates '<' to "&lt;", '>' to "&gt;" and '&' to "&amp;"
static void OutputStringEnt(wxOutputStream& stream, const wxString& str,
                            wxMBConv *convMem, wxMBConv *convFile,
                            bool escapeQuotes = false)
{
    wxString buf;
    size_t i, last, len;
    wxChar c;

    len = str.Len();
    last = 0;
    for (i = 0; i < len; i++)
    {
        c = str.GetChar(i);
        if (c == wxT('<') || c == wxT('>') ||
            (c == wxT('&') && str.Mid(i+1, 4) != wxT("amp;")) ||
            (escapeQuotes && c == wxT('"')))
        {
            OutputString(stream, str.Mid(last, i - last), convMem, convFile);
            switch (c)
            {
                case wxT('<'):
                    OutputString(stream, wxT("&lt;"), NULL, NULL);
                    break;
                case wxT('>'):
                    OutputString(stream, wxT("&gt;"), NULL, NULL);
                    break;
                case wxT('&'):
                    OutputString(stream, wxT("&amp;"), NULL, NULL);
                    break;
                case wxT('"'):
                    OutputString(stream, wxT("&quot;"), NULL, NULL);
                    break;
                default: break;
            }
            last = i + 1;
        }
    }
    OutputString(stream, str.Mid(last, i - last), convMem, convFile);
}

inline static void OutputIndentation(wxOutputStream& stream, int indent)
{
    wxString str = wxT("\n");
    for (int i = 0; i < indent; i++)
        str << wxT(' ') << wxT(' ');
    OutputString(stream, str, NULL, NULL);
}

static void OutputNode(wxOutputStream& stream, wxSvgXmlNode *node, int indent,
                       wxMBConv *convMem, wxMBConv *convFile)
{
    wxSvgXmlNode *n, *prev;
    wxSvgXmlAttrHash attributes;

    switch (node->GetType())
    {
        case wxSVGXML_TEXT_NODE:
            OutputStringEnt(stream, node->GetContent(), convMem, convFile);
            break;

        case wxSVGXML_ELEMENT_NODE:
            OutputString(stream, wxT("<"), NULL, NULL);
            OutputString(stream, node->GetName(), NULL, NULL);

            attributes = node->GetAttributes();
            for (wxSvgXmlAttrHash::iterator it = attributes.begin(); it != attributes.end(); ++it) {
                OutputString(stream, wxT(" ") + it->GetName() +  wxT("=\""), NULL, NULL);
                OutputStringEnt(stream, it->GetValue(), NULL, NULL, true/*escapeQuotes*/);
                OutputString(stream, wxT("\""), NULL, NULL);
            }

            if (node->GetChildren())
            {
                OutputString(stream, wxT(">"), NULL, NULL);
                prev = NULL;
                n = node->GetChildren();
                while (n)
                {
                    if (n && n->GetType() != wxSVGXML_TEXT_NODE && node->GetAttribute(wxT("xml:space")) != wxT("preserve"))
                        OutputIndentation(stream, indent + 1);
                    OutputNode(stream, n, indent + 1, convMem, convFile);
                    prev = n;
                    n = n->GetNext();
                }
                if (prev && prev->GetType() != wxSVGXML_TEXT_NODE)
                    OutputIndentation(stream, indent);
                OutputString(stream, wxT("</"), NULL, NULL);
                OutputString(stream, node->GetName(), NULL, NULL);
                OutputString(stream, wxT(">"), NULL, NULL);
            }
            else
                OutputString(stream, wxT("/>"), NULL, NULL);
            break;

        case wxSVGXML_COMMENT_NODE:
            OutputString(stream, wxT("<!--"), NULL, NULL);
            OutputString(stream, node->GetContent(), convMem, convFile);
            OutputString(stream, wxT("-->"), NULL, NULL);
            break;

        default:
            wxFAIL_MSG(wxT("unsupported node type"));
    }
}

bool wxSvgXmlDocument::Save(wxOutputStream& stream) const
{
    if ( !IsOk() )
        return FALSE;

    wxString s;

    wxMBConv *convMem = NULL, *convFile = NULL;
#if wxUSE_UNICODE
    convFile = new wxCSConv(GetFileEncoding());
#else
    if ( GetFileEncoding() != GetEncoding() )
    {
        convFile = new wxCSConv(GetFileEncoding());
        convMem = new wxCSConv(GetEncoding());
    }
#endif

    s.Printf(wxT("<?xml version=\"%s\" encoding=\"%s\"?>\n"),
             GetVersion().c_str(), GetFileEncoding().c_str());
    OutputString(stream, s, NULL, NULL);

    OutputNode(stream, GetRoot(), 0, convMem, convFile);
    OutputString(stream, wxT("\n"), NULL, NULL);

    if ( convFile )
        delete convFile;
    if ( convMem )
        delete convMem;

    return TRUE;
}
