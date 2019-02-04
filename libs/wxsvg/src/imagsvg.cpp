//////////////////////////////////////////////////////////////////////////////
// Name:        imagsvg.cpp
// Purpose:     SVG Image Handler
// Author:      Alex Thuering
// Created:     2011/11/22
// RCS-ID:      $Id: imagsvg.cpp,v 1.2 2011/12/27 08:20:32 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "imagsvg.h"
#include "SVGDocument.h"

IMPLEMENT_DYNAMIC_CLASS(wxSVGHandler, wxImageHandler)

wxSVGHandler::wxSVGHandler() {
	m_name = wxT("SVG file");
	m_extension = wxT("svg");
	m_type = (wxBitmapType) wxBITMAP_TYPE_SVG;
	m_mime = wxT("image/svg");
}

wxSVGHandler::~wxSVGHandler() {
	// nothing to do
}

bool wxSVGHandler::LoadFile(wxImage *image, wxInputStream& stream, bool verbose, int index) {
	// save this before calling Destroy()
	int maxWidth = image->HasOption(wxT("max_width")) ? image->GetOptionInt(wxT("max_width")) : -1;
	int maxHeight = image->HasOption(wxT("max_height")) ? image->GetOptionInt(wxT("max_height")) : -1;
	image->Destroy();
	
	wxSVGDocument imgDoc;
	if (!imgDoc.Load(stream))
		return false;
	
	*image = imgDoc.Render(maxWidth, maxHeight);
	return true;
}

bool wxSVGHandler::SaveFile(wxImage *image, wxOutputStream& stream, bool verbose) {
	return false; // not implemented
}

bool wxSVGHandler::DoCanRead(wxInputStream& stream) {
	unsigned char hdr[5];
	
	if (!stream.Read(hdr, WXSIZEOF(hdr))) // it's ok to modify the stream position here
		return false;
	
	return hdr[0] == '<' && hdr[1] == '?' && hdr[2] == 'x' && hdr[3] == 'm' && hdr[4] == 'l'; // <?xml
}
