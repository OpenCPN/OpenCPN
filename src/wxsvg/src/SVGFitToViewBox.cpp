//////////////////////////////////////////////////////////////////////////////
// Name:        SVGFitToViewBox.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2006/01/07
// RCS-ID:      $Id: SVGFitToViewBox.cpp,v 1.2 2014/07/06 15:20:38 ntalex Exp $
// Copyright:   (c) 2006 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGFitToViewBox.h"

void wxSVGFitToViewBox::UpdateMatrix(wxSVGMatrix& matrix, const wxSVGLength& width, const wxSVGLength& height) {
	// view box
	wxSVGRect viewbox = GetViewBox().GetAnimVal();
	if (viewbox.GetWidth() <= 0 || viewbox.GetHeight() <= 0)
		return;
	
	wxSVG_PRESERVEASPECTRATIO align = GetPreserveAspectRatio().GetAnimVal().GetAlign();
	if (align == wxSVG_PRESERVEASPECTRATIO_UNKNOWN) {
		align = wxSVG_PRESERVEASPECTRATIO_XMIDYMID;
	}
	
	if (align == wxSVG_PRESERVEASPECTRATIO_NONE) {
		matrix = matrix.ScaleNonUniform(width / viewbox.GetWidth(), height / viewbox.GetHeight());
	} else {
		double scale = 1;
		scale = width / viewbox.GetWidth();
		if (scale > height / viewbox.GetHeight())
			scale = height / viewbox.GetHeight();

		double x = 0;
		if (align == wxSVG_PRESERVEASPECTRATIO_XMIDYMIN
				|| align == wxSVG_PRESERVEASPECTRATIO_XMIDYMID
				|| align == wxSVG_PRESERVEASPECTRATIO_XMIDYMAX)
			x = (width - viewbox.GetWidth() * scale) / 2;
		else if (align == wxSVG_PRESERVEASPECTRATIO_XMAXYMIN
				|| align == wxSVG_PRESERVEASPECTRATIO_XMAXYMID
				|| align == wxSVG_PRESERVEASPECTRATIO_XMAXYMAX)
			x = width - viewbox.GetWidth() * scale;

		double y = 0;
		if (align == wxSVG_PRESERVEASPECTRATIO_XMINYMID
				|| align == wxSVG_PRESERVEASPECTRATIO_XMIDYMID
				|| align == wxSVG_PRESERVEASPECTRATIO_XMAXYMID)
			y = (height - viewbox.GetHeight() * scale) / 2;
		else if (align == wxSVG_PRESERVEASPECTRATIO_XMINYMAX
				|| align == wxSVG_PRESERVEASPECTRATIO_XMIDYMAX
				|| align == wxSVG_PRESERVEASPECTRATIO_XMAXYMAX)
			y = height - viewbox.GetHeight() * scale;

		if (x != 0 || y != 0)
			matrix = matrix.Translate(x, y);

		matrix = matrix.Scale(scale);
	}
	if (viewbox.GetX() != 0 || viewbox.GetY() != 0)
		matrix = matrix.Translate(-viewbox.GetX(), -viewbox.GetY());
}

