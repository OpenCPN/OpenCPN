#ifndef ROUTE_VALIDATORS_H
#define ROUTE_VALIDATORS_H

#include <wx/wx.h>
#include <wx/valtext.h>

class RoutePointNameValidator : public wxTextValidator {
public:
  RoutePointNameValidator();

  bool Validate(wxWindow* parent) override;
};

#endif  // ROUTE_VALIDATORS_H