/*
Copyright (C) 2026  Alec Leamas

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ocpn_nlohmann_header
#define ocpn_nlohmann_header

// https://github.com/nlohmann/json/issues/1408
#ifdef _MSC_VER
#undef snprintf
#endif

// See "Implicit Conversions" in https://github.com/nlohmann/json/README.md
#define JSON_USE_IMPLICIT_CONVERSIONS 0

#include "nlohmann/json.hpp"

#endif  //  ocpn_nlohmann_header
