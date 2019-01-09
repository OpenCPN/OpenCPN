//////////////////////////////////////////////////////////////////////////////
// Name:        SVGPreserveAspectRatio.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/09/27
// RCS-ID:      $Id: SVGPreserveAspectRatio.cpp,v 1.3 2014/04/06 10:17:54 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGPreserveAspectRatio.h"
#include <wx/tokenzr.h>

wxString wxSVGPreserveAspectRatio::GetValueAsString() const {
	wxString value;
	switch (m_align) {
	case wxSVG_PRESERVEASPECTRATIO_UNKNOWN:
		return value;
	case wxSVG_PRESERVEASPECTRATIO_NONE:
		value = wxT("none");
		return value;
	case wxSVG_PRESERVEASPECTRATIO_XMINYMIN:
		value = wxT("xminymin");
		break;
	case wxSVG_PRESERVEASPECTRATIO_XMIDYMIN:
		value = wxT("xmidymin");
		break;
	case wxSVG_PRESERVEASPECTRATIO_XMAXYMIN:
		value = wxT("xmaxymin");
		break;
	case wxSVG_PRESERVEASPECTRATIO_XMINYMID:
		value = wxT("xminymid");
		break;
	case wxSVG_PRESERVEASPECTRATIO_XMIDYMID:
		value = wxT("xmidymid");
		break;
	case wxSVG_PRESERVEASPECTRATIO_XMAXYMID:
		value = wxT("xmaxymid");
		break;
	case wxSVG_PRESERVEASPECTRATIO_XMINYMAX:
		value = wxT("xminymax");
		break;
	case wxSVG_PRESERVEASPECTRATIO_XMIDYMAX:
		value = wxT("xmidymax");
		break;
	case wxSVG_PRESERVEASPECTRATIO_XMAXYMAX:
		value = wxT("xmaxymax");
		break;
	}

	if (value.length() && m_meetOrSlice != wxSVG_MEETORSLICE_UNKNOWN)
		value += wxT(" ");

	switch (m_meetOrSlice) {
	case wxSVG_MEETORSLICE_UNKNOWN:
		break;
	case wxSVG_MEETORSLICE_MEET:
		value += wxT("meet");
		break;
	case wxSVG_MEETORSLICE_SLICE:
		value += wxT("slice");
		break;
	}

	return value;
}

void wxSVGPreserveAspectRatio::SetValueAsString(const wxString& value) {
	m_align = wxSVG_PRESERVEASPECTRATIO_UNKNOWN;
	m_meetOrSlice = wxSVG_MEETORSLICE_UNKNOWN;

	wxString valueLower = value.Strip(wxString::both).Lower();
	wxString val = valueLower.BeforeFirst(wxT(' '));
	if (val == wxT("defer")) {
		valueLower = valueLower.AfterFirst(wxT(' '));
		val = valueLower.BeforeFirst(wxT(' '));
	}
	if (!val.length())
		return;
	else if (val == wxT("none"))
		m_align = wxSVG_PRESERVEASPECTRATIO_NONE;
	else if (val == wxT("xminymin"))
		m_align = wxSVG_PRESERVEASPECTRATIO_XMINYMIN;
	else if (val == wxT("xmidymin"))
		m_align = wxSVG_PRESERVEASPECTRATIO_XMIDYMIN;
	else if (val == wxT("xmaxymin"))
		m_align = wxSVG_PRESERVEASPECTRATIO_XMAXYMIN;
	else if (val == wxT("xminymid"))
		m_align = wxSVG_PRESERVEASPECTRATIO_XMINYMID;
	else if (val == wxT("xmidymid"))
		m_align = wxSVG_PRESERVEASPECTRATIO_XMIDYMID;
	else if (val == wxT("xmaxymid"))
		m_align = wxSVG_PRESERVEASPECTRATIO_XMAXYMID;
	else if (val == wxT("xminymax"))
		m_align = wxSVG_PRESERVEASPECTRATIO_XMINYMAX;
	else if (val == wxT("xmidymax"))
		m_align = wxSVG_PRESERVEASPECTRATIO_XMIDYMAX;
	else if (val == wxT("xmaxymax"))
		m_align = wxSVG_PRESERVEASPECTRATIO_XMAXYMAX;
	valueLower = valueLower.AfterFirst(wxT(' '));
	val = valueLower.BeforeFirst(wxT(' '));
	if (val == wxT("meet"))
		m_meetOrSlice = wxSVG_MEETORSLICE_MEET;
	else if (val == wxT("slice"))
		m_meetOrSlice = wxSVG_MEETORSLICE_SLICE;
}
