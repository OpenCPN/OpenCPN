//////////////////////////////////////////////////////////////////////////////
// Name:        SVGCanvasItem.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/05/09
// RCS-ID:      $Id: SVGCanvasItem.cpp,v 1.52 2016/01/09 23:31:14 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include <wx/wx.h>
#include <wx/tokenzr.h>
#include "SVGCanvasItem.h"
#include "SVGCanvas.h"
#include <math.h>
#include <wx/log.h>
#include <wx/progdlg.h>
#include <wx/filename.h>

#ifdef USE_LIBAV
#include <wxSVG/mediadec_ffmpeg.h>
#endif

#undef GetCurrentTime

//////////////////////////////////////////////////////////////////////////////
/////////////////////////////// wxSVGCanvasPath //////////////////////////////
//////////////////////////////////////////////////////////////////////////////

wxSVGCanvasPath::wxSVGCanvasPath() : wxSVGCanvasItem(wxSVG_CANVAS_ITEM_PATH) {
	m_element = NULL;
	m_fill = true;
	m_curx = m_cury = m_cubicx = m_cubicy = m_quadx = m_quady = 0;
	m_begx = m_begy = 0;
}

void wxSVGCanvasPath::Init(wxSVGLineElement& element) {
	m_element = &element;
	SetFill(false);

	MoveTo(element.GetX1().GetAnimVal(), element.GetY1().GetAnimVal());
	LineTo(element.GetX2().GetAnimVal(), element.GetY2().GetAnimVal());
	End();
}

void wxSVGCanvasPath::Init(wxSVGPolylineElement& element) {
	m_element = &element;
	SetFill(false);

	const wxSVGPointList& points = element.GetPoints();
	if (points.Count())
		MoveTo(points[0].GetX(), points[0].GetY());
	for (unsigned int i = 1; i < points.Count(); i++)
		LineTo(points[i].GetX(), points[i].GetY());
	End();
}

void wxSVGCanvasPath::Init(wxSVGPolygonElement& element) {
	m_element = &element;
	const wxSVGPointList& points = element.GetPoints();
	if (points.Count())
		MoveTo(points[0].GetX(), points[0].GetY());
	for (unsigned int i = 1; i < points.Count(); i++)
		LineTo(points[i].GetX(), points[i].GetY());
	ClosePath();
	End();
}

void wxSVGCanvasPath::Init(wxSVGRectElement& element) {
	m_element = &element;
	double x = element.GetX().GetAnimVal();
	double y = element.GetY().GetAnimVal();
	double width = element.GetWidth().GetAnimVal();
	double height = element.GetHeight().GetAnimVal();
	double rx = element.GetRx().GetAnimVal();
	double ry = element.GetRy().GetAnimVal();

	if (rx == 0 && ry == 0) {
		MoveTo(x, y);
		LineTo(width, 0, true);
		LineTo(0, height, true);
		LineTo(-width, 0, true);
		ClosePath();
	} else {
		if (rx == 0)
			rx = ry;
		if (ry == 0)
			ry = rx;
		if (rx > width / 2)
			rx = width / 2;
		if (ry > height / 2)
			ry = height / 2;
		MoveTo(x + rx, y);
		CurveToCubic(x + rx * 0.448, y, x, y + ry * 0.448, x, y + ry);
		if (ry < height / 2)
			LineTo(x, y + height - ry);
		CurveToCubic(x, y + height - ry * 0.448, x + rx * 0.448, y + height,
				x + rx, y + height);
		if (rx < width / 2)
			LineTo(x + width - rx, y + height);
		CurveToCubic(x + width - rx * 0.448, y + height, x + width,
				y + height - ry * 0.448, x + width, y + height - ry);
		if (ry < height / 2)
			LineTo(x + width, y + ry);
		CurveToCubic(x + width, y + ry * 0.448, x + width - rx * 0.448, y,
				x + width - rx, y);
		if (rx < width / 2)
			LineTo(x + rx, y);
		ClosePath();
	}
	End();
}

void wxSVGCanvasPath::Init(wxSVGCircleElement& element) {
	m_element = &element;
	double cx = element.GetCx().GetAnimVal();
	double cy = element.GetCy().GetAnimVal();
	double r = element.GetR().GetAnimVal();
	double len = 0.55228474983079356;
	double cos4[] = { 1.0, 0.0, -1.0, 0.0, 1.0 };
	double sin4[] = { 0.0, 1.0, 0.0, -1.0, 0.0 };

	MoveTo(cx + r, cy);

	for (int i = 1; i < 5; i++) {
		CurveToCubic(cx + (cos4[i - 1] + len * cos4[i]) * r,
				cy + (sin4[i - 1] + len * sin4[i]) * r,
				cx + (cos4[i] + len * cos4[i - 1]) * r,
				cy + (sin4[i] + len * sin4[i - 1]) * r, cx + (cos4[i]) * r,
				cy + (sin4[i]) * r);
	}
	End();
}

void wxSVGCanvasPath::Init(wxSVGEllipseElement& element) {
	m_element = &element;
	double cx = element.GetCx().GetAnimVal();
	double cy = element.GetCy().GetAnimVal();
	double rx = element.GetRx().GetAnimVal();
	double ry = element.GetRy().GetAnimVal();
	double len = 0.55228474983079356;
	double cos4[] = { 1.0, 0.0, -1.0, 0.0, 1.0 };
	double sin4[] = { 0.0, 1.0, 0.0, -1.0, 0.0 };

	MoveTo(cx + rx, cy);

	for (int i = 1; i < 5; i++) {
		CurveToCubic(cx + (cos4[i - 1] + len * cos4[i]) * rx,
				cy + (sin4[i - 1] + len * sin4[i]) * ry,
				cx + (cos4[i] + len * cos4[i - 1]) * rx,
				cy + (sin4[i] + len * sin4[i - 1]) * ry, cx + (cos4[i]) * rx,
				cy + (sin4[i]) * ry);
	}
	End();
}

void wxSVGCanvasPath::Init(wxSVGPathElement& element) {
	m_element = &element;
	const wxSVGPathSegList& segList = element.GetPathSegList();
	for (int i = 0; i < (int) segList.Count(); i++) {
		switch (segList[i].GetPathSegType()) {
		case wxPATHSEG_MOVETO_ABS: {
			wxSVGPathSegMovetoAbs& seg = (wxSVGPathSegMovetoAbs&) (segList[i]);
			MoveTo(seg.GetX(), seg.GetY());
			break;
		}
		case wxPATHSEG_MOVETO_REL: {
			wxSVGPathSegMovetoRel& seg = (wxSVGPathSegMovetoRel&) (segList[i]);
			MoveTo(seg.GetX(), seg.GetY(), true);
			break;
		}
		case wxPATHSEG_LINETO_ABS: {
			wxSVGPathSegLinetoAbs& seg = (wxSVGPathSegLinetoAbs&) segList[i];
			LineTo(seg.GetX(), seg.GetY());
			break;
		}
		case wxPATHSEG_LINETO_REL: {
			wxSVGPathSegLinetoRel& seg = (wxSVGPathSegLinetoRel&) segList[i];
			LineTo(seg.GetX(), seg.GetY(), true);
			break;
		}
		case wxPATHSEG_LINETO_HORIZONTAL_ABS: {
			wxSVGPathSegLinetoHorizontalAbs& seg =
					(wxSVGPathSegLinetoHorizontalAbs&) segList[i];
			LineToHorizontal(seg.GetX());
			break;
		}
		case wxPATHSEG_LINETO_HORIZONTAL_REL: {
			wxSVGPathSegLinetoHorizontalRel& seg =
					(wxSVGPathSegLinetoHorizontalRel&) segList[i];
			LineToHorizontal(seg.GetX(), true);
			break;
		}
		case wxPATHSEG_LINETO_VERTICAL_ABS: {
			wxSVGPathSegLinetoVerticalAbs& seg =
					(wxSVGPathSegLinetoVerticalAbs&) segList[i];
			LineToVertical(seg.GetY());
			break;
		}
		case wxPATHSEG_LINETO_VERTICAL_REL: {
			wxSVGPathSegLinetoVerticalRel& seg =
					(wxSVGPathSegLinetoVerticalRel&) segList[i];
			LineToVertical(seg.GetY(), true);
			break;
		}
		case wxPATHSEG_CURVETO_CUBIC_ABS: {
			wxSVGPathSegCurvetoCubicAbs& seg =
					(wxSVGPathSegCurvetoCubicAbs&) segList[i];
			CurveToCubic(seg.GetX1(), seg.GetY1(), seg.GetX2(), seg.GetY2(),
					seg.GetX(), seg.GetY());
			break;
		}
		case wxPATHSEG_CURVETO_CUBIC_REL: {
			wxSVGPathSegCurvetoCubicRel& seg =
					(wxSVGPathSegCurvetoCubicRel&) segList[i];
			CurveToCubic(seg.GetX1(), seg.GetY1(), seg.GetX2(), seg.GetY2(),
					seg.GetX(), seg.GetY(), true);
			break;
		}
		case wxPATHSEG_CURVETO_CUBIC_SMOOTH_ABS: {
			wxSVGPathSegCurvetoCubicSmoothAbs& seg =
					(wxSVGPathSegCurvetoCubicSmoothAbs&) segList[i];
			CurveToCubicSmooth(seg.GetX2(), seg.GetY2(), seg.GetX(),
					seg.GetY());
			break;
		}
		case wxPATHSEG_CURVETO_CUBIC_SMOOTH_REL: {
			wxSVGPathSegCurvetoCubicSmoothRel& seg =
					(wxSVGPathSegCurvetoCubicSmoothRel&) segList[i];
			CurveToCubicSmooth(seg.GetX2(), seg.GetY2(), seg.GetX(), seg.GetY(),
					true);
			break;
		}
		case wxPATHSEG_CURVETO_QUADRATIC_ABS: {
			wxSVGPathSegCurvetoQuadraticAbs& seg =
					(wxSVGPathSegCurvetoQuadraticAbs&) segList[i];
			CurveToQuadratic(seg.GetX1(), seg.GetY1(), seg.GetX(), seg.GetY());
			break;
		}
		case wxPATHSEG_CURVETO_QUADRATIC_REL: {
			wxSVGPathSegCurvetoQuadraticRel& seg =
					(wxSVGPathSegCurvetoQuadraticRel&) segList[i];
			CurveToQuadratic(seg.GetX1(), seg.GetY1(), seg.GetX(), seg.GetY(),
					true);
			break;
		}
		case wxPATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS: {
			wxSVGPathSegCurvetoQuadraticSmoothAbs& seg =
					(wxSVGPathSegCurvetoQuadraticSmoothAbs&) segList[i];
			CurveToQuadraticSmooth(seg.GetX(), seg.GetY());
			break;
		}
		case wxPATHSEG_CURVETO_QUADRATIC_SMOOTH_REL: {
			wxSVGPathSegCurvetoQuadraticSmoothRel& seg =
					(wxSVGPathSegCurvetoQuadraticSmoothRel&) segList[i];
			CurveToQuadraticSmooth(seg.GetX(), seg.GetY(), true);
			break;
		}
		case wxPATHSEG_ARC_ABS: {
			wxSVGPathSegArcAbs& seg = (wxSVGPathSegArcAbs&) segList[i];
			Arc(seg.GetX(), seg.GetY(), seg.GetR1(), seg.GetR2(),
					seg.GetAngle(), seg.GetLargeArcFlag(), seg.GetSweepFlag());
			break;
		}
		case wxPATHSEG_ARC_REL: {
			wxSVGPathSegArcRel& seg = (wxSVGPathSegArcRel&) segList[i];
			Arc(seg.GetX(), seg.GetY(), seg.GetR1(), seg.GetR2(),
					seg.GetAngle(), seg.GetLargeArcFlag(), seg.GetSweepFlag(),
					true);
			break;
		}
		case wxPATHSEG_CLOSEPATH:
			ClosePath();
			break;
		case wxPATHSEG_UNKNOWN:
			break;
		}
	}
	End();
}

//////////////////////////////////////////////////////////////////////////////
///////////////////////// Path functions /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void wxSVGCanvasPath::MoveTo(double x, double y, bool relative)
{
  if (relative)
  {
	x += m_curx;
	y += m_cury;
  }
  MoveToImpl(x, y);
  m_begx = m_curx = x;
  m_begy = m_cury = y;
}

void wxSVGCanvasPath::LineTo(double x, double y, bool relative)
{
  if (relative)
  {
	x += m_curx;
	y += m_cury;
  }
  LineToImpl(x, y);
  m_curx = x;
  m_cury = y;
}

void wxSVGCanvasPath::LineToHorizontal(double x, bool relative)
{
  if (relative)
	x += m_curx;
  LineToImpl(x, m_cury);
  m_curx = x;
}

void wxSVGCanvasPath::LineToVertical(double y, bool relative)
{
  if (relative)
	y += m_cury;
  LineToImpl(m_curx, y);
  m_cury = y;
}

void wxSVGCanvasPath::CurveToCubic(double x1, double y1, double x2, double y2, double x, double y, bool relative)
{
  if (relative)
  {
	x1 += m_curx;
	y1 += m_cury;
	x2 += m_curx;
	y2 += m_cury;
	x += m_curx;
	y += m_cury;
  }
  CurveToCubicImpl(x1, y1, x2, y2, x, y);
  m_curx = x;
  m_cury = y;
  m_cubicx = 2*m_curx - x2;
  m_cubicy = 2*m_cury - y2;
}

void wxSVGCanvasPath::CurveToCubicSmooth(double x2, double y2, double x, double y, bool relative)
{
  if (relative)
  {
	x2 += m_curx;
	y2 += m_cury;
	x += m_curx;
	y += m_cury;
  }
  CurveToCubicImpl(m_cubicx, m_cubicy, x2, y2, x, y);
  m_curx = x;
  m_cury = y;
  m_cubicx = 2*m_curx - x2;
  m_cubicy = 2*m_cury - y2;
}

void wxSVGCanvasPath::CurveToQuadratic(double x1, double y1, double x, double y, bool relative)
{
  if (relative)
  {
	x1 += m_curx;
	y1 += m_cury;
	x += m_curx;
	y += m_cury;
  }
  m_quadx = 2*x - x1;
  m_quady = 2*y - y1;
  double x2 = (x + 2*x1)/3;
  double y2 = (y + 2*y1)/3;
  x1 = (m_curx + 2*x1)/3;
  y1 = (m_cury + 2*y1)/3;
  CurveToCubicImpl(x1, y1, x2, y2, x, y);
  m_curx = x;
  m_cury = y;
}

void wxSVGCanvasPath::CurveToQuadraticSmooth(double x, double y, bool relative)
{
  if (relative)
  {
	x += m_curx;
	y += m_cury;
  }
  double x1 = (m_curx + 2*m_quadx)/3;
  double y1 = (m_cury + 2*m_quady)/3;
  double x2 = (x + 2*m_quadx)/3;
  double y2 = (y + 2*m_quady)/3;
  CurveToCubicImpl(x1, y1, x2, y2, x, y);
  m_curx = x;
  m_cury = y;
  m_quadx = 2*x - m_quadx;
  m_quady = 2*y - m_quady;
}

// This works by converting the SVG arc to "simple" beziers.
// For each bezier found a svgToCurve call is done.
// Adapted from Niko's code in kdelibs/kdecore/svgicons.
void wxSVGCanvasPath::Arc(double x, double y, double r1, double r2,
  double angle, bool largeArcFlag, bool sweepFlag,
  bool relative)
{
  if (relative)
  {
	x += m_curx;
	y += m_cury;
  }
  
  double sin_th = sin(angle*(M_PI/180.0));
  double cos_th = cos(angle*(M_PI/180.0));

  double dx = (m_curx - x)/2.0;
  double dy = (m_cury - y)/2.0;
	  
  double _x1 =  cos_th*dx + sin_th*dy;
  double _y1 = -sin_th*dx + cos_th*dy;
  double Pr1 = r1*r1;
  double Pr2 = r2*r2;
  double Px = _x1*_x1;
  double Py = _y1*_y1;

  // Spec : check if radii are large enough
  double check = Px/Pr1 + Py/Pr2;
  if (check > 1)
  {
	r1 = r1*sqrt(check);
	r2 = r2*sqrt(check);
  }

  double a00 = cos_th/r1;
  double a01 = sin_th/r1;
  double a10 = -sin_th/r2;
  double a11 = cos_th/r2;

  double x0 = a00*m_curx + a01*m_cury;
  double y0 = a10*m_curx + a11*m_cury;

  double x1 = a00*x + a01*y;
  double y1 = a10*x + a11*y;

  /* (x0, y0) is current point in transformed coordinate space.
	 (x1, y1) is new point in transformed coordinate space.
	 The arc fits a unit-radius circle in this space. */

  double d = (x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0);

  double sfactor_sq = 1.0/d - 0.25;

  if (sfactor_sq < 0)
	sfactor_sq = 0;

  double sfactor = sqrt(sfactor_sq);

  if (sweepFlag == largeArcFlag)
	sfactor = -sfactor;

  double xc = 0.5*(x0 + x1) - sfactor*(y1 - y0);
  double yc = 0.5*(y0 + y1) + sfactor*(x1 - x0);

  /* (xc, yc) is center of the circle. */
  double th0 = atan2(y0 - yc, x0 - xc);
  double th1 = atan2(y1 - yc, x1 - xc);

  double th_arc = th1 - th0;
  if (th_arc < 0 && sweepFlag)
	th_arc += 2*M_PI;
  else if (th_arc > 0 && !sweepFlag)
	th_arc -= 2*M_PI;

  int n_segs = (int) (int) ceil(fabs(th_arc/(M_PI*0.5 + 0.001)));

  for (int i = 0; i < n_segs; i++)
  {
	double sin_th, cos_th;
	double a00, a01, a10, a11;
	double x1, y1, x2, y2, x3, y3;
	double t;
	double th_half;

	double _th0 = th0 + i*th_arc/n_segs;
	double _th1 = th0 + (i + 1)*th_arc/n_segs;

	sin_th = sin(angle*(M_PI/180.0));
	cos_th = cos(angle*(M_PI/180.0));

	/* inverse transform compared with rsvg_path_arc */
	a00 = cos_th*r1;
	a01 = -sin_th*r2;
	a10 = sin_th*r1;
	a11 = cos_th*r2;

	th_half = 0.5*(_th1 - _th0);
	t = (8.0/3.0)*sin(th_half*0.5)*sin(th_half*0.5)/sin(th_half);
	x1 = xc + cos(_th0) - t*sin(_th0);
	y1 = yc + sin(_th0) + t*cos(_th0);
	x3 = xc + cos(_th1);
	y3 = yc + sin(_th1);
	x2 = x3 + t*sin(_th1);
	y2 = y3 - t*cos(_th1);

	CurveToCubicImpl(a00*x1 + a01*y1, a10*x1 + a11*y1, a00*x2 + a01*y2,
	  a10*x2 + a11*y2, a00*x3 + a01*y3, a10*x3 + a11*y3);
  }
  m_curx = x;
  m_cury = y;
}

bool wxSVGCanvasPath::ClosePath()
{
  bool isClosed = ClosePathImpl();
  m_curx = m_begx;
  m_cury = m_begy;
  return isClosed;
}

double AngleOfVector(const wxSVGPoint& vec) {
	return vec != wxSVGPoint(0.0, 0.0) ? atan2(vec.GetY(), vec.GetX()) : 0.0;
}

bool IsMoveto(wxPATHSEG segType) {
	return segType == wxPATHSEG_MOVETO_ABS || segType == wxPATHSEG_MOVETO_REL;
}

bool IsCubicType(wxPATHSEG segType) {
	return segType == wxPATHSEG_CURVETO_CUBIC_REL
			|| segType == wxPATHSEG_CURVETO_CUBIC_ABS
			|| segType == wxPATHSEG_CURVETO_CUBIC_SMOOTH_REL
			|| segType == wxPATHSEG_CURVETO_CUBIC_SMOOTH_ABS;
}

bool IsQuadraticType(wxPATHSEG segType) {
	return segType == wxPATHSEG_CURVETO_QUADRATIC_REL
			|| segType == wxPATHSEG_CURVETO_QUADRATIC_ABS
			|| segType == wxPATHSEG_CURVETO_QUADRATIC_SMOOTH_REL
			|| segType == wxPATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS;
}

double AngleBisect(float a1, float a2) {
	double delta = fmod((double)(a2 - a1), 2 * M_PI);
	if (delta < 0) {
		delta += 2 * M_PI;
	}
	/* delta is now the angle from a1 around to a2, in the range [0, 2*M_PI) */
	float r = a1 + delta / 2;
	if (delta >= M_PI) {
		/* the arc from a2 to a1 is smaller, so use the ray on that side */
		r += M_PI;
	}
	return r;
}

void GetPolylineMarkPoints(const wxSVGPointList &points, vector<wxSVGMark>& marks) {
	if (!points.size())
		return;
	
	float px = points[0].GetX(), py = points[0].GetY(), prevAngle = 0.0;
	
	marks.push_back(wxSVGMark(px, py, 0, wxSVGMark::START));
	
	for (unsigned int i = 1; i < points.size(); ++i) {
		float x = points[i].GetX();
		float y = points[i].GetY();
		float angle = atan2(y-py, x-px);
		
		// Vertex marker.
		if (i == 1) {
			marks.begin()->angle = angle;
		} else {
			(marks.begin() + (marks.size() - 2))->angle = AngleBisect(prevAngle, angle);
		}
		
		marks.push_back(wxSVGMark(x, y, 0, wxSVGMark::MID));
		
		prevAngle = angle;
		px = x;
		py = y;
	}
	
	marks.back().angle = prevAngle;
	marks.back().type = wxSVGMark::END;
}

void GetPathMarkPoints(const wxSVGPathSegList& segments, vector<wxSVGMark>& marks) {
	// This code should assume that ANY type of segment can appear at ANY index.
	// It should also assume that segments such as M and Z can appear in weird
	// places, and repeat multiple times consecutively.

	// info on current [sub]path (reset every M command):
	wxSVGPoint pathStart(0.0, 0.0);
	double pathStartAngle = 0.0f;

	// info on previous segment:
	wxPATHSEG prevSegType = wxPATHSEG_UNKNOWN;
	wxSVGPoint prevSegEnd(0.0, 0.0);
	double prevSegEndAngle = 0.0f;
	wxSVGPoint prevCP; // if prev seg was a bezier, this was its last control point

	unsigned int i = 0;
	while (i < segments.size()) {
		// info on current segment:
		wxPATHSEG segType = segments[i].GetPathSegType();
		wxSVGPoint& segStart = prevSegEnd;
		wxSVGPoint segEnd;
		double segStartAngle = 0, segEndAngle = 0;

		switch (segType) { // to find segStartAngle, segEnd and segEndAngle
		case wxPATHSEG_CLOSEPATH:
			segEnd = pathStart;
			segStartAngle = segEndAngle = AngleOfVector(segEnd - segStart);
			break;

		case wxPATHSEG_MOVETO_ABS: {
			wxSVGPathSegMovetoAbs& seg = ((wxSVGPathSegMovetoAbs&) segments[i]);
			segEnd = wxSVGPoint(seg.GetX(), seg.GetY());
			pathStart = segEnd;
			// If authors are going to specify multiple consecutive moveto commands
			// with markers, me might as well make the angle do something useful:
			segStartAngle = segEndAngle = AngleOfVector(segEnd - segStart);
			break;
		}
		case wxPATHSEG_MOVETO_REL: {
			wxSVGPathSegMovetoRel& seg = ((wxSVGPathSegMovetoRel&) segments[i]);
			segEnd = segStart + wxSVGPoint(seg.GetX(), seg.GetY());
			pathStart = segEnd;
			segStartAngle = segEndAngle = AngleOfVector(segEnd - segStart);
			break;
		}
		case wxPATHSEG_LINETO_ABS: {
			wxSVGPathSegLinetoAbs& seg = ((wxSVGPathSegLinetoAbs&) segments[i]);
			segEnd = wxSVGPoint(seg.GetX(), seg.GetY());
			segStartAngle = segEndAngle = AngleOfVector(segEnd - segStart);
			break;
		}
		case wxPATHSEG_LINETO_REL: {
			wxSVGPathSegLinetoRel& seg = ((wxSVGPathSegLinetoRel&) segments[i]);
			segEnd = segStart + wxSVGPoint(seg.GetX(), seg.GetY());
			segStartAngle = segEndAngle = AngleOfVector(segEnd - segStart);
			break;
		}
		case wxPATHSEG_CURVETO_CUBIC_ABS: {
			wxSVGPathSegCurvetoCubicAbs& seg = ((wxSVGPathSegCurvetoCubicAbs&) segments[i]);
			wxSVGPoint cp1 = wxSVGPoint(seg.GetX1(), seg.GetY1());
			wxSVGPoint cp2 = wxSVGPoint(seg.GetX2(), seg.GetY2());
			segEnd = wxSVGPoint(seg.GetX(), seg.GetY());
			prevCP = cp2;
			if (cp1 == segStart) {
				cp1 = cp2;
			}
			if (cp2 == segEnd) {
				cp2 = cp1;
			}
			segStartAngle = AngleOfVector(cp1 - segStart);
			segEndAngle = AngleOfVector(segEnd - cp2);
			break;
		}
		case wxPATHSEG_CURVETO_CUBIC_REL: {
			wxSVGPathSegCurvetoCubicRel& seg = ((wxSVGPathSegCurvetoCubicRel&) segments[i]);
			wxSVGPoint cp1 = segStart + wxSVGPoint(seg.GetX1(), seg.GetY1());
			wxSVGPoint cp2 = segStart + wxSVGPoint(seg.GetX2(), seg.GetY2());
			segEnd = segStart + wxSVGPoint(seg.GetX(), seg.GetY());
			prevCP = cp2;
			if (cp1 == segStart) {
				cp1 = cp2;
			}
			if (cp2 == segEnd) {
				cp2 = cp1;
			}
			segStartAngle = AngleOfVector(cp1 - segStart);
			segEndAngle = AngleOfVector(segEnd - cp2);
			break;
		}

		case wxPATHSEG_CURVETO_QUADRATIC_ABS: {
			wxSVGPathSegCurvetoQuadraticAbs& seg = ((wxSVGPathSegCurvetoQuadraticAbs&) segments[i]);
			wxSVGPoint cp1 = wxSVGPoint(seg.GetX1(), seg.GetY1());
			segEnd = wxSVGPoint(seg.GetX(), seg.GetY());
			prevCP = cp1;
			segStartAngle = AngleOfVector(cp1 - segStart);
			segEndAngle = AngleOfVector(segEnd - cp1);
			break;
		}
		case wxPATHSEG_CURVETO_QUADRATIC_REL: {
			wxSVGPathSegCurvetoQuadraticRel& seg = ((wxSVGPathSegCurvetoQuadraticRel&) segments[i]);
			wxSVGPoint cp1 = segStart + wxSVGPoint(seg.GetX1(), seg.GetY1());
			wxSVGPoint segEnd = segStart + wxSVGPoint(seg.GetX(), seg.GetY());
			prevCP = cp1;
			segStartAngle = AngleOfVector(cp1 - segStart);
			segEndAngle = AngleOfVector(segEnd - cp1);
			break;
		}

		case wxPATHSEG_ARC_ABS: {
			wxSVGPathSegArcAbs& seg = ((wxSVGPathSegArcAbs&) segments[i]);
			double rx = seg.GetR1();
			double ry = seg.GetR2();
			double angle = seg.GetAngle();
			bool largeArcFlag = seg.GetLargeArcFlag();
			bool sweepFlag = seg.GetSweepFlag();
			segEnd = wxSVGPoint(seg.GetX(), seg.GetY());

			if (segStart == segEnd) {
				i++;
				continue;
			}

			if (rx == 0.0 || ry == 0.0) {
				segStartAngle = segEndAngle = AngleOfVector(segEnd - segStart);
				break;
			}
			rx = fabs(rx);
			ry = fabs(ry);

			angle = angle * M_PI / 180.0;
			double x1p = cos(angle) * (segStart.GetX() - segEnd.GetX()) / 2.0
					+ sin(angle) * (segStart.GetY() - segEnd.GetY()) / 2.0;
			double y1p = -sin(angle) * (segStart.GetX() - segEnd.GetX()) / 2.0
					+ cos(angle) * (segStart.GetY() - segEnd.GetY()) / 2.0;

			double root;
			double numerator = rx * rx * ry * ry - rx * rx * y1p * y1p - ry * ry * x1p * x1p;

			if (numerator >= 0.0) {
				root = sqrt(numerator / (rx * rx * y1p * y1p + ry * ry * x1p * x1p));
				if (largeArcFlag == sweepFlag)
					root = -root;
			} else {
				double lamedh = 1.0 - numerator / (rx * rx * ry * ry);
				double s = sqrt(lamedh);
				rx *= s;
				ry *= s;
				root = 0.0;
			}

			double cxp = root * rx * y1p / ry; 
			double cyp = -root * ry * x1p / rx;

			double theta, delta;
			theta = AngleOfVector(wxSVGPoint((x1p - cxp) / rx, (y1p - cyp) / ry));
			delta = AngleOfVector(wxSVGPoint((-x1p - cxp) / rx, (-y1p - cyp) / ry)) - theta;
			if (!sweepFlag && delta > 0)
				delta -= 2.0 * M_PI;
			else if (sweepFlag && delta < 0)
				delta += 2.0 * M_PI;

			double tx1, ty1, tx2, ty2;
			tx1 = -cos(angle) * rx * sin(theta)	- sin(angle) * ry * cos(theta);
			ty1 = -sin(angle) * rx * sin(theta)	+ cos(angle) * ry * cos(theta);
			tx2 = -cos(angle) * rx * sin(theta + delta)	- sin(angle) * ry * cos(theta + delta);
			ty2 = -sin(angle) * rx * sin(theta + delta)	+ cos(angle) * ry * cos(theta + delta);

			if (delta < 0.0f) {
				tx1 = -tx1;
				ty1 = -ty1;
				tx2 = -tx2;
				ty2 = -ty2;
			}

			segStartAngle = atan2(ty1, tx1);
			segEndAngle = atan2(ty2, tx2);
			break;
		}
		case wxPATHSEG_ARC_REL: {
			wxSVGPathSegArcRel& seg = ((wxSVGPathSegArcRel&) segments[i]);
			double rx = seg.GetR1();
			double ry = seg.GetR2();
			double angle = seg.GetAngle();
			bool largeArcFlag = seg.GetLargeArcFlag();
			bool sweepFlag = seg.GetSweepFlag();
			segEnd = segStart + wxSVGPoint(seg.GetX(), seg.GetY());

			if (segStart == segEnd) {
				i++;
				continue;
			}

			if (rx == 0.0 || ry == 0.0) {
				segStartAngle = segEndAngle = AngleOfVector(segEnd - segStart);
				break;
			}
			rx = fabs(rx);
			ry = fabs(ry);

			angle = angle * M_PI / 180.0;
			double x1p = cos(angle) * (segStart.GetX() - segEnd.GetX()) / 2.0
					+ sin(angle) * (segStart.GetY() - segEnd.GetY()) / 2.0;
			double y1p = -sin(angle) * (segStart.GetX() - segEnd.GetX()) / 2.0
					+ cos(angle) * (segStart.GetY() - segEnd.GetY()) / 2.0;

			double root;
			double numerator = rx * rx * ry * ry - rx * rx * y1p * y1p - ry * ry * x1p * x1p;

			if (numerator >= 0.0) {
				root = sqrt(numerator / (rx * rx * y1p * y1p + ry * ry * x1p * x1p));
				if (largeArcFlag == sweepFlag)
					root = -root;
			} else {
				double lamedh = 1.0 - numerator / (rx * rx * ry * ry);
				double s = sqrt(lamedh);
				rx *= s;
				ry *= s;
				root = 0.0;
			}

			double cxp = root * rx * y1p / ry; 
			double cyp = -root * ry * x1p / rx;

			double theta, delta;
			theta = AngleOfVector(wxSVGPoint((x1p - cxp) / rx, (y1p - cyp) / ry));
			delta = AngleOfVector(wxSVGPoint((-x1p - cxp) / rx, (-y1p - cyp) / ry)) - theta;
			if (!sweepFlag && delta > 0)
				delta -= 2.0 * M_PI;
			else if (sweepFlag && delta < 0)
				delta += 2.0 * M_PI;

			double tx1, ty1, tx2, ty2;
			tx1 = -cos(angle) * rx * sin(theta)	- sin(angle) * ry * cos(theta);
			ty1 = -sin(angle) * rx * sin(theta)	+ cos(angle) * ry * cos(theta);
			tx2 = -cos(angle) * rx * sin(theta + delta)	- sin(angle) * ry * cos(theta + delta);
			ty2 = -sin(angle) * rx * sin(theta + delta)	+ cos(angle) * ry * cos(theta + delta);

			if (delta < 0.0f) {
				tx1 = -tx1;
				ty1 = -ty1;
				tx2 = -tx2;
				ty2 = -ty2;
			}

			segStartAngle = atan2(ty1, tx1);
			segEndAngle = atan2(ty2, tx2);
			break;
		}

		case wxPATHSEG_LINETO_HORIZONTAL_ABS: {
			wxSVGPathSegLinetoHorizontalAbs& seg = ((wxSVGPathSegLinetoHorizontalAbs&) segments[i]);
			segEnd = wxSVGPoint(seg.GetX(), segStart.GetY());
			segStartAngle = segEndAngle = AngleOfVector(segEnd - segStart);
			break;
		}
		case wxPATHSEG_LINETO_HORIZONTAL_REL: {
			wxSVGPathSegLinetoHorizontalRel& seg = ((wxSVGPathSegLinetoHorizontalRel&) segments[i]);
			segEnd = segStart + wxSVGPoint(seg.GetX(), 0.0f);
			segStartAngle = segEndAngle = AngleOfVector(segEnd - segStart);
			break;
		}

		case wxPATHSEG_LINETO_VERTICAL_ABS: {
			wxSVGPathSegLinetoVerticalAbs& seg = ((wxSVGPathSegLinetoVerticalAbs&) segments[i]);
			segEnd = wxSVGPoint(segStart.GetX(), seg.GetY());
			segStartAngle = segEndAngle = AngleOfVector(segEnd - segStart);
			break;
		}
		case wxPATHSEG_LINETO_VERTICAL_REL: {
			wxSVGPathSegLinetoVerticalRel& seg = ((wxSVGPathSegLinetoVerticalRel&) segments[i]);
			segEnd = segStart + wxSVGPoint(0.0f, seg.GetY());
			segStartAngle = segEndAngle = AngleOfVector(segEnd - segStart);
			break;
		}

		case wxPATHSEG_CURVETO_CUBIC_SMOOTH_ABS: {
			wxSVGPathSegCurvetoCubicSmoothAbs& seg = ((wxSVGPathSegCurvetoCubicSmoothAbs&) segments[i]);
			wxSVGPoint cp1 = IsCubicType(prevSegType) ? segStart * 2 - prevCP : segStart;
			wxSVGPoint cp2 = wxSVGPoint(seg.GetX2(), seg.GetY2());
			segEnd = wxSVGPoint(seg.GetX(), seg.GetY());
			prevCP = cp2;
			if (cp1 == segStart) {
				cp1 = cp2;
			}
			if (cp2 == segEnd) {
				cp2 = cp1;
			}
			segStartAngle = AngleOfVector(cp1 - segStart);
			segEndAngle = AngleOfVector(segEnd - cp2);
			break;
		}
		case wxPATHSEG_CURVETO_CUBIC_SMOOTH_REL: {
			wxSVGPathSegCurvetoCubicSmoothRel& seg = ((wxSVGPathSegCurvetoCubicSmoothRel&) segments[i]);
			wxSVGPoint cp1 = IsCubicType(prevSegType) ? segStart * 2 - prevCP : segStart;
			wxSVGPoint cp2 = segStart + wxSVGPoint(seg.GetX2(), seg.GetY2());
			segEnd = segStart + wxSVGPoint(seg.GetX(), seg.GetY());
			prevCP = cp2;
			if (cp1 == segStart) {
				cp1 = cp2;
			}
			if (cp2 == segEnd) {
				cp2 = cp1;
			}
			segStartAngle = AngleOfVector(cp1 - segStart);
			segEndAngle = AngleOfVector(segEnd - cp2);
			break;
		}

		case wxPATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS: {
			wxSVGPathSegCurvetoQuadraticAbs& seg = ((wxSVGPathSegCurvetoQuadraticAbs&) segments[i]);
			wxSVGPoint cp1 = IsQuadraticType(prevSegType) ? segStart * 2 - prevCP : segStart;
			segEnd = wxSVGPoint(seg.GetX(), seg.GetY());
			prevCP = cp1;
			segStartAngle = AngleOfVector(cp1 - segStart);
			segEndAngle = AngleOfVector(segEnd - cp1);
			break;
		}
		case wxPATHSEG_CURVETO_QUADRATIC_SMOOTH_REL: {
			wxSVGPathSegCurvetoQuadraticRel& seg = ((wxSVGPathSegCurvetoQuadraticRel&) segments[i]);
			wxSVGPoint cp1 = IsQuadraticType(prevSegType) ? segStart * 2 - prevCP : segStart;
			segEnd = segStart + wxSVGPoint(seg.GetX(), seg.GetY());
			prevCP = cp1;
			segStartAngle = AngleOfVector(cp1 - segStart);
			segEndAngle = AngleOfVector(segEnd - cp1);
			break;
		}
		
		case wxPATHSEG_UNKNOWN:
		default:
			break;
		}
		i++;

		// Set the angle of the mark at the start of this segment:
		if (marks.size()) {
			wxSVGMark &mark = marks.back();
			if (!IsMoveto(segType) && IsMoveto(prevSegType)) {
				// start of new subpath
				pathStartAngle = mark.angle = segStartAngle;
			} else if (IsMoveto(segType) && !IsMoveto(prevSegType)) {
				// end of a subpath
				if (prevSegType != wxPATHSEG_CLOSEPATH)
					mark.angle = prevSegEndAngle;
			} else {
				if (!(segType == wxPATHSEG_CLOSEPATH && prevSegType == wxPATHSEG_CLOSEPATH))
					mark.angle = AngleBisect(prevSegEndAngle, segStartAngle);
			}
		}

		// Add the mark at the end of this segment, and set its position:
		marks.push_back(wxSVGMark(segEnd.GetX(), segEnd.GetY(), 0.0f, wxSVGMark::MID));

		if (segType == wxPATHSEG_CLOSEPATH && prevSegType != wxPATHSEG_CLOSEPATH) {
			marks.back().angle = AngleBisect(segEndAngle, pathStartAngle);
		}

		prevSegType = segType;
		prevSegEnd = segEnd;
		prevSegEndAngle = segEndAngle;
	}

	if (marks.size()) {
		if (prevSegType != wxPATHSEG_CLOSEPATH) {
			marks.back().angle = prevSegEndAngle;
		}
		marks.back().type = wxSVGMark::END;
		marks.begin()->type = wxSVGMark::START;
	}
}

/** Returns the marker points.
 *  Adopted from mozilla code.
 */
vector<wxSVGMark> wxSVGCanvasPath::GetMarkPoints() {
	vector<wxSVGMark> marks;
	if (m_element == NULL)
		return marks;
	switch (m_element->GetDtd()) {
	case wxSVG_POLYLINE_ELEMENT: {
		wxSVGPolylineElement* elem = ((wxSVGPolylineElement*) m_element);
		GetPolylineMarkPoints(elem->GetPoints(), marks);
		break;
	}
	case wxSVG_POLYGON_ELEMENT: {
		wxSVGPolygonElement* elem = ((wxSVGPolygonElement*) m_element);
		GetPolylineMarkPoints(elem->GetPoints(), marks);
		if (marks.size() == 0 || marks.back().type != wxSVGMark::END) {
			break;
		}
		
		wxSVGMark& endMark = marks.back();
		wxSVGMark& startMark = *marks.begin();
		float angle = atan2(startMark.y - endMark.y, startMark.x - endMark.x);
		
		endMark.type = wxSVGMark::MID;
		endMark.angle = AngleBisect(angle, endMark.angle);
		startMark.angle = AngleBisect(angle, startMark.angle);
		// for a polygon (as opposed to a polyline) there's an implicit extra point co-located with the start point
		// that GetPolylineMarkPoints doesn't return
		marks.push_back(wxSVGMark(startMark.x, startMark.y, startMark.angle, wxSVGMark::END));
		break;
	}
	case wxSVG_LINE_ELEMENT: {
		wxSVGLineElement* elem = ((wxSVGLineElement*) m_element);
		double angle = atan2(elem->GetY2().GetAnimVal() - elem->GetY1().GetAnimVal(),
				elem->GetX2().GetAnimVal() - elem->GetX1().GetAnimVal());

		marks.push_back(wxSVGMark(elem->GetX1().GetAnimVal(), elem->GetY1().GetAnimVal(), angle, wxSVGMark::START));
		marks.push_back(wxSVGMark(elem->GetX2().GetAnimVal(), elem->GetY2().GetAnimVal(), angle, wxSVGMark::END));
		break;
	}
	case wxSVG_PATH_ELEMENT: {
		GetPathMarkPoints(((wxSVGPathElement*) m_element)->GetPathSegList(), marks);
		break;
	}
	default:
		break;
	}
	return marks;
}

//////////////////////////////////////////////////////////////////////////////
/////////////////////////////// wxSVGCanvasText //////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(wxSVGCanvasTextCharList);
WX_DEFINE_OBJARRAY(wxSVGCanvasTextChunkList);

wxSVGCanvasText::wxSVGCanvasText(wxSVGCanvas* canvas) :
	wxSVGCanvasItem(wxSVG_CANVAS_ITEM_TEXT), m_canvas(canvas) {
	m_char = NULL;
	m_tx = m_ty = 0;
	m_textAnchor = wxCSS_VALUE_START;
	m_textAnchorBeginIndex = 0;
	m_textAnchorBeginPos = 0;
	m_dominantBaseline = wxCSS_VALUE_AUTO;
	m_dominantBaselineBeginIndex = 0;
}

wxSVGCanvasText::~wxSVGCanvasText() {
	for (unsigned int i = 0; i < m_chunks.Count(); i++)
		for (unsigned int j = 0; j < m_chunks[i].chars.Count(); j++)
			delete m_chunks[i].chars[j].path;
}

void wxSVGCanvasText::Init(wxSVGTextElement& element, const wxCSSStyleDeclaration& style, wxSVGMatrix* matrix) {
	m_tx = element.GetX().GetAnimVal().Count() ? element.GetX().GetAnimVal()[0] : wxSVGLength(0);
	m_ty = element.GetY().GetAnimVal().Count() ? element.GetY().GetAnimVal()[0] : wxSVGLength(0);
	InitChildren(element, style, matrix);
	EndTextAnchor();
}

void wxSVGCanvasText::Init(wxSVGTSpanElement& element, const wxCSSStyleDeclaration& style, wxSVGMatrix* matrix) {
	if (element.GetX().GetAnimVal().Count())
		EndTextAnchor();
	
	if (element.GetX().GetAnimVal().Count())
		m_tx = element.GetX().GetAnimVal()[0];
	if (element.GetY().GetAnimVal().Count())
		m_ty = element.GetY().GetAnimVal()[0];
	InitChildren(element, style, matrix);
	
	if (element.GetX().GetAnimVal().Count())
		EndTextAnchor();
}

void wxSVGCanvasText::InitChildren(wxSVGTextPositioningElement& element, const wxCSSStyleDeclaration& style,
		wxSVGMatrix* matrix) {
	wxString text;
	wxSVGElement* elem = (wxSVGElement*) element.GetChildren();
	while (elem) {
		if (elem->GetType() == wxSVGXML_TEXT_NODE) {
			if (element.GetXmlspace() == wxT("preserve")) {
				wxString t = elem->GetContent();
				for (unsigned int i = 0; i< t.length(); i++)
					if (t[i] == wxT('\t') || t[i] == wxT('\n') || t[i] == wxT('\r'))
						t[i] = wxT('\t');
				text += t;
			} else {
				if (elem->GetPreviousSibling() != NULL) {
					wxChar ch = elem->GetContent().GetChar(0);
					if (ch == wxT(' ') || ch == wxT('\t') || ch == wxT('\n') || ch == wxT('\r'))
						text += wxT(' ');
				}
				wxStringTokenizer tokenizer(elem->GetContent());
				while (tokenizer.HasMoreTokens()) {
					text += tokenizer.GetNextToken();
					if (tokenizer.HasMoreTokens()) {
						text += wxT(' ');
					} else if (elem->GetNext() != NULL) {
						wxChar ch = elem->GetContent().Last();
						if (ch == wxT(' ') || ch == wxT('\t') || ch == wxT('\n') || ch == wxT('\r'))
							text += wxT(' ');
					}
				}
			}
		} else if (elem->GetType() == wxSVGXML_ELEMENT_NODE && elem->GetDtd() == wxSVG_TBREAK_ELEMENT) {
			text += wxT("\n");
		} else if (text.length()) {
			AddChunk(text, style, matrix);
			text = wxT("");
		}
		
		if (elem->GetType() == wxSVGXML_ELEMENT_NODE && elem->GetDtd() == wxSVG_TSPAN_ELEMENT) {
			wxSVGTSpanElement& tElem = (wxSVGTSpanElement&) *elem;
			wxCSSStyleDeclaration tStyle(style);
			tStyle.Add(tElem.GetStyle());
			Init(tElem, tStyle, matrix);
		}
		elem = (wxSVGElement*) elem->GetNext();
	}
	if (text.length())
		AddChunk(text, style, matrix);
}

void wxSVGCanvasText::AddChunk(const wxString& text, const wxCSSStyleDeclaration& style, wxSVGMatrix* matrix) {
	wxSVGCanvasTextChunk* chunk = new wxSVGCanvasTextChunk;
	chunk->style.Add(style);
	chunk->x = m_tx;
	chunk->y = m_ty;
	chunk->text = text;
	m_chunks.Add(chunk);
	
	// set textAnchor and dominantBaseline if they are not already set
	if (style.HasTextAnchor() && m_textAnchor == wxCSS_VALUE_START) {
		m_textAnchor = style.GetTextAnchor();
		m_textAnchorBeginIndex = m_chunks.Count() - 1;
		m_textAnchorBeginPos = m_tx;
	}
	if (style.HasDominantBaseline() && (m_dominantBaseline == wxCSS_VALUE_AUTO || m_dominantBaseline
			== wxCSS_VALUE_ALPHABETIC)) {
		m_dominantBaseline = style.GetDominantBaseline();
		m_dominantBaselineBeginIndex = m_chunks.Count() - 1;
	}
	
	InitText(text, style, matrix);
}

void wxSVGCanvasText::BeginChar(wxSVGMatrix* matrix) {
	m_char = new wxSVGCanvasTextChar;
	m_char->path = m_canvas->CreateCanvasPath(matrix);
	m_chunks[m_chunks.GetCount() - 1].chars.Add(m_char);
}

void wxSVGCanvasText::EndChar() {
	m_char->path->End();
}

void wxSVGCanvasText::EndTextAnchor() {
	if (m_textAnchor != wxCSS_VALUE_START) {
		for (int i = m_textAnchorBeginIndex; i < (int) m_chunks.Count(); i++) {
			wxSVGCanvasTextChunk& chunk = m_chunks[i];
			if (m_textAnchor == wxCSS_VALUE_END)
				chunk.matrix = chunk.matrix.Translate(m_textAnchorBeginPos - m_tx, 0);
			else if (m_textAnchor == wxCSS_VALUE_MIDDLE)
				chunk.matrix = chunk.matrix.Translate((m_textAnchorBeginPos - m_tx) / 2, 0);
		}
		if (m_textAnchor == wxCSS_VALUE_END)
			m_tx = m_textAnchorBeginPos;
		else if (m_textAnchor == wxCSS_VALUE_MIDDLE)
			m_tx -= (m_textAnchorBeginPos - m_tx) / 2;
		m_textAnchor = wxCSS_VALUE_START;
	}
	if (m_dominantBaseline != wxCSS_VALUE_AUTO && m_dominantBaseline != wxCSS_VALUE_ALPHABETIC) {
		for (int i = m_dominantBaselineBeginIndex; i < (int) m_chunks.Count(); i++) {
			wxSVGCanvasTextChunk& chunk = m_chunks[i];
			wxSVGRect chunkBBox = chunk.GetBBox();
			if (chunkBBox.IsEmpty())
				continue;
			if (m_dominantBaseline == wxCSS_VALUE_MIDDLE || m_dominantBaseline == wxCSS_VALUE_CENTRAL)
				chunk.matrix = chunk.matrix.Translate(0, m_ty - chunkBBox.GetY() - chunkBBox.GetHeight() / 2);
			else if (m_dominantBaseline == wxCSS_VALUE_TEXT_AFTER_EDGE)
				chunk.matrix = chunk.matrix.Translate(0, m_ty - chunkBBox.GetY());
			else if (m_dominantBaseline == wxCSS_VALUE_TEXT_BEFORE_EDGE)
				chunk.matrix = chunk.matrix.Translate(0, m_ty - chunkBBox.GetY() - chunkBBox.GetHeight());
		}
		m_dominantBaseline = wxCSS_VALUE_AUTO;
	}
}

wxSVGRect wxSVGCanvasTextChunk::GetBBox(const wxSVGMatrix& matrix) {
	wxSVGRect bbox;
	for (int i = 0; i < (int) chars.Count(); i++) {
		wxSVGRect elemBBox = chars[i].path->GetBBox(matrix);
		if (elemBBox.IsEmpty())
			elemBBox = &matrix ? chars[i].bbox.MatrixTransform(matrix) : chars[i].bbox;
		if (i == 0)
			bbox = elemBBox;
		else {
			if (bbox.GetX() > elemBBox.GetX()) {
				bbox.SetWidth(bbox.GetWidth() + bbox.GetX() - elemBBox.GetX());
				bbox.SetX(elemBBox.GetX());
			}
			if (bbox.GetY() > elemBBox.GetY()) {
				bbox.SetHeight(bbox.GetHeight() + bbox.GetY() - elemBBox.GetY());
				bbox.SetY(elemBBox.GetY());
			}
			if (bbox.GetX() + bbox.GetWidth() < elemBBox.GetX() + elemBBox.GetWidth())
				bbox.SetWidth(elemBBox.GetX() + elemBBox.GetWidth() - bbox.GetX());
			if (bbox.GetY() + bbox.GetHeight() < elemBBox.GetY() + elemBBox.GetHeight())
				bbox.SetHeight(elemBBox.GetY() + elemBBox.GetHeight() - bbox.GetY());
		}
	}
	return bbox;
}

wxSVGRect wxSVGCanvasText::GetBBox(const wxSVGMatrix& matrix)
{
  wxSVGRect bbox;
  for (int i=0; i<(int)m_chunks.Count(); i++)
  {
    wxSVGMatrix tmpMatrix = m_chunks[i].matrix;
    if (&matrix)
      tmpMatrix = ((wxSVGMatrix&) matrix).Multiply(m_chunks[i].matrix);
    wxSVGRect elemBBox = m_chunks[i].GetBBox(tmpMatrix);
	if (i == 0)
	  bbox = elemBBox;
	else
	{
	  if (bbox.GetX() > elemBBox.GetX())
	  {
		bbox.SetWidth(bbox.GetWidth() + bbox.GetX() - elemBBox.GetX());
		bbox.SetX(elemBBox.GetX());
	  }
	  if (bbox.GetY() > elemBBox.GetY())
	  {
		bbox.SetHeight(bbox.GetHeight() + bbox.GetY() - elemBBox.GetY());
		bbox.SetY(elemBBox.GetY());
	  }
	  if (bbox.GetX() + bbox.GetWidth() < elemBBox.GetX() + elemBBox.GetWidth())
		bbox.SetWidth(elemBBox.GetX() + elemBBox.GetWidth() - bbox.GetX());
	  if (bbox.GetY() + bbox.GetHeight() < elemBBox.GetY() + elemBBox.GetHeight())
		bbox.SetHeight(elemBBox.GetY() + elemBBox.GetHeight() - bbox.GetY());
	}
  }
  return bbox;
}

long wxSVGCanvasText::GetNumberOfChars()
{
	long res = 0;
	for (int i=0; i<(int)m_chunks.Count(); i++)
		res += m_chunks[i].chars.GetCount();
	return res;
}

wxSVGCanvasTextChunk* wxSVGCanvasText::GetChunk(unsigned long& charnum)
{
	for (int i=0; i<(int)m_chunks.Count(); i++)
	{
		if (charnum<m_chunks[i].chars.GetCount())
			return &m_chunks[i];
		charnum -= m_chunks[i].chars.GetCount();
	}
	return NULL;
}

double wxSVGCanvasText::GetComputedTextLength()
{
	if (m_chunks.Count() && m_chunks[0].chars.GetCount())
	{
		wxSVGCanvasTextChunk& firstChunk = m_chunks[0]; 
		wxSVGRect bboxFirst = firstChunk.chars[0].path->GetBBox();
		if (bboxFirst.IsEmpty())
			bboxFirst = firstChunk.chars[0].bbox;
		bboxFirst = bboxFirst.MatrixTransform(firstChunk.matrix);
		wxSVGCanvasTextChunk& lastChunk = m_chunks[m_chunks.Count()-1];
		wxSVGRect bboxLast = lastChunk.chars[lastChunk.chars.Count()-1].path->GetBBox();
		if (bboxLast.IsEmpty())
			bboxLast = lastChunk.chars[lastChunk.chars.Count()-1].bbox;
		bboxLast = bboxLast.MatrixTransform(lastChunk.matrix);
		return (double)(bboxLast.GetX() + bboxLast.GetWidth() - bboxFirst.GetX());
	}
	return 0;
}

double wxSVGCanvasText::GetSubStringLength(unsigned long charnum, unsigned long nchars)
{
	unsigned long lastCharnum = charnum + nchars - 1;
	wxSVGCanvasTextChunk* firstChunk = GetChunk(charnum);
	wxSVGCanvasTextChunk* lastChunk = GetChunk(lastCharnum);
	if (firstChunk != NULL && lastChunk != NULL)
	{
		wxSVGRect bboxFirst = firstChunk->chars[charnum].path->GetBBox();
		if (bboxFirst.IsEmpty())
			bboxFirst = firstChunk->chars[charnum].bbox;
		bboxFirst = bboxFirst.MatrixTransform(firstChunk->matrix);
		wxSVGRect bboxLast = lastChunk->chars[lastCharnum].path->GetBBox();
		if (bboxLast.IsEmpty())
			bboxLast = lastChunk->chars[lastCharnum].bbox;
		bboxLast = bboxLast.MatrixTransform(lastChunk->matrix);
		return (double)(bboxLast.GetX() + bboxLast.GetWidth() - bboxFirst.GetX());
	}
	return 0;
}

wxSVGPoint wxSVGCanvasText::GetStartPositionOfChar(unsigned long charnum)
{
	wxSVGCanvasTextChunk* chunk = GetChunk(charnum);
	if (chunk != NULL)
	{
		wxSVGRect bbox = chunk->chars[charnum].path->GetBBox();
		if (bbox.IsEmpty())
			bbox = chunk->chars[charnum].bbox;
		bbox = bbox.MatrixTransform(chunk->matrix);
		return wxSVGPoint(bbox.GetX(), bbox.GetY());
	}
	return wxSVGPoint(0, 0);
}

wxSVGPoint wxSVGCanvasText::GetEndPositionOfChar(unsigned long charnum)
{
    wxSVGCanvasTextChunk* chunk = GetChunk(charnum);
	if (chunk != NULL)
	{
		wxSVGRect bbox = chunk->chars[charnum].path->GetBBox();
		if (bbox.IsEmpty())
			bbox = chunk->chars[charnum].bbox;
		bbox = bbox.MatrixTransform(chunk->matrix);
        return wxSVGPoint(bbox.GetX() + bbox.GetWidth(), bbox.GetY());
    }
    return wxSVGPoint(0, 0);
}

wxSVGRect wxSVGCanvasText::GetExtentOfChar(unsigned long charnum)
{
    wxSVGCanvasTextChunk* chunk = GetChunk(charnum);
	if (chunk != NULL)
	{
		wxSVGRect bbox = chunk->chars[charnum].path->GetBBox();
		if (bbox.IsEmpty())
			bbox = chunk->chars[charnum].bbox;
		return bbox.MatrixTransform(chunk->matrix);
    }
    return wxSVGRect(0, 0, 0, 0);
}

long wxSVGCanvasText::GetCharNumAtPosition(const wxSVGPoint& point)
{
	double X = point.GetX();
	double Y = point.GetY();
	for (int n = 0; n < (int)m_chunks.Count(); n++)
	{
		wxSVGCanvasTextChunk& chunk = m_chunks[n];
		wxSVGRect bbox;
		for (int i = 0; i < (int)chunk.chars.Count();i++)
		{
			bbox = chunk.chars[i].path->GetBBox().MatrixTransform(chunk.matrix);
			double Xmin = bbox.GetX();
			double Xmax = Xmin + bbox.GetWidth(); 
			double Ymin = bbox.GetY();
			double Ymax = Ymin + bbox.GetHeight(); 
			if (X >= Xmin && X <= Xmax && Y >= Ymin && Y <= Ymax)
				return i;
		}
	}
    return -1;
}

double wxSVGCanvasText::GetRotationOfChar(unsigned long charnum)
{
    return 0;
}


//////////////////////////////////////////////////////////////////////////////
////////////////////////// wxSVGCanvasSvgImageData ///////////////////////////
//////////////////////////////////////////////////////////////////////////////
wxSVGCanvasSvgImageData::wxSVGCanvasSvgImageData(const wxString& filename, wxSVGDocument* doc) {
	m_count = 1;
	m_svgImage = NULL;
	wxSVGDocument imgDoc;
	if (imgDoc.Load(filename) && imgDoc.GetRootElement() != NULL) {
		m_svgImage = imgDoc.GetRootElement();
		imgDoc.RemoveChild(m_svgImage);
		
		m_svgImage->SetOwnerDocument(doc);
		if (m_svgImage->GetViewBox().GetBaseVal().IsEmpty()
				&& m_svgImage->GetWidth().GetBaseVal().GetValue() > 0
				&& m_svgImage->GetWidth().GetBaseVal().GetUnitType() != wxSVG_LENGTHTYPE_PERCENTAGE)
			m_svgImage->SetViewBox(
					wxSVGRect(0, 0, m_svgImage->GetWidth().GetBaseVal(), m_svgImage->GetHeight().GetBaseVal()));
	}
}

wxSVGCanvasSvgImageData::wxSVGCanvasSvgImageData(wxSVGSVGElement* svgImage, wxSVGDocument* doc) {
	m_count = 1;
	m_svgImage = new wxSVGSVGElement(*svgImage);
	m_svgImage->SetOwnerDocument(doc);
}

wxSVGCanvasSvgImageData::~wxSVGCanvasSvgImageData() {
	if (m_svgImage)
		delete m_svgImage;
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// wxSVGCanvasImage //////////////////////////////
//////////////////////////////////////////////////////////////////////////////
wxSVGCanvasImage::~wxSVGCanvasImage() {
	if (m_svgImageData != NULL && m_svgImageData->DecRef() == 0)
		delete m_svgImageData;
}

void wxSVGCanvasImage::Init(wxSVGImageElement& element, const wxCSSStyleDeclaration& style,
		wxProgressDialog* progressDlg) {
	m_x = element.GetX().GetAnimVal();
	m_y = element.GetY().GetAnimVal();
	m_width = element.GetWidth().GetAnimVal();
	m_height = element.GetHeight().GetAnimVal();
	m_href = element.GetHref();
	m_preserveAspectRatio = element.GetPreserveAspectRatio();
	m_defHeightScale = 1;
	wxSVGCanvasImage* prevItem = (wxSVGCanvasImage*) element.GetCanvasItem();
	if (prevItem != NULL && prevItem->m_href == m_href) {
		m_image = prevItem->m_image;
		m_defHeightScale = prevItem->m_defHeightScale;
		if (prevItem->m_svgImageData) {
			m_svgImageData = prevItem->m_svgImageData;
			m_svgImageData->IncRef();
		}
	} else if (m_href.length()) {
		long pos = 0;
		wxString filename = m_href;
		if (filename.StartsWith(wxT("concat:"))) {
			if (filename.Find(wxT('#')) != wxNOT_FOUND && filename.AfterLast(wxT('#')).ToLong(&pos))
				filename = filename.BeforeLast(wxT('#'));
		} else {
			wxFileName fn(filename);
			if (fn.IsRelative() && element.GetOwnerDocument() != NULL) {
				wxString path = ((wxSVGDocument*) element.GetOwnerDocument())->GetPath();
				if (path.length() && (wxFileExists(path + wxFILE_SEP_PATH + filename)
						|| (filename.Find(wxT('#')) != wxNOT_FOUND
								&& wxFileExists(path + wxFILE_SEP_PATH + filename.BeforeLast(wxT('#')))))) {
					filename = path + wxFILE_SEP_PATH + filename;
				}
			}
			if (!wxFileExists(filename) && filename.Find(wxT('#')) != wxNOT_FOUND
					&& filename.AfterLast(wxT('#')).ToLong(&pos))
				filename = filename.BeforeLast(wxT('#'));
			if (!wxFileExists(filename)) {
				wxLogError(_("Can't load image from file '%s': file does not exist."), filename.c_str());
				return;
			}
			if (filename.EndsWith(wxT(".svg"))) {
				m_svgImageData = new wxSVGCanvasSvgImageData(filename, (wxSVGDocument*) element.GetOwnerDocument());
				if (m_svgImageData->GetSvgImage() == NULL) {
					delete m_svgImageData;
					m_svgImageData = NULL;
				}
				return;
			}
		}
#ifdef USE_LIBAV
		bool log = wxLog::EnableLogging(false);
		m_image.LoadFile(filename);
		wxLog::EnableLogging(log);
		if (!m_image.Ok()) {
			wxFfmpegMediaDecoder decoder;
			if (decoder.Load(filename)) {
				if (progressDlg) {
					progressDlg->Pulse();
					wxYield();
				}
				double duration = decoder.GetDuration();
				if (duration > 0 || pos > 0) {
					m_image = decoder.GetNextFrame();
					double dpos = pos > 0 ? ((double)pos)/1000 : (duration < 6000 ? duration * 0.05 : 300);
					if (!decoder.SetPosition(dpos > 1.0 ? dpos - 1.0 : 0.0)) {
						wxLog* oldLog = wxLog::SetActiveTarget(new wxLogStderr());
						wxLogError(wxT("decoder.GetDuration(): %f"), duration);
						wxLogError(wxT("decoder.SetPosition(%f) failed"), dpos > 1.0 ? dpos - 1.0 : dpos);
						delete wxLog::SetActiveTarget(oldLog);
					}
					for (int i = 0; i < 60; i++) {
						m_image = decoder.GetNextFrame();
						double dpos1 = decoder.GetPosition();
						if (dpos1 >= dpos || dpos1 < 0)
							break;
						if (progressDlg) {
							progressDlg->Pulse();
							wxYield();
						}
					}
				} else {
					for (int i = 0; i < 30; i++)
						m_image = decoder.GetNextFrame();
				}
				if (m_image.Ok() && decoder.GetFrameAspectRatio() > 0)
					m_defHeightScale = ((double)m_image.GetWidth())/m_image.GetHeight()/decoder.GetFrameAspectRatio();
				decoder.Close();
			}
		}
#else
		m_image.LoadFile(filename);
#endif
	}
}

int wxSVGCanvasImage::GetDefaultWidth() {
	if (GetSvgImage() != NULL && !GetSvgImage()->GetViewBox().GetBaseVal().IsEmpty())
		return GetSvgImage()->GetViewBox().GetBaseVal().GetWidth();
	return  m_image.Ok() ? m_image.GetWidth() : 0;
}

int wxSVGCanvasImage::GetDefaultHeight() {
	if (GetSvgImage() != NULL && !GetSvgImage()->GetViewBox().GetBaseVal().IsEmpty())
		return GetSvgImage()->GetViewBox().GetBaseVal().GetHeight();
	return m_image.Ok() ? m_image.GetHeight() * m_defHeightScale : 0;
}

wxSVGSVGElement* wxSVGCanvasImage::GetSvgImage(wxSVGDocument* doc) {
	if (m_svgImageData == NULL)
		return NULL;
	if (doc != NULL) {
		if (m_svgImageData->GetSvgImage()->GetOwnerDocument() == NULL) {
			m_svgImageData->GetSvgImage()->SetOwnerDocument(doc);
		} else if (m_svgImageData->GetSvgImage()->GetOwnerDocument() != doc) {
			wxSVGCanvasSvgImageData* svgImageDataOld = m_svgImageData;
			m_svgImageData = new wxSVGCanvasSvgImageData(m_svgImageData->GetSvgImage(), doc);
			wxSVGDocument::ApplyAnimation(m_svgImageData->GetSvgImage());
			if (svgImageDataOld->DecRef() == 0)
				delete svgImageDataOld;
		}
	}
	return m_svgImageData->GetSvgImage();
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// wxSVGCanvasVideo //////////////////////////////
//////////////////////////////////////////////////////////////////////////////
wxSVGCanvasVideoData::wxSVGCanvasVideoData(wxFfmpegMediaDecoder* mediaDecoder) {
	m_count = 1;
	m_mediaDecoder = mediaDecoder;
}

wxSVGCanvasVideoData::~wxSVGCanvasVideoData() {
#ifdef USE_LIBAV
	if (m_mediaDecoder)
		delete m_mediaDecoder;
#endif
}

wxImage wxSVGCanvasVideoData::GetImage(double time) {
#ifdef USE_LIBAV
	double currTime = m_mediaDecoder->GetPosition();
	double ftime = m_mediaDecoder->GetFps() >= 1 ? 1.0 / m_mediaDecoder->GetFps() : 0.04;
	if (currTime >= time + ftime/2 || currTime < time - ftime/2 || !m_image.IsOk()) {
		if (currTime > time || time - currTime > 50*ftime) {
			m_mediaDecoder->SetPosition(time > 1.0 ? time - 1.0 : 0.0);
		}
		for (int i = 0; i < 60; i++) {
			m_image = m_mediaDecoder->GetNextFrame();
			currTime = m_mediaDecoder->GetPosition();
			if (currTime >= time - ftime/2 || currTime < 0) {
				break;
			}
		}
	}
#endif
	return m_image;
}

wxSVGCanvasVideo::wxSVGCanvasVideo(): wxSVGCanvasImage(wxSVG_CANVAS_ITEM_VIDEO) {
	m_time = 0;
	m_duration = 0;
	m_videoData = NULL;
}

wxSVGCanvasVideo::~wxSVGCanvasVideo() {
	if (m_videoData != NULL && m_videoData->DecRef() == 0)
		delete m_videoData;
}

void wxSVGCanvasVideo::Init(wxSVGVideoElement& element, const wxCSSStyleDeclaration& style,
		wxProgressDialog* progressDlg) {
	m_x = element.GetX().GetAnimVal();
	m_y = element.GetY().GetAnimVal();
	m_width = element.GetWidth().GetAnimVal();
	m_height = element.GetHeight().GetAnimVal();
	m_href = element.GetHref();
	m_preserveAspectRatio = element.GetPreserveAspectRatio();
	m_defHeightScale = 1;
	m_time = element.GetOwnerDocument() != NULL ? ((wxSVGDocument*) element.GetOwnerDocument())->GetCurrentTime() : 0;
	if (element.GetBegin() > 0)
		m_time = m_time > element.GetBegin() ? m_time - element.GetBegin() : 0;
	double dur = element.GetDur() > 0 ? element.GetDur() : element.GetClipEnd() - element.GetClipBegin();
	if (dur > 0 && m_time > dur)
		m_time = dur;
	m_time += element.GetClipBegin();
#ifdef USE_LIBAV
	wxSVGCanvasVideo* prevItem = (wxSVGCanvasVideo*) element.GetCanvasItem();
	if (prevItem != NULL && prevItem->m_href == m_href && prevItem->m_videoData != NULL) {
		m_videoData = prevItem->m_videoData;
		m_videoData->IncRef();
		m_duration = prevItem->m_duration;
		m_defHeightScale = prevItem->m_defHeightScale;
		wxFfmpegMediaDecoder* decoder = m_videoData->GetMediaDecoder();
		if (decoder != NULL) {
			double prevTime = prevItem->m_time;
			double ftime = decoder->GetFps() >= 1 ? 1.0 / decoder->GetFps() : 0.04;
			if (prevTime >= m_time + ftime/2 || prevTime < m_time - ftime/2) {
				m_image = m_videoData->GetImage(m_time);
			} else {
				m_image = prevItem->m_image;
			}
		}
	} else if (m_href.length()) {
		wxFfmpegMediaDecoder* decoder = new wxFfmpegMediaDecoder();
		if (decoder->Load(m_href)) {
			m_duration = decoder->GetDuration();
			if (m_time > 0) {
				m_image = decoder->GetNextFrame();
				if (!decoder->SetPosition(m_time > 1.0 ? m_time - 1.0 : 0.0)) {
					wxLog* oldLog = wxLog::SetActiveTarget(new wxLogStderr());
					wxLogError(wxT("decoder.GetDuration(): %f"), m_duration);
					wxLogError(wxT("decoder.SetPosition(%f) failed"), m_time > 1.0 ? m_time - 1.0 : m_time);
					delete wxLog::SetActiveTarget(oldLog);
				}
				for (int i = 0; i < 60; i++) {
					m_image = decoder->GetNextFrame();
					double currTime = decoder->GetPosition();
					if (currTime >= m_time || currTime < 0)
						break;
				}
			} else
				m_image = decoder->GetNextFrame();
			m_videoData = new wxSVGCanvasVideoData(decoder);
			if (m_image.Ok() && decoder->GetFrameAspectRatio() > 0)
				m_defHeightScale = ((double)m_image.GetWidth())/m_image.GetHeight()/decoder->GetFrameAspectRatio();
		} else {
			delete decoder;
			m_duration = 0;
		}
	}
#endif
}
