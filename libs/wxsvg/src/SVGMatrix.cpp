//////////////////////////////////////////////////////////////////////////////
// Name:        SVGMatrix.cpp
// Purpose:     
// Author:      Alex Thuering
// Created:     2005/05/05
// RCS-ID:      $Id: SVGMatrix.cpp,v 1.6 2014/08/09 11:13:02 ntalex Exp $
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGMatrix.h"
#include <wx/math.h>

wxSVGMatrix wxSVGMatrix::Multiply(const wxSVGMatrix& secondMatrix) const {
	wxSVGMatrix res;
	res.SetA(GetA() * secondMatrix.GetA() + GetC() * secondMatrix.GetB());
	res.SetB(GetB() * secondMatrix.GetA() + GetD() * secondMatrix.GetB());
	res.SetC(GetA() * secondMatrix.GetC() + GetC() * secondMatrix.GetD());
	res.SetD(GetB() * secondMatrix.GetC() + GetD() * secondMatrix.GetD());
	res.SetE(GetA() * secondMatrix.GetE() + GetC() * secondMatrix.GetF() + GetE());
	res.SetF(GetB() * secondMatrix.GetE() + GetD() * secondMatrix.GetF() + GetF());
	return res;
}

wxSVGMatrix wxSVGMatrix::Inverse() const {
	wxSVGMatrix res;
	double d = 1.0 / (GetA() * GetD() - GetB() * GetC());
	res.SetA(GetD() * d);
	res.SetB(-GetB() * d);
	res.SetC(-GetC() * d);
	res.SetD(GetA() * d);
	res.SetE(-GetE() * res.GetA() - GetF() * res.GetC());
	res.SetF(-GetE() * res.GetB() - GetF() * res.GetD());
	return res;
}

wxSVGMatrix wxSVGMatrix::Translate(double x, double y) const {
	return Multiply(wxSVGMatrix(1, 0, 0, 1, x, y));
}

wxSVGMatrix wxSVGMatrix::Scale(double scaleFactor) const {
	return Multiply(wxSVGMatrix(scaleFactor, 0, 0, scaleFactor, 0, 0));
}

wxSVGMatrix wxSVGMatrix::ScaleNonUniform(double scaleFactorX, double scaleFactorY) const {
	return Multiply(wxSVGMatrix(scaleFactorX, 0, 0, scaleFactorY, 0, 0));
}

wxSVGMatrix wxSVGMatrix::Rotate(double angle) const {
	angle = angle * M_PI / 180;
	return Multiply(wxSVGMatrix(cos(angle), sin(angle), -sin(angle), cos(angle), 0, 0));
}

wxSVGMatrix wxSVGMatrix::RotateFromVector(double x, double y) const {
	return Multiply(wxSVGMatrix(1, 0, 0, 1, 0, 0));
}

wxSVGMatrix wxSVGMatrix::FlipX() const {
	return Multiply(wxSVGMatrix(-1, 0, 0, 1, 0, 0));
}

wxSVGMatrix wxSVGMatrix::FlipY() const {
	return Multiply(wxSVGMatrix(1, 0, 0, -1, 0, 0));
}

wxSVGMatrix wxSVGMatrix::SkewX(double angle) const {
	return Multiply(wxSVGMatrix(1, 0, tan(angle * M_PI / 180), 1, 0, 0));
}

wxSVGMatrix wxSVGMatrix::SkewY(double angle) const {
	return Multiply(wxSVGMatrix(1, tan(angle * M_PI / 180), 0, 1, 0, 0));
}

