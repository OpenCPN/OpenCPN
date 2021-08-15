//////////////////////////////////////////////////////////////////////////////
// Name:        svgctrl.h
// Purpose:     svg control widget
// Author:      Alex Thuering
// Created:     2005/05/07
// RCS-ID:      $Id: svgctrl.h,v 1.12 2010/07/22 20:20:42 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#ifndef wxSVG_CTRL_H
#define wxSVG_CTRL_H

#include "SVGDocument.h"
#include "SVGRectElement.h"
#include "SVGTransformable.h"
#include <wx/control.h>
#include <wx/bitmap.h>

class wxSVGCtrlBase: public wxControl {
public:
    wxSVGCtrlBase();
    wxSVGCtrlBase(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
    		const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator,
    		const wxString& name = wxPanelNameStr);
    virtual ~wxSVGCtrlBase();

    void SetFitToFrame(bool fit = true) { m_fitToFrame = fit; }
    double GetScale() const;
    double GetScaleX() const;
    double GetScaleY() const;
    wxSVGMatrix GetScreenCTM() const;

    void SetSVG(wxSVGDocument* doc);
    wxSVGDocument* GetSVG() { return m_doc; }
    /** clears SVGCtrl (deletes svg document) */
    void Clear();

    /** loads svg file */
    bool Load(const wxString& filename);
    /** renders svg and repaints window */
    void Refresh(bool eraseBackground = true, const wxRect* rect = NULL);
    void Refresh(const wxSVGRect* rect);
    /** Redraws the contents of the given rectangle:
        only the area inside it will be repainted. */
    void RefreshRect(const wxRect& rect) { Refresh(true, &rect); }
    void RefreshRect(const wxSVGRect& rect) { Refresh(&rect); }

    void MoveElement(wxSVGElement* elem, double Xposition, double Yposition);

protected:
    wxSVGDocument* m_doc;
    bool m_docDelete;
    bool m_repaint;
    wxRect m_repaintRect;
    wxBitmap m_buffer;
    bool m_fitToFrame;

    virtual void Init();
    virtual void OnPaint(wxPaintEvent& event);
    virtual void OnResize(wxSizeEvent& event) { Refresh(); }
    virtual void OnEraseBackground(wxEraseEvent &event) {}
    virtual void RepaintBuffer();

private:
    DECLARE_ABSTRACT_CLASS(wxSVGCtrlBase)
    DECLARE_EVENT_TABLE()
};

class wxSVGCtrl: public wxSVGCtrlBase {
public:
    wxSVGCtrl();
    wxSVGCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
    		const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);
private:
    DECLARE_DYNAMIC_CLASS(wxSVGCtrl)
};


#endif // wxSVG_CTRL_H
