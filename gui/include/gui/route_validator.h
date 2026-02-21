/***************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Route dialog validation checks.
 */

#ifndef ROUTE_VALIDATOR_H
#define ROUTE_VALIDATOR_H

#include <wx/validate.h>
#include <wx/string.h>

#include "model/route_point.h"

#include "field_text.h"

/**
 * Route point name validator.
 */
class RoutePointNameValidator : public TextValidator {
public:
  RoutePointNameValidator(RoutePoint* wp_ptr);
  wxValidator* Clone() const override;
  wxString IsValid(const wxString& val) const override;

private:
  RoutePoint* m_wp_ptr;
};

#endif  // ROUTE_VALIDATOR_H
