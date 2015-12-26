//////////////////////////////////////////////////////////////////////////////
// Name:        svgctrl.cpp
// Purpose:     svg control widget
// Author:      Alex Thuering
// Created:     2005/05/07
// RCS-ID:      $Id: svgctrl.cpp,v 1.19 2012/04/09 11:29:36 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "svgctrl.h"
#include <SVGSVGElement.h>
#include <wx/wx.h>

IMPLEMENT_ABSTRACT_CLASS(wxSVGCtrlBase, wxControl)

BEGIN_EVENT_TABLE(wxSVGCtrlBase, wxControl)
	EVT_PAINT(wxSVGCtrlBase::OnPaint)
	EVT_SIZE(wxSVGCtrlBase::OnResize)
	EVT_ERASE_BACKGROUND(wxSVGCtrlBase::OnEraseBackground)
END_EVENT_TABLE()

wxSVGCtrlBase::wxSVGCtrlBase() {
	Init();
}

wxSVGCtrlBase::wxSVGCtrlBase(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style,
		const wxValidator& validator, const wxString& name) {
	Init();
	Create(parent, id, pos, size, style, validator, name);
}

void wxSVGCtrlBase::Init() {
	m_doc = NULL;
	m_docDelete = false;
	m_repaint = false;
	m_fitToFrame = true;
}

wxSVGCtrlBase::~wxSVGCtrlBase() {
	Clear();
}

void wxSVGCtrlBase::Clear() {
	if (m_doc && m_docDelete)
		delete m_doc;
	m_doc = NULL;
	m_docDelete = false;
}

void wxSVGCtrlBase::SetSVG(wxSVGDocument* doc) {
	Clear();
	m_doc = doc;
}

bool wxSVGCtrlBase::Load(const wxString& filename) {
	if (!m_doc) {
		m_doc = new wxSVGDocument;
		m_docDelete = true;
	}

	if (!m_doc->Load(filename)) {
		return false;
	}
	Refresh();
	return true;
}

void wxSVGCtrlBase::Refresh(bool eraseBackground, const wxRect* rect) {
	if (rect && m_repaintRect.width > 0 && m_repaintRect.height > 0) {
		int x2 = wxMax(m_repaintRect.x+m_repaintRect.width, rect->x+rect->width);
		int y2 = wxMax(m_repaintRect.y+m_repaintRect.height, rect->y+rect->height);
		m_repaintRect.x = wxMin(m_repaintRect.x, rect->x);
		m_repaintRect.y = wxMin(m_repaintRect.y, rect->y);
		m_repaintRect.width = x2 - m_repaintRect.x;
		m_repaintRect.height = y2 - m_repaintRect.y;
	} else
		m_repaintRect = rect && !m_repaint ? *rect : wxRect();
	m_repaint = true;
	wxControl::Refresh(eraseBackground, rect);
}

void wxSVGCtrlBase::Refresh(const wxSVGRect* rect) {
	if (!rect || rect->IsEmpty())
		Refresh(true, NULL);
	else {
		wxRect winRect((int) (rect->GetX() * GetScaleX()), (int) (rect->GetY() * GetScaleY()),
				(int) (rect->GetWidth() * GetScaleX()), (int) (rect->GetHeight() * GetScaleY()));
		RefreshRect(winRect);
	}
}

void wxSVGCtrlBase::OnPaint(wxPaintEvent& event) {
	if (!m_doc)
		m_buffer = wxBitmap();
	else if (m_repaint)
		RepaintBuffer();
	wxPaintDC dc(this);

#ifdef __WXMSW__
	int w = GetClientSize().GetWidth();
	int h = GetClientSize().GetHeight();
	dc.SetPen(wxPen(wxColour(), 0, wxTRANSPARENT));
	dc.DrawRectangle(m_buffer.GetWidth(), 0, w - m_buffer.GetWidth(), h);
	dc.DrawRectangle(0, m_buffer.GetHeight(), m_buffer.GetWidth(), h - m_buffer.GetHeight());
#endif
	if (m_buffer.IsOk())
		dc.DrawBitmap(m_buffer, 0, 0);
}

void wxSVGCtrlBase::RepaintBuffer() {
	int w = -1, h = -1;
	if (m_fitToFrame)
		GetClientSize(&w, &h);

	//wxDateTime time = wxDateTime::UNow();

	if (m_repaintRect.width > 0 && m_repaintRect.height > 0
			&& (m_repaintRect.width < 2 * m_buffer.GetWidth() / 3
					|| m_repaintRect.height < 2 * m_buffer.GetHeight() / 3)) {
		m_repaintRect.x = wxMax(m_repaintRect.x, 0);
		m_repaintRect.y = wxMax(m_repaintRect.y, 0);
		wxSVGRect rect(m_repaintRect.x / GetScaleX(), m_repaintRect.y / GetScaleY(),
				m_repaintRect.width / GetScaleX(), m_repaintRect.height / GetScaleY());
		wxBitmap bitmap = m_doc->Render(w, h, &rect);
		wxMemoryDC dc;
		dc.SelectObject(m_buffer);
		dc.DrawBitmap(bitmap, m_repaintRect.x, m_repaintRect.y);
	} else
		m_buffer = wxBitmap(m_doc->Render(w, h));

	m_repaintRect = wxRect();

	//wxLogError(wxDateTime::UNow().Subtract(time).Format(wxT("draw buffer %l ms")));
}

double wxSVGCtrlBase::GetScale() const {
	if (m_doc)
		return m_doc->GetScale();
	return 1;
}

double wxSVGCtrlBase::GetScaleX() const {
	if (m_doc)
		return m_doc->GetScaleX();
	return 1;
}

double wxSVGCtrlBase::GetScaleY() const {
	if (m_doc)
		return m_doc->GetScaleY();
	return 1;
}

void wxSVGCtrlBase::MoveElement(wxSVGElement* elem, double Xposition, double Yposition) {
	if (elem->GetDtd() == wxSVG_RECT_ELEMENT) {
		double stroke_width = 0;
  		if (((wxSVGRectElement*)elem)->GetStroke().GetPaintType() != wxSVG_PAINTTYPE_NONE)
  			stroke_width = ((wxSVGRectElement*)elem)->GetStrokeWidth();
		wxSVGMatrix CTM = ((wxSVGRectElement*)elem)->GetCTM();
		double denom = CTM.GetB()*CTM.GetC() - CTM.GetA()*CTM.GetD();
		double x = (CTM.GetC()*(Yposition-CTM.GetF()) - CTM.GetD()*(Xposition-CTM.GetE())) / denom;
		double y = (CTM.GetB()*(Xposition-CTM.GetE()) - CTM.GetA()*(Yposition-CTM.GetF())) / denom;
		wxSVGLength Xvalue(x + stroke_width / 2);
		wxSVGLength Yvalue(y + stroke_width / 2);
		((wxSVGRectElement*)elem)->SetX(Xvalue);
		((wxSVGRectElement*)elem)->SetY(Yvalue);
	} else {
		wxSVGTransformable* element = wxSVGTransformable::GetSVGTransformable(*elem);
        wxSVGMatrix CTM = element->GetCTM();
		wxSVGTransformList transforms = element->GetTransform().GetBaseVal();
		wxSVGMatrix matrix = transforms[(int)transforms.Count()-1].GetMatrix();
		wxSVGRect bbox = element->GetResultBBox(wxSVG_COORDINATES_VIEWPORT);
		wxSVGPoint LeftUp = wxSVGPoint(bbox.GetX(), bbox.GetY());
		wxSVGMatrix new_matrix = wxSVGMatrix();
		new_matrix = new_matrix.Translate(Xposition - LeftUp.GetX(), Yposition - LeftUp.GetY());
		new_matrix = matrix.Multiply(CTM.Inverse().Multiply(new_matrix.Multiply(CTM)));
		transforms[transforms.Count()-1].SetMatrix(new_matrix);
		element->SetTransform(transforms);
	}
}

IMPLEMENT_ABSTRACT_CLASS(wxSVGCtrl, wxSVGCtrlBase)
wxSVGCtrl::wxSVGCtrl() {
	// nothing to do
}

wxSVGCtrl::wxSVGCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style,
		const wxString& name) {
 	Create( parent,  id,  pos, size, style, wxDefaultValidator, name);
}

wxSVGMatrix wxSVGCtrlBase::GetScreenCTM() const {
	if (m_doc && m_doc->GetRootElement())
		return m_doc->GetRootElement()->GetScreenCTM();
	return wxSVGMatrix();
}
