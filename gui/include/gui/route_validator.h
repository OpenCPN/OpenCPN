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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

/**
 * \file
 * Route validators for dialog validation.
 */
#ifndef ROUTE_VALIDATOR_H
#define ROUTE_VALIDATOR_H

#include "dialog_cntrl.h"
#include "model/route_point.h"

/**
 * Route point name validator to validate the name.
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