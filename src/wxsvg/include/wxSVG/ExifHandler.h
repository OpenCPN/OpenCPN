/////////////////////////////////////////////////////////////////////////////
// Name:        ExifHandler.h
// Author:      Alex Thuering
// Created:		30.12.2007
// RCS-ID:      $Id: ExifHandler.h,v 1.1 2016/10/23 18:34:03 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef WXSVG_EXIFHANDLER_H
#define WXSVG_EXIFHANDLER_H

#include <wx/string.h>
#include <wx/image.h>

/** Reads EXIF metainformation from image files */
class ExifHandler {
  public:
	/** Returns the orientation tag that indicates the orientation of the captured scene */
	static int getOrient(const wxString& filename);
	/** Rotates the image according to orientation tag  */
	static void rotateImage(const wxString& filename, wxImage& image);
};

#endif // WXSVG_EXIFHANDLER_H
