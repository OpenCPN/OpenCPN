/////////////////////////////////////////////////////////////////////////////
// Name:        ExifHandler.cpp
// Author:      Alex Thuering
// Created:		30.12.2007
// RCS-ID:      $Id: ExifHandler.cpp,v 1.2 2016/10/23 18:54:56 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "ExifHandler.h"
#include <libexif/exif-loader.h>
#include <wx/log.h>

/** Returns the orientation tag that indicates the orientation of the captured scene */
int ExifHandler::getOrient(const wxString& filename) {
	ExifData* exifData = exif_data_new_from_file(filename.mb_str());
	if (!exifData)
		return -1;
	if (!exif_content_get_entry(exifData->ifd[EXIF_IFD_EXIF], EXIF_TAG_EXIF_VERSION))
		return -1;
	int orient = -1;
	ExifEntry* entry = exif_content_get_entry(exifData->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
	if (entry) {
		ExifByteOrder byteOrder = exif_data_get_byte_order(exifData);
		orient = exif_get_short(entry->data, byteOrder);
	}
	exif_data_unref(exifData);
	return (int) orient;
}

/** Rotates the image according to orientation tag  */
void ExifHandler::rotateImage(const wxString& filename, wxImage& image) {
	if (!image.Ok() || filename.length() < 5 || filename.Mid(filename.length() - 4).Lower() != wxT(".jpg"))
		return;
	int orient = getOrient(filename);
	switch (orient) {
	case 2: // horizontally mirror
		image = image.Mirror();
		break;
	case 3: // rotated 180
		image = image.Rotate90().Rotate90();
		break;
	case 4: // vertically mirror
		image = image.Mirror(false);
		break;
	case 5: // 90 CW and horizontally mirror
		image = image.Rotate90().Mirror();
		break;
	case 6: // 90 CW
		image = image.Rotate90();
		break;
	case 7: // 90 CW and vertically mirror
		image = image.Rotate90().Mirror(false);
		break;
	case 8: // 90 CCW
		image = image.Rotate90(false);
		break;
	default:
		break;
	}
}
