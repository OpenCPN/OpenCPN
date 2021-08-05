//////////////////////////////////////////////////////////////////////////////
// Name:        svgxml.h
// Purpose:     wxSvgXmlDocument - XML parser & data holder class
// Author:      Vaclav Slavik
// Created:     2000/03/05
// RCS-ID:      $Id: svgxml.h,v 1.8 2014/03/27 19:24:49 ntalex Exp $
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SVGXML_H_
#define _WX_SVGXML_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "svgxml.h"
#endif

#include <wx/string.h>
#include <wx/object.h>
#include <wx/list.h>
#include <wx/hashmap.h>
#include <vector>

class wxSvgXmlNode;
class wxSvgXmlProperty;
class wxSvgXmlDocument;
class wxSvgXmlIOHandler;
class wxInputStream;
class wxOutputStream;

class wxSVGElement;

// Represents XML node type.
enum wxSvgXmlNodeType
{
    // note: values are synchronized with xmlElementType from libxml
    wxSVGXML_ELEMENT_NODE       =  1,
    wxSVGXML_ATTRIBUTE_NODE     =  2,
    wxSVGXML_TEXT_NODE          =  3,
    wxSVGXML_CDATA_SECTION_NODE =  4,
    wxSVGXML_ENTITY_REF_NODE    =  5,
    wxSVGXML_ENTITY_NODE        =  6,
    wxSVGXML_PI_NODE            =  7,
    wxSVGXML_COMMENT_NODE       =  8,
    wxSVGXML_DOCUMENT_NODE      =  9,
    wxSVGXML_DOCUMENT_TYPE_NODE = 10,
    wxSVGXML_DOCUMENT_FRAG_NODE = 11,
    wxSVGXML_NOTATION_NODE      = 12,
    wxSVGXML_HTML_DOCUMENT_NODE = 13
};


// Represents node property(ies).
// Example: in <img src="hello.gif" id="3"/> "src" is property with value
//          "hello.gif" and "id" is prop. with value "3".

class wxSvgXmlProperty
{
public:
    wxSvgXmlProperty() : m_next(NULL) {}
    wxSvgXmlProperty(const wxString& name, const wxString& value,
                  wxSvgXmlProperty *next)
            : m_name(name), m_value(value), m_next(next) {}

    wxString GetName() const { return m_name; }
    wxString GetValue() const { return m_value; }
    wxSvgXmlProperty *GetNext() const { return m_next; }

    void SetName(const wxString& name) { m_name = name; }
    void SetValue(const wxString& value) { m_value = value; }
    void SetNext(wxSvgXmlProperty *next) { m_next = next; }

private:
    wxString m_name;
    wxString m_value;
    wxSvgXmlProperty *m_next;
};

class wxSvgXmlAttribute {
public:
	wxSvgXmlAttribute() {}
	wxSvgXmlAttribute(const wxString& name, const wxString& value): m_name(name), m_value(value) {}

    wxString GetName() const { return m_name; }
    wxString GetValue() const { return m_value; }

    void SetName(const wxString& name) { m_name = name; }
    void SetValue(const wxString& value) { m_value = value; }

private:
    wxString m_name;
    wxString m_value;
};

class wxSvgXmlAttrHash: public std::vector<wxSvgXmlAttribute> {
public:
    void Add(wxString name, wxString value) {
		push_back(wxSvgXmlAttribute(name, value));
	}
	void Add(const wxSvgXmlAttrHash& value) {
		wxSvgXmlAttrHash::const_iterator it;
		for (it = value.begin(); it != value.end(); ++it)
			push_back(*it);
	}
};

// Represents node in XML document. Node has name and may have content
// and properties. Most common node types are wxSVGXML_TEXT_NODE (name and props
// are irrelevant) and wxSVGXML_ELEMENT_NODE (e.g. in <title>hi</title> there is
// element with name="title", irrelevant content and one child (wxSVGXML_TEXT_NODE
// with content="hi").
//
// If wxUSE_UNICODE is 0, all strings are encoded in the encoding given to Load
// (default is UTF-8).

class wxSvgXmlNode
{
public:
    wxSvgXmlNode() : m_type(wxSVGXML_ELEMENT_NODE), m_properties(NULL), m_parent(NULL),
                  m_children(NULL), m_next(NULL), m_ownerDocument(NULL) {}
    wxSvgXmlNode(wxSvgXmlNode* parent, wxSvgXmlNodeType type,
              const wxString& name, const wxString& content,
              wxSvgXmlProperty* props, wxSvgXmlNode *next);
    virtual ~wxSvgXmlNode();

    // copy ctor & operator=. Note that this does NOT copy syblings
    // and parent pointer, i.e. m_parent and m_next will be NULL
    // after using copy ctor and are never unmodified by operator=.
    // On the other hand, it DOES copy children and properties.
    wxSvgXmlNode(const wxSvgXmlNode& node);
    wxSvgXmlNode& operator=(const wxSvgXmlNode& node);
	virtual wxSvgXmlNode* CloneNode(bool deep = true) { return new wxSvgXmlNode(*this); }

    // user-friendly creation:
    wxSvgXmlNode(wxSvgXmlNodeType type, const wxString& name,
              const wxString& content = wxEmptyString);

    void AddChild(wxSvgXmlNode* child);
	inline wxSvgXmlNode* AppendChild(wxSvgXmlNode* child)
	{ AddChild(child); return child; }

    void InsertChild(wxSvgXmlNode *child, wxSvgXmlNode *before_node);
	inline wxSvgXmlNode* InsertBefore(wxSvgXmlNode *newChild, wxSvgXmlNode *refChild)
	{ InsertChild(newChild, refChild); return newChild; }

    bool RemoveChild(wxSvgXmlNode *child);

    virtual void AddProperty(const wxString& name, const wxString& value);
    virtual bool DeleteProperty(const wxString& name);

    // access methods:
    wxSvgXmlNodeType GetType() const { return m_type; }
    wxString GetName() const { return m_name; }
    wxString GetContent() const { return m_content; }

    wxSvgXmlDocument *GetOwnerDocument() const { return m_ownerDocument; }
    wxSvgXmlNode *GetParent() const { return m_parent; }
    wxSvgXmlNode *GetNext() const { return m_next; }
    wxSvgXmlNode *GetChildren() const { return m_children; }

    wxSvgXmlNode* GetParentNode() const { return m_parent; }
    wxSvgXmlNode* GetChildNodes() const { return m_children; }
    wxSvgXmlNode* GetFirstChild() const { return m_children; }
    wxSvgXmlNode* GetLastChild() const;
    wxSvgXmlNode* GetPreviousSibling() const;
    wxSvgXmlNode* GetNextSibling() const { return m_next; }

    virtual wxSVGElement* GetSvgElement(){return NULL;}

    wxSvgXmlProperty *GetProperties() const { return m_properties; }
    bool GetPropVal(const wxString& propName, wxString *value) const;
    wxString GetPropVal(const wxString& propName,
                        const wxString& defaultVal) const;
    bool HasProp(const wxString& propName) const;

    void SetType(wxSvgXmlNodeType type) { m_type = type; }
    void SetName(const wxString& name) { m_name = name; }
    void SetContent(const wxString& con) { m_content = con; }

    void SetParent(wxSvgXmlNode *parent) { m_parent = parent; }
    void SetNext(wxSvgXmlNode *next) { m_next = next; }
    void SetChildren(wxSvgXmlNode *child) { m_children = child; }

    void SetProperties(wxSvgXmlProperty *prop) { m_properties = prop; }
    void AddProperty(wxSvgXmlProperty *prop);

public: // W3C DOM Methods
	virtual wxString GetAttribute(const wxString& name) const;
	virtual wxString GetAttributeNS(const wxString& namespaceURI,
									const wxString& localName) const;
    virtual bool SetAttribute(const wxString& name, const wxString& value);
	virtual bool SetAttributeNS(const wxString& namespaceURI,
								const wxString& qualifiedName,
								const wxString& value);
    virtual void RemoveAttribute(const wxString& name);
    virtual void RemoveAttributeNS(const wxString& namespaceURI,
								 const wxString& localName);
    virtual bool HasAttribute(const wxString& name) const;
    virtual bool HasAttributeNS(const wxString& namespaceURI,
								const wxString& localName) const;

    virtual wxSvgXmlAttrHash GetAttributes() const;

    void SetOwnerDocument(wxSvgXmlDocument* ownerDocument);

private:
    wxSvgXmlNodeType m_type;
    wxString m_name;
    wxString m_content;
    wxSvgXmlProperty *m_properties;
    wxSvgXmlNode *m_parent, *m_children, *m_next;
    wxSvgXmlDocument *m_ownerDocument;

    void DoCopy(const wxSvgXmlNode& node);
};


typedef wxSvgXmlNode wxSvgXmlElement;
typedef wxSvgXmlProperty wxSvgXmlAttr;

// This class holds XML data/document as parsed by XML parser.

class wxSvgXmlDocument : public wxObject
{
public:
    wxSvgXmlDocument();
    wxSvgXmlDocument(const wxString& filename,
                  const wxString& encoding = wxT("UTF-8"));
    wxSvgXmlDocument(wxInputStream& stream,
                  const wxString& encoding = wxT("UTF-8"));
    virtual ~wxSvgXmlDocument() { delete m_root; }

    wxSvgXmlDocument(const wxSvgXmlDocument& doc);
    wxSvgXmlDocument& operator=(const wxSvgXmlDocument& doc);

    // Parses .xml file and loads data. Returns TRUE on success, FALSE
    // otherwise.
    virtual bool Load(const wxString& filename,
              const wxString& encoding = wxT("UTF-8"));
    virtual bool Load(wxInputStream& stream,
              const wxString& encoding = wxT("UTF-8"));

    // Saves document as .xml file.
    bool Save(const wxString& filename) const;
    bool Save(wxOutputStream& stream) const;

    bool IsOk() const { return m_root != NULL; }

    // Returns root node of the document.
    wxSvgXmlNode *GetRoot() const { return m_root; }

    // Returns version of document (may be empty).
    wxString GetVersion() const { return m_version; }
    // Returns encoding of document (may be empty).
    // Note: this is the encoding original file was saved in, *not* the
    // encoding of in-memory representation!
    wxString GetFileEncoding() const { return m_fileEncoding; }

    // Write-access methods:
    void SetRoot(wxSvgXmlNode *node);
    void SetVersion(const wxString& version) { m_version = version; }
    void SetFileEncoding(const wxString& encoding) { m_fileEncoding = encoding; }

#if !wxUSE_UNICODE
    // Returns encoding of in-memory representation of the document
    // (same as passed to Load or ctor, defaults to UTF-8).
    // NB: this is meaningless in Unicode build where data are stored as wchar_t*
    wxString GetEncoding() const { return m_encoding; }
    void SetEncoding(const wxString& enc) { m_encoding = enc; }
#endif

public: // W3C DOM Methods
	virtual wxSvgXmlElement* CreateElement(const wxString& tagName);
	virtual wxSvgXmlElement* CreateElementNS(const wxString& namespaceURI,
										  const wxString& qualifiedName);

	inline wxSvgXmlNode* AppendChild(wxSvgXmlNode* child)
	{ if (!m_root) SetRoot(child); return child; }
	inline wxSvgXmlNode* RemoveChild(wxSvgXmlNode* child)
	{ if (m_root != child) return NULL; m_root = NULL; return child; }

	inline wxSvgXmlNode* GetFirstChild() { return m_root; }

private:
    wxString   m_version;
    wxString   m_fileEncoding;
#if !wxUSE_UNICODE
    wxString   m_encoding;
#endif
    wxSvgXmlNode *m_root;

    void DoCopy(const wxSvgXmlDocument& doc);
};

#endif // _WX_SVGXML_H_
