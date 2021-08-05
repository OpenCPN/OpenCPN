/////////////////////////////////////////////////////////////////////////////
// Name:        SVGCanvasItem.h
// Purpose:     Canvas items
// Author:      Alex Thuering
// Created:     2005/05/09
// RCS-ID:      $Id: SVGCanvasItem.h,v 1.29 2016/07/27 08:54:21 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef WX_SVG_CANVAS_ITEM_H
#define WX_SVG_CANVAS_ITEM_H

#include "svg.h"
#include <wx/dynarray.h>
#include <vector>

using std::vector;

enum wxSVGCanvasItemType {
	wxSVG_CANVAS_ITEM_PATH,
	wxSVG_CANVAS_ITEM_TEXT,
	wxSVG_CANVAS_ITEM_IMAGE,
	wxSVG_CANVAS_ITEM_VIDEO
};


struct wxSVGMark {
	enum Type {
		START, MID, END
	};

	double x, y, angle;
	Type type;

	wxSVGMark(double aX, double aY, double aAngle, Type aType): x(aX), y(aY), angle(aAngle), type(aType) {}
};

/** Base class for canvas items */
class wxSVGCanvasItem {
  public:
	wxSVGCanvasItem(wxSVGCanvasItemType type) { m_type = type; }
	virtual ~wxSVGCanvasItem() {}
	wxSVGCanvasItemType GetType() { return m_type; }

    /** returns the bounding box of the item */
    virtual wxSVGRect GetBBox(const wxSVGMatrix* matrix = NULL) { return wxSVGRect(); }
    virtual wxSVGRect GetResultBBox(const wxCSSStyleDeclaration& style,
      const wxSVGMatrix* matrix = NULL) { return GetBBox(matrix); }

  protected:
	wxSVGCanvasItemType m_type;
};

/** Canvas item, that saves a graphic path (SVGPathElement) and
  * and other elements that can be converted to a path (SVGRectElement, etc.)
  */
class wxSVGCanvasPath: public wxSVGCanvasItem
{
  public:
	wxSVGCanvasPath();
	virtual ~wxSVGCanvasPath() {}

	void Init(wxSVGLineElement& element);
	void Init(wxSVGPolylineElement& element);
	void Init(wxSVGPolygonElement& element);
	void Init(wxSVGRectElement& element);
	void Init(wxSVGCircleElement& element);
	void Init(wxSVGEllipseElement& element);
	void Init(wxSVGPathElement& element);

	void MoveTo(double x, double y, bool relative = false);
	void LineTo(double x, double y, bool relative = false);
	void LineToHorizontal(double x, bool relative = false);
	void LineToVertical(double y, bool relative = false);
	void CurveToCubic(double x1, double y1, double x2, double y2, double x, double y, bool relative = false);
	void CurveToCubicSmooth(double x2, double y2, double x, double y, bool relative = false);
	void CurveToQuadratic(double x1, double y1, double x, double y, bool relative = false);
	void CurveToQuadraticSmooth(double x, double y, bool relative = false);
	void Arc(double x, double y, double r1, double r2, double angle,
	  bool largeArcFlag, bool sweepFlag, bool relative = false);
	bool ClosePath();

	virtual void End() = 0;

	inline void SetFill(bool fill = true) { m_fill = fill; }
	inline bool GetFill() { return m_fill; }

    /** returns the marker points */
    virtual vector<wxSVGMark> GetMarkPoints();

  protected:
    wxSVGElement* m_element;
	bool m_fill; /* define, if a path can be filled (disabled for line) */
	double m_curx, m_cury, m_cubicx, m_cubicy, m_quadx, m_quady, m_begx, m_begy;
	virtual void MoveToImpl(double x, double y) = 0;
	virtual void LineToImpl(double x, double y) = 0;
	virtual void CurveToCubicImpl(double x1, double y1, double x2, double y2, double x, double y) = 0;
	virtual bool ClosePathImpl() = 0;
};

/** character */
struct wxSVGCanvasTextChar {
	wxSVGCanvasPath* path;
	wxSVGRect bbox;
};
WX_DECLARE_OBJARRAY(wxSVGCanvasTextChar, wxSVGCanvasTextCharList);

/** text-chunk */
struct wxSVGCanvasTextChunk {
  double x;
  double y;
  wxString text;
  wxSVGCanvasTextCharList chars;
  wxCSSStyleDeclaration style;
  wxSVGMatrix matrix;
  wxSVGRect GetBBox(const wxSVGMatrix* matrix);
  wxSVGRect GetBBox() { return GetBBox(NULL); }
};

WX_DECLARE_OBJARRAY(wxSVGCanvasTextChunk, wxSVGCanvasTextChunkList);

/** Canvas item, that saves text (SVGTextElement) as list of chunks */
class wxSVGCanvasText: public wxSVGCanvasItem
{
  public:
	wxSVGCanvasText(wxSVGCanvas* canvas);
	virtual ~wxSVGCanvasText();

	virtual void Init(wxSVGTextElement& element, const wxCSSStyleDeclaration& style, wxSVGMatrix* matrix);
    virtual wxSVGRect GetBBox(const wxSVGMatrix* matrix = NULL);
	virtual long GetNumberOfChars();
    virtual double GetComputedTextLength();
    virtual double GetSubStringLength(unsigned long charnum, unsigned long nchars);
    virtual wxSVGPoint GetStartPositionOfChar(unsigned long charnum);
    virtual wxSVGPoint GetEndPositionOfChar(unsigned long charnum);
    virtual wxSVGRect GetExtentOfChar(unsigned long charnum);
    virtual double GetRotationOfChar(unsigned long charnum);
    virtual long GetCharNumAtPosition(const wxSVGPoint& point);

  public:
    wxSVGCanvasTextChunkList m_chunks; /** list of text-chunks */
	wxSVGCanvasTextChar* m_char; /** current char */

  protected:
    wxSVGCanvas* m_canvas;
    double m_tx, m_ty; /** current text position */
    wxCSS_VALUE m_textAnchor; /** current text anchor */
	int m_textAnchorBeginIndex; /** index of first chunk with current text anchor */
	double m_textAnchorBeginPos; /** x-coordinate of text with current text anchor */
	wxCSS_VALUE m_dominantBaseline; /** current dominant baseline */
    int m_dominantBaselineBeginIndex; /** index of first chunk with current baseline */
	virtual void Init(wxSVGTSpanElement& element, const wxCSSStyleDeclaration& style, wxSVGMatrix* matrix);
	virtual void InitChildren(wxSVGTextPositioningElement& element, const wxCSSStyleDeclaration& style,
			wxSVGMatrix* matrix);
	virtual void AddChunk(const wxString& text, const wxCSSStyleDeclaration& style, wxSVGMatrix* matrix);
	virtual void BeginChar(wxSVGMatrix* matrix);
	virtual void EndChar();
	virtual void EndTextAnchor();
	wxSVGCanvasTextChunk* GetChunk(unsigned long& charnum);
    /** Converts text in path and saves in current chunk (m_chunk->path) */
    virtual void InitText(const wxString& text, const wxCSSStyleDeclaration& style, wxSVGMatrix* matrix) = 0;
};

class wxSVGCanvasSvgImageData {
public:
	wxSVGCanvasSvgImageData(const wxString& filename, wxSVGDocument* doc);
	wxSVGCanvasSvgImageData(wxSVGSVGElement* svgImage, wxSVGDocument* doc);
	~wxSVGCanvasSvgImageData();

	void IncRef() { m_count++; }
	int DecRef() { return (--m_count); }

	inline wxSVGSVGElement* GetSvgImage() { return m_svgImage; }

private:
    int m_count;
    wxSVGSVGElement* m_svgImage;
};

/** Canvas item, that saves image (SVGImageElement) */
class wxSVGCanvasImage: public wxSVGCanvasItem {
public:
	wxSVGCanvasImage(): wxSVGCanvasItem(wxSVG_CANVAS_ITEM_IMAGE), m_x(0), m_y(0), m_width(0), m_height(0),
		m_defHeightScale(1), m_svgImageData(NULL) {}
	wxSVGCanvasImage(wxSVGCanvasItemType type): wxSVGCanvasItem(type), m_x(0), m_y(0), m_width(0), m_height(0),
		m_defHeightScale(1), m_svgImageData(NULL) {}
	virtual ~wxSVGCanvasImage();
	virtual void Init(wxSVGImageElement& element, const wxCSSStyleDeclaration& style, wxProgressDialog* progressDlg);
	virtual int GetDefaultWidth();
	virtual int GetDefaultHeight();
	const wxSVGPreserveAspectRatio& GetPreserveAspectRatio() { return m_preserveAspectRatio; }
	wxSVGSVGElement* GetSvgImage(wxSVGDocument* doc = NULL);

public:
	double m_x, m_y, m_width, m_height; /** position and size of image */
    wxString m_href; /** link to the image (filename) */
	wxImage m_image; /** image data */
	double m_defHeightScale;
	wxSVGPreserveAspectRatio m_preserveAspectRatio;
	wxSVGCanvasSvgImageData* m_svgImageData;
};

class wxFfmpegMediaDecoder;

/** CanvasVideoData */
class wxSVGCanvasVideoData {
public:
	wxSVGCanvasVideoData(wxFfmpegMediaDecoder* mediaDecoder);
	~wxSVGCanvasVideoData();

	void IncRef() { m_count++; }
	int DecRef() { return (--m_count); }

	wxFfmpegMediaDecoder* GetMediaDecoder() { return m_mediaDecoder; }
	wxImage GetImage(double time);

private:
	int m_count;
	wxFfmpegMediaDecoder* m_mediaDecoder;
	wxImage m_image;
};

/** Canvas item, that saves video (wxSVGVideoElement) */
class wxSVGCanvasVideo: public wxSVGCanvasImage {
public:
	wxSVGCanvasVideo();
	virtual ~wxSVGCanvasVideo();
	virtual void Init(wxSVGVideoElement& element, const wxCSSStyleDeclaration& style, wxProgressDialog* progressDlg);
	double GetDuration() { return m_duration; }

public:
	double m_time; /** time of the loaded frame */
	double m_duration;
	wxSVGCanvasVideoData* m_videoData;
};

#endif // WX_SVG_CANVAS_ITEM_H
