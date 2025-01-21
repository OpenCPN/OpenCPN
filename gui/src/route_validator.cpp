#include "route_validator.h"
#include "model/routeman.h"
#include "model/route.h"

/*!
 * Route point name validator validates the field value.
 */
RoutePointNameValidator::RoutePointNameValidator(RoutePoint* wp_ptr)
    : TextValidator() {
  m_wp_ptr = wp_ptr;
}

wxValidator* RoutePointNameValidator::Clone() const {
  return new RoutePointNameValidator(m_wp_ptr);
}

/**
 * Validate the route point name.
 */
wxString RoutePointNameValidator::IsValid(const wxString& val) const {
  if (val.Len() < 3) {
    return wxString("Name must be at least 3 characters");
  } else {
    Route* route = FindRouteContainingWaypoint(m_wp_ptr);
    if (route) {
      return route->IsPointNameValid(m_wp_ptr, val);
    } else {
      return wxEmptyString;
    }
  }
}