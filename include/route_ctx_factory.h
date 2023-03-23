/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Wrapper for creating a RouteCtx based on global vars
 * Author:   Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2023 by Alec Leamas
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
 **************************************************************************/
#ifndef _ROUTE_CTX_FACTORY_H__
#define _ROUTE_CTX_FACTORY_H__

#include <wx/string.h>

#include "nav_object_database.h"
#include "routeman.h"
#include "track.h"

extern Routeman *g_pRouteMan;
extern std::vector<Track*> g_TrackList;

RouteCtx RouteCtxFactory() {
    RouteCtx ctx;
    ctx.find_route_by_guid =
        [](wxString guid) {
            if (!g_pRouteMan) return static_cast<Route*>(0);
            return g_pRouteMan->FindRouteByGUID(guid); };
    ctx.find_track_by_guid =
        [](wxString guid) { 
            if (!g_pRouteMan) return static_cast<Track*>(0);
            return g_pRouteMan->FindTrackByGUID(guid); };
    ctx.delete_route =
        [](Route* route) {
            if (!g_pRouteMan) return;
            g_pRouteMan->DeleteRoute(route, NavObjectChanges::getInstance()); };
    ctx.delete_track =
        [](Track* track) {
            auto it = std::find(g_TrackList.begin(), g_TrackList.end(), track);
              if (it != g_TrackList.end()) {
                g_TrackList.erase(it);
              }
              delete track;
        };
    return ctx;
}
#endif   //  _ROUTE_CTX_FACTORY_H__
