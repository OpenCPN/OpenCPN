//////////////////////////////////////////////////////////////////////////////
// Name:        SVGAnimateTransformElement.cpp
// Purpose:     Implementation of SVGAnimateTransformElement
// Author:      Alex Thuering
// Created:     2014/02/24
// RCS-ID:      $Id: SVGAnimateTransformElement.cpp,v 1.3 2016/01/09 23:31:14 ntalex Exp $
// Copyright:   (c) 2014 Alex Thuering
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#include "SVGAnimateTransformElement.h"
#include "SVGDocument.h"
#include "SVGTransformList.h"
#include "SVGTransformable.h"
#include <wx/log.h>

void UpdateTransform(wxSVGTransform& transform, wxSVG_ANIMATETRANSFORM type, const wxSVGLengthList& values) {
	switch (type) {
	case wxSVG_ANIMATETRANSFORM_TRANSLATE:
		if (values.size() > 1)
			transform.SetTranslate(values[0], values[1]);
		break;
	case wxSVG_ANIMATETRANSFORM_SCALE:
		if (values.size() > 1)
			transform.SetScale(values[0], values[1]);
		else
			transform.SetScale(values[0], values[0]);
		break;
	case wxSVG_ANIMATETRANSFORM_ROTATE:
		if (values.size() > 2)
			transform.SetRotate(values[0], values[1], values[2]);
		else
			transform.SetRotate(values[0], 0, 0);
		break;
	case wxSVG_ANIMATETRANSFORM_SKEWX:
		transform.SetSkewX(values[0]);
		break;
	case wxSVG_ANIMATETRANSFORM_SKEWY:
		transform.SetSkewY(values[0]);
		break;
	}
}

void wxSVGAnimateTransformElement::ApplyAnimation() {
	wxSVGElement* targetElement = GetTargetElement();
	if (targetElement == NULL || GetDur() <= 0 || (GetTo().GetPropertyType() != wxSVG_ANIMATED_LENGTH
			&& GetTo().GetPropertyType() != wxSVG_ANIMATED_LENGTH_LIST))
		return;
	wxSVGLengthList values;
	if (GetCurrentTime() >= GetStartTime() + GetDur()) {
		if (GetTo().GetPropertyType() == wxSVG_ANIMATED_LENGTH) {
			values.push_back(GetTo().GetLength());
		} else {
			values = GetTo().GetLengthList();
		}
	} else if (GetCurrentTime() >= GetStartTime() && GetFrom().GetPropertyType() == GetTo().GetPropertyType()
			&& GetFrom().GetLengthList().size() == GetTo().GetLengthList().size()) {
		if (GetTo().GetPropertyType() == wxSVG_ANIMATED_LENGTH) {
			values.Add(wxSVGLength(GetTo().GetLength().GetUnitType(), GetFrom().GetLength().GetValue()
					+ (GetTo().GetLength().GetValue() - GetFrom().GetLength().GetValue())*
					(GetCurrentTime() - GetStartTime())/GetDur()));
		} else {
			for (unsigned int i = 0; i < GetFrom().GetLengthList().size(); i++) {
				const wxSVGLength& from = GetFrom().GetLengthList()[i];
				const wxSVGLength& to = GetTo().GetLengthList()[i];
				values.Add(wxSVGLength(to.GetUnitType(), from.GetValue()
						+ (to.GetValue() - from.GetValue())*(GetCurrentTime() - GetStartTime())/GetDur()));
			}
		}
	} else {
		return;
	}
	wxSVGTransformable* transformable = wxSVGTransformable::GetSVGTransformable(*targetElement);
	if (transformable != NULL) {
		if (m_transformIdx == -1 || m_transformIdx >= (int) transformable->GetTransformList().GetAnimVal().size()) {
			wxSVGTransformList& transforms = transformable->GetTransformList().GetAnimVal();
			if (GetAdditive() != wxSVG_ANIMATION_ADDITIVE_SUM) {
				transforms.Clear();
			}
			wxSVGTransform* transform = new wxSVGTransform();
			UpdateTransform(*transform, GetType(), values);
			transforms.Add(transform);
			m_transformIdx = transforms.size() - 1; 
		} else {
			UpdateTransform(transformable->GetTransformList().GetAnimVal()[m_transformIdx], GetType(), values);
		}
	} else {
		wxSVGTransformList transforms;
		wxSVGTransform* transform = new wxSVGTransform();
		UpdateTransform(*transform, GetType(), values);
		transforms.Add(transform);
		targetElement->SetAnimatedValue(GetAttributeName(), wxSVGAnimatedType(transforms));
	}
}
