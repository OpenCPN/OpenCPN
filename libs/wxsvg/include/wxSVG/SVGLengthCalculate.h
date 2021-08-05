/////////////////////////////////////////////////////////////////////////////
// Name:        SVGLengthCalculate.h
// Purpose:     calculate length relative to viewport size
// Author:      Alex Thuering
// Created:     2005/06/19
// RCS-ID:      $Id: SVGLengthCalculate.h,v 1.1 2005/07/28 13:02:57 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef WX_SVG_LENGTH_CALCULATE_H
#define WX_SVG_LENGTH_CALCULATE_H

/** calculate length (wxLength) relative to viewport size */
#define WX_SVG_LENGTH_CALC_WIDTH(l, viewportElement)\
  if (l.GetUnitType() == wxSVG_LENGTHTYPE_PERCENTAGE && viewportElement &&\
	  viewportElement->GetDtd() == wxSVG_SVG_ELEMENT)\
	l.ToViewportWidth(((wxSVGSVGElement*)viewportElement)->GetWidth().GetAnimVal())
#define WX_SVG_LENGTH_CALC_HEIGHT(l, viewportElement)\
  if (l.GetUnitType() == wxSVG_LENGTHTYPE_PERCENTAGE && viewportElement &&\
	  viewportElement->GetDtd() == wxSVG_SVG_ELEMENT)\
	l.ToViewportHeight(((wxSVGSVGElement*)viewportElement)->GetHeight().GetAnimVal())
#define WX_SVG_LENGTH_CALC_SIZE(l, viewportElement)\
  if (l.GetUnitType() == wxSVG_LENGTHTYPE_PERCENTAGE && viewportElement &&\
	  viewportElement->GetDtd() == wxSVG_SVG_ELEMENT)\
	l.ToViewportSize(((wxSVGSVGElement*)viewportElement)->GetWidth().GetAnimVal(),\
	  ((wxSVGSVGElement*)viewportElement)->GetHeight().GetAnimVal())

/** calculate length (wxAnimatedLength) relative to viewport size */
#define WX_SVG_ANIM_LENGTH_CALC_WIDTH(l, viewportElement)\
  WX_SVG_LENGTH_CALC_WIDTH(((wxSVGLength&)l.GetBaseVal()), viewportElement)
#define WX_SVG_ANIM_LENGTH_CALC_HEIGHT(l, viewportElement)\
  WX_SVG_LENGTH_CALC_HEIGHT(((wxSVGLength&)l.GetBaseVal()), viewportElement)
#define WX_SVG_ANIM_LENGTH_CALC_SIZE(l, viewportElement)\
  WX_SVG_LENGTH_CALC_SIZE(((wxSVGLength&)l.GetBaseVal()), viewportElement)

/** calculate length list (wxLengthList) relative to viewport size */
#define WX_SVG_LENGTH_LIST_CALC_WIDTH(l, viewportElement)\
  for (unsigned int i = 0; i<l.Count(); i++)\
	WX_SVG_LENGTH_CALC_WIDTH(l[i], viewportElement)
#define WX_SVG_LENGTH_LIST_CALC_HEIGHT(l, viewportElement)\
  for (unsigned int i = 0; i<l.Count(); i++)\
	WX_SVG_LENGTH_CALC_HEIGHT(l[i], viewportElement)
#define WX_SVG_LENGTH_LIST_CALC_SIZE(l, viewportElement)\
  for (unsigned int i = 0; i<l.Count(); i++)\
	WX_SVG_LENGTH_CALC_SIZE(l[i], viewportElement)

/** calculate length list (wxAnimatedLengthList) relative to viewport size */
#define WX_SVG_ANIM_LENGTH_LIST_CALC_WIDTH(l, viewportElement)\
  WX_SVG_LENGTH_LIST_CALC_WIDTH(((wxSVGLengthList&)l.GetBaseVal()), viewportElement)
#define WX_SVG_ANIM_LENGTH_LIST_CALC_HEIGHT(l, viewportElement)\
  WX_SVG_LENGTH_LIST_CALC_HEIGHT(((wxSVGLengthList&)l.GetBaseVal()), viewportElement)
#define WX_SVG_ANIM_LENGTH_LIST_CALC_SIZE(l, viewportElement)\
  WX_SVG_LENGTH_LIST_CALC_SIZE(((wxSVGLengthList&)l.GetBaseVal()), viewportElement)

#endif // WX_SVG_LENGTH_CALCULATE_H
