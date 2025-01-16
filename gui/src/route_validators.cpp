#include "route_validators.h"

/*!
 * Route point name validator.
 */
RoutePointNameValidator::RoutePointNameValidator()
    : wxTextValidator(wxFILTER_ALPHANUMERIC) {}

/*!
 * Validate the route point name.
 * @param parent Parent window.
 * @return True if valid, false otherwise.
 */
bool RoutePointNameValidator::Validate(wxWindow* parent) {
  wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(GetWindow());
  if (textCtrl) {
    if (textCtrl->GetValue().Len() < 3) {
      wxMessageBox("Name is too short!", "Error", wxOK | wxICON_ERROR, parent);
      return false;
    }
  }
  return true;
}
