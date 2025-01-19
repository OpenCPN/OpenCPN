#include "route_validator.h"

/*!
 * Route point name validator validates the field value.
 */
RoutePointNameValidator::RoutePointNameValidator() : TextValidator() {}

wxValidator* RoutePointNameValidator::Clone() const {
  return new RoutePointNameValidator();
}

wxString RoutePointNameValidator::IsValid(const wxString& val) const {
  return wxString("Error message");
}
