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
 * Implement route_validator.h -- route dialog validation checks.
 */

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
 * Validate route point name.
 */
wxString RoutePointNameValidator::IsValid(const wxString& val) const {
  Route* route = FindRouteContainingWaypoint(m_wp_ptr);
  return route ? route->IsPointNameValid(m_wp_ptr, val) : "";
}
