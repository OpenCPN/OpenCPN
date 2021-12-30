//////////////////////////////////////////////////////////////////////////////
// Name:        imagsvg.h
// Purpose:     SVG Image Handler
// Author:      Alex Thuering
// Created:     2011/11/22
// RCS-ID:      $Id: imagsvg.h,v 1.1 2011/11/22 20:08:35 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#ifndef IMAGE_SVG_H
#define IMAGE_SVG_H

#include <wx/image.h>

#define wxBITMAP_TYPE_SVG 45

class wxSVGHandler: public wxImageHandler {
public:
	wxSVGHandler();
	virtual ~wxSVGHandler();

#if wxUSE_STREAMS
	virtual bool LoadFile(wxImage *image, wxInputStream& stream, bool verbose = true, int index = -1);
	virtual bool SaveFile(wxImage *image, wxOutputStream& stream, bool verbose = true);
protected:
	virtual bool DoCanRead(wxInputStream& stream);
#endif

private:
    DECLARE_DYNAMIC_CLASS(wxImageHandler)
};

#endif // IMAGE_SVG_H
