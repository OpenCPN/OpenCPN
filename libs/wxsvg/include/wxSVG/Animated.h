//////////////////////////////////////////////////////////////////////////////
// Name:        Animated.h
// Purpose:
// Author:      Alex Thuering
// Created:     2005/05/07
// RCS-ID:      $Id: Animated.h,v 1.3 2007/09/21 06:47:34 etisserant Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#ifndef WXSVG_ANIMATED_H
#define WXSVG_ANIMATED_H

#include "String_wxsvg.h"

#define WXSVG_MAKE_ANIMATED(X, T) class wxSVGAnimated##X\
{\
  public:\
    inline T& GetBaseVal() { return m_baseVal; }\
	inline const T& GetBaseVal() const { return m_baseVal; }\
	inline void SetBaseVal(const T& value) { m_baseVal = m_animVal = value; }\
    \
	inline T& GetAnimVal() { return m_animVal; }\
	inline const T& GetAnimVal() const { return m_animVal; }\
    inline void SetAnimVal(const T& value) { m_animVal = value; }\
    \
  public:\
    inline operator const T&() const { return GetBaseVal(); }\
    \
  protected:\
    T m_baseVal;\
    T m_animVal;\
};

#define WXSVG_MAKE_ANIMATED_NUMBER(X, T) class wxSVGAnimated##X\
{\
  public:\
    wxSVGAnimated##X(): m_baseVal(0), m_animVal(0) {}\
	\
    inline T GetBaseVal() const { return m_baseVal; };\
	inline void SetBaseVal(T value) { m_baseVal = m_animVal = value; }\
	\
    inline T GetAnimVal() const { return m_animVal; }\
	inline void SetAnimVal(T value) { m_animVal = value; }\
    \
  public:\
    inline operator T() const { return GetBaseVal(); }\
    \
  protected:\
    T m_baseVal;\
    T m_animVal;\
};


#endif // WXSVG_ANIMATED_H

