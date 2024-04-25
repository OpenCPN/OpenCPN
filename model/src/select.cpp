/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#include <wx/list.h>
#include <wx/gdicmn.h>

#include "model/base_platform.h"
#include "model/georef.h"
#include "model/nav_object_database.h"
#include "model/route.h"
#include "model/select.h"
#include "model/track.h"

#include "vector2D.h"

Select* pSelect;

Select::Select() {
  pSelectList = new SelectableItemList;
  pixelRadius = g_BasePlatform->GetSelectRadiusPix();
}

Select::~Select() {
  pSelectList->DeleteContents(true);
  pSelectList->Clear();
  delete pSelectList;
}

bool Select::IsSelectableRoutePointValid(RoutePoint *pRoutePoint) {
  SelectItem *pFindSel;

  //    Iterate on the select list
  wxSelectableItemListNode *node = pSelectList->GetFirst();

  while (node) {
    pFindSel = node->GetData();
    if (pFindSel->m_seltype == SELTYPE_ROUTEPOINT &&
        (RoutePoint *)pFindSel->m_pData1 == pRoutePoint)
      return true;
    node = node->GetNext();
  }
  return false;
}

bool Select::AddSelectableRoutePoint(float slat, float slon,
                                     RoutePoint *pRoutePointAdd) {
  SelectItem *pSelItem = new SelectItem;
  pSelItem->m_slat = slat;
  pSelItem->m_slon = slon;
  pSelItem->m_seltype = SELTYPE_ROUTEPOINT;
  pSelItem->m_bIsSelected = false;
  pSelItem->m_pData1 = pRoutePointAdd;

  wxSelectableItemListNode *node;

  if (pRoutePointAdd->m_bIsInLayer)
    node = pSelectList->Append(pSelItem);
  else
    node = pSelectList->Insert(pSelItem);

  pRoutePointAdd->SetSelectNode(node);

  return true;
}

bool Select::AddSelectableRouteSegment(float slat1, float slon1, float slat2,
                                       float slon2, RoutePoint *pRoutePointAdd1,
                                       RoutePoint *pRoutePointAdd2,
                                       Route *pRoute) {
  SelectItem *pSelItem = new SelectItem;
  pSelItem->m_slat = slat1;
  pSelItem->m_slon = slon1;
  pSelItem->m_slat2 = slat2;
  pSelItem->m_slon2 = slon2;
  pSelItem->m_seltype = SELTYPE_ROUTESEGMENT;
  pSelItem->m_bIsSelected = false;
  pSelItem->m_pData1 = pRoutePointAdd1;
  pSelItem->m_pData2 = pRoutePointAdd2;
  pSelItem->m_pData3 = pRoute;

  if (pRoute->m_bIsInLayer)
    pSelectList->Append(pSelItem);
  else
    pSelectList->Insert(pSelItem);

  return true;
}

bool Select::DeleteAllSelectableRouteSegments(Route *pr) {
  SelectItem *pFindSel;

  //    Iterate on the select list
  wxSelectableItemListNode *node = pSelectList->GetFirst();

  while (node) {
    pFindSel = node->GetData();
    if (pFindSel->m_seltype == SELTYPE_ROUTESEGMENT &&
        (Route *)pFindSel->m_pData3 == pr) {
      delete pFindSel;
      wxSelectableItemListNode *d = node;
      node = node->GetNext();
      pSelectList->DeleteNode(d);  // delete node;
    } else
      node = node->GetNext();
  }

  return true;
}

bool Select::DeleteAllSelectableRoutePoints(Route *pr) {
  SelectItem *pFindSel;

  //    Iterate on the select list
  wxSelectableItemListNode *node = pSelectList->GetFirst();

  while (node) {
    pFindSel = node->GetData();
    if (pFindSel->m_seltype == SELTYPE_ROUTEPOINT) {
      RoutePoint *ps = (RoutePoint *)pFindSel->m_pData1;

      //    inner loop iterates on the route's point list
      wxRoutePointListNode *pnode = (pr->pRoutePointList)->GetFirst();
      while (pnode) {
        RoutePoint *prp = pnode->GetData();

        if (prp == ps) {
          delete pFindSel;
          pSelectList->DeleteNode(node);  // delete node;
          prp->SetSelectNode(NULL);

          node = pSelectList->GetFirst();

          goto got_next_outer_node;
        }
        pnode = pnode->GetNext();
      }
    }

    node = node->GetNext();
  got_next_outer_node:
    continue;
  }
  return true;
}

bool Select::AddAllSelectableRoutePoints(Route *pr) {
  if (pr->pRoutePointList->GetCount()) {
    wxRoutePointListNode *node = (pr->pRoutePointList)->GetFirst();

    while (node) {
      RoutePoint *prp = node->GetData();
      AddSelectableRoutePoint(prp->m_lat, prp->m_lon, prp);
      node = node->GetNext();
    }
    return true;
  } else
    return false;
}

bool Select::AddAllSelectableRouteSegments(Route *pr) {
  wxPoint rpt, rptn;
  float slat1, slon1, slat2, slon2;

  if (pr->pRoutePointList->GetCount()) {
    wxRoutePointListNode *node = (pr->pRoutePointList)->GetFirst();

    RoutePoint *prp0 = node->GetData();
    slat1 = prp0->m_lat;
    slon1 = prp0->m_lon;

    node = node->GetNext();

    while (node) {
      RoutePoint *prp = node->GetData();
      slat2 = prp->m_lat;
      slon2 = prp->m_lon;

      AddSelectableRouteSegment(slat1, slon1, slat2, slon2, prp0, prp, pr);

      slat1 = slat2;
      slon1 = slon2;
      prp0 = prp;

      node = node->GetNext();
    }
    return true;
  } else
    return false;
}

bool Select::AddAllSelectableTrackSegments(Track *pr) {
  wxPoint rpt, rptn;
  float slat1, slon1, slat2, slon2;

  if (pr->GetnPoints()) {
    TrackPoint *prp0 = pr->GetPoint(0);
    slat1 = prp0->m_lat;
    slon1 = prp0->m_lon;

    for (int i = 1; i < pr->GetnPoints(); i++) {
      TrackPoint *prp = pr->GetPoint(i);
      slat2 = prp->m_lat;
      slon2 = prp->m_lon;

      AddSelectableTrackSegment(slat1, slon1, slat2, slon2, prp0, prp, pr);

      slat1 = slat2;
      slon1 = slon2;
      prp0 = prp;
    }
    return true;
  } else
    return false;
}

bool Select::UpdateSelectableRouteSegments(RoutePoint *prp) {
  SelectItem *pFindSel;
  bool ret = false;

  //    Iterate on the select list
  wxSelectableItemListNode *node = pSelectList->GetFirst();

  while (node) {
    pFindSel = node->GetData();
    if (pFindSel->m_seltype == SELTYPE_ROUTESEGMENT) {
      if (pFindSel->m_pData1 == prp) {
        pFindSel->m_slat = prp->m_lat;
        pFindSel->m_slon = prp->m_lon;
        ret = true;
        ;
      }

      else if (pFindSel->m_pData2 == prp) {
        pFindSel->m_slat2 = prp->m_lat;
        pFindSel->m_slon2 = prp->m_lon;
        ret = true;
      }
    }
    node = node->GetNext();
  }

  return ret;
}

SelectItem *Select::AddSelectablePoint(float slat, float slon,
                                       const void *pdata, int fseltype) {
  SelectItem *pSelItem = new SelectItem;
  if (pSelItem) {
    pSelItem->m_slat = slat;
    pSelItem->m_slon = slon;
    pSelItem->m_seltype = fseltype;
    pSelItem->m_bIsSelected = false;
    pSelItem->m_pData1 = pdata;

    pSelectList->Append(pSelItem);
  }

  return pSelItem;
}

/*
bool Select::DeleteAllPoints( void )
{
    pSelectList->DeleteContents( true );
    pSelectList->Clear();
    return true;
}
*/

bool Select::DeleteSelectablePoint(void *pdata, int SeltypeToDelete) {
  SelectItem *pFindSel;

  if (NULL != pdata) {
    //    Iterate on the list
    wxSelectableItemListNode *node = pSelectList->GetFirst();

    while (node) {
      pFindSel = node->GetData();
      if (pFindSel->m_seltype == SeltypeToDelete) {
        if (pdata == pFindSel->m_pData1) {
          delete pFindSel;
          delete node;

          if (SELTYPE_ROUTEPOINT == SeltypeToDelete) {
            RoutePoint *prp = (RoutePoint *)pdata;
            prp->SetSelectNode(NULL);
          }

          return true;
        }
      }
      node = node->GetNext();
    }
  }
  return false;
}

bool Select::DeleteAllSelectableTypePoints(int SeltypeToDelete) {
  SelectItem *pFindSel;

  //    Iterate on the list
  wxSelectableItemListNode *node = pSelectList->GetFirst();

  while (node) {
    pFindSel = node->GetData();
    if (pFindSel->m_seltype == SeltypeToDelete) {
      delete node;

      if (SELTYPE_ROUTEPOINT == SeltypeToDelete) {
        RoutePoint *prp = (RoutePoint *)pFindSel->m_pData1;
        prp->SetSelectNode(NULL);
      }
      delete pFindSel;

      node = pSelectList->GetFirst();
      goto got_next_node;
    }

    node = node->GetNext();
  got_next_node:
    continue;
  }
  return true;
}

bool Select::DeleteSelectableRoutePoint(RoutePoint *prp) {
  if (NULL != prp) {
    wxSelectableItemListNode *node =
        (wxSelectableItemListNode *)prp->GetSelectNode();
    if (node) {
      SelectItem *pFindSel = node->GetData();
      if (pFindSel) {
        delete pFindSel;
        delete node;  // automatically removes from list
        prp->SetSelectNode(NULL);
        return true;
      }
    } else
      return DeleteSelectablePoint(prp, SELTYPE_ROUTEPOINT);
  }
  return false;
}

bool Select::ModifySelectablePoint(float lat, float lon, void *data,
                                   int SeltypeToModify) {
  SelectItem *pFindSel;

  //    Iterate on the list
  wxSelectableItemListNode *node = pSelectList->GetFirst();

  while (node) {
    pFindSel = node->GetData();
    if (pFindSel->m_seltype == SeltypeToModify) {
      if (data == pFindSel->m_pData1) {
        pFindSel->m_slat = lat;
        pFindSel->m_slon = lon;
        return true;
      }
    }

    node = node->GetNext();
  }
  return false;
}

bool Select::AddSelectableTrackSegment(float slat1, float slon1, float slat2,
                                       float slon2, TrackPoint *pTrackPointAdd1,
                                       TrackPoint *pTrackPointAdd2,
                                       Track *pTrack) {
  SelectItem *pSelItem = new SelectItem;
  pSelItem->m_slat = slat1;
  pSelItem->m_slon = slon1;
  pSelItem->m_slat2 = slat2;
  pSelItem->m_slon2 = slon2;
  pSelItem->m_seltype = SELTYPE_TRACKSEGMENT;
  pSelItem->m_bIsSelected = false;
  pSelItem->m_pData1 = pTrackPointAdd1;
  pSelItem->m_pData2 = pTrackPointAdd2;
  pSelItem->m_pData3 = pTrack;

  if (pTrack->m_bIsInLayer)
    pSelectList->Append(pSelItem);
  else
    pSelectList->Insert(pSelItem);

  return true;
}

bool Select::DeleteAllSelectableTrackSegments(Track *pt) {
  SelectItem *pFindSel;

  //    Iterate on the select list
  wxSelectableItemListNode *node = pSelectList->GetFirst();

  while (node) {
    pFindSel = node->GetData();
    if (pFindSel->m_seltype == SELTYPE_TRACKSEGMENT &&
        (Track *)pFindSel->m_pData3 == pt) {
      delete pFindSel;
      wxSelectableItemListNode *d = node;
      node = node->GetNext();
      pSelectList->DeleteNode(d);  // delete node;
    } else
      node = node->GetNext();
  }
  return true;
}

bool Select::DeletePointSelectableTrackSegments(TrackPoint *pt) {
  SelectItem *pFindSel;

  //    Iterate on the select list
  wxSelectableItemListNode *node = pSelectList->GetFirst();

  while (node) {
    pFindSel = node->GetData();
    if (pFindSel->m_seltype == SELTYPE_TRACKSEGMENT &&
        ((TrackPoint *)pFindSel->m_pData1 == pt ||
         (TrackPoint *)pFindSel->m_pData2 == pt)) {
      delete pFindSel;
      wxSelectableItemListNode *d = node;
      node = node->GetNext();
      pSelectList->DeleteNode(d);  // delete node;
    } else
      node = node->GetNext();
  }
  return true;
}

bool Select::IsSegmentSelected(float a, float b, float c, float d, float slat,
                               float slon) {
  double adder = 0.;

  // Track segments for some reason can have longitude values > 180.
  // Therefore, we normalize all the lat/lon values here.
  if (a > 90.0) a -= 180.0;
  if (b > 90.0) b -= 180.0;
  if (c > 180.0) c -= 360.0;
  if (d > 180.0) d -= 360.0;
  if (slat > 90.0) slat -= 180.0;
  if (slon > 180.0) slon -= 360.0;

  if ((c * d) < 0.) {
    //    Arrange for points to be increasing longitude, c to d
    double dist, brg;
    DistanceBearingMercator(a, c, b, d, &brg, &dist);
    if (brg < 180.)  // swap points?
    {
      double tmp;
      tmp = c;
      c = d;
      d = tmp;
      tmp = a;
      a = b;
      b = tmp;
    }
    if (d < 0.)  // idl?
    {
      d += 360.;
      if (slon < 0.) adder = 360.;
    }
  }

  //    As a course test, use segment bounding box test
  if ((slat >= (fmin(a, b) - selectRadius)) &&
      (slat <= (fmax(a, b) + selectRadius)) &&
      ((slon + adder) >= (fmin(c, d) - selectRadius)) &&
      ((slon + adder) <= (fmax(c, d) + selectRadius))) {
    //    Use vectors to do hit test....
    vector2D va, vb, vn;

    //    Assuming a Mercator projection
    double ap, cp;
    toSM(a, c, 0., 0., &cp, &ap);
    double bp, dp;
    toSM(b, d, 0., 0., &dp, &bp);
    double slatp, slonp;
    toSM(slat, slon + adder, 0., 0., &slonp, &slatp);

    va.x = slonp - cp;
    va.y = slatp - ap;
    vb.x = dp - cp;
    vb.y = bp - ap;

    double delta = vGetLengthOfNormal(&va, &vb, &vn);
    if (fabs(delta) < (selectRadius * 1852 * 60)) return true;
  }
  return false;
}

void Select::CalcSelectRadius(SelectCtx& ctx) {
  selectRadius = pixelRadius / (ctx.scale * 1852 * 60);
}

SelectItem *Select::FindSelection(SelectCtx& ctx, float slat, float slon,
                                  int fseltype) {
  float a, b, c, d;
  SelectItem *pFindSel;

  CalcSelectRadius(ctx);

  //    Iterate on the list
  wxSelectableItemListNode *node = pSelectList->GetFirst();

  while (node) {
    pFindSel = node->GetData();
    if (pFindSel->m_seltype == fseltype) {
      switch (fseltype) {
        case SELTYPE_ROUTEPOINT:
        case SELTYPE_TIDEPOINT:
        case SELTYPE_CURRENTPOINT:
        case SELTYPE_AISTARGET:
          if ((fabs(slat - pFindSel->m_slat) < selectRadius) &&
              (fabs(slon - pFindSel->m_slon) < selectRadius)) {
                if(fseltype == SELTYPE_ROUTEPOINT) {
                    if (((RoutePoint *)pFindSel->m_pData1)->IsVisibleSelectable(ctx.chart_scale))
                      goto find_ok;
                } else {
                    goto find_ok;
                }
          }
          break;
        case SELTYPE_ROUTESEGMENT:
        case SELTYPE_TRACKSEGMENT: {
          a = pFindSel->m_slat;
          b = pFindSel->m_slat2;
          c = pFindSel->m_slon;
          d = pFindSel->m_slon2;

          if (IsSegmentSelected(a, b, c, d, slat, slon)) goto find_ok;
          break;
        }
        default:
          break;
      }
    }

    node = node->GetNext();
  }

  return NULL;
find_ok:
  return pFindSel;
}

bool Select::IsSelectableSegmentSelected(SelectCtx& ctx, float slat,
                                         float slon, SelectItem *pFindSel) {
  bool valid = false;
  wxSelectableItemListNode *node = pSelectList->GetFirst();

  while (node) {
    if (pFindSel == node->GetData()) {
      valid = true;
      break;
    }
    node = node->GetNext();
  }

  if (valid == false) {
    // not in the list anymore
    return false;
  }
  CalcSelectRadius(ctx);

  float a = pFindSel->m_slat;
  float b = pFindSel->m_slat2;
  float c = pFindSel->m_slon;
  float d = pFindSel->m_slon2;

  return IsSegmentSelected(a, b, c, d, slat, slon);
}

static bool is_selectable_wp(SelectCtx ctx, RoutePoint *wp) {
  if (ctx.show_nav_objects) return true;

  if (wp->m_bIsActive) return true;

  Route *rte;
  rte = FindRouteContainingWaypoint(wp);
  if (rte && rte->IsActive()) return true;

  return false;
}

SelectableItemList Select::FindSelectionList(SelectCtx& ctx, float slat,
                                             float slon, int fseltype) {
  float a, b, c, d;
  SelectItem *pFindSel;
  SelectableItemList ret_list;

  CalcSelectRadius(ctx);

  //    Iterate on the list
  wxSelectableItemListNode *node = pSelectList->GetFirst();

  while (node) {
    pFindSel = node->GetData();
    if (pFindSel->m_seltype == fseltype) {
      switch (fseltype) {
        case SELTYPE_ROUTEPOINT:
          if ((fabs(slat - pFindSel->m_slat) < selectRadius) &&
              (fabs(slon - pFindSel->m_slon) < selectRadius))
            if (is_selectable_wp(ctx, (RoutePoint *)pFindSel->m_pData1))
              if (((RoutePoint *)pFindSel->m_pData1)->IsVisibleSelectable(ctx.chart_scale))
                ret_list.Append(pFindSel);
          break;
        case SELTYPE_TIDEPOINT:
        case SELTYPE_CURRENTPOINT:
        case SELTYPE_AISTARGET:
        case SELTYPE_DRAGHANDLE:
          if ((fabs(slat - pFindSel->m_slat) < selectRadius) &&
              (fabs(slon - pFindSel->m_slon) < selectRadius)) {
            if (is_selectable_wp(ctx, (RoutePoint *)pFindSel->m_pData1))
              ret_list.Append(pFindSel);
          }
          break;
        case SELTYPE_ROUTESEGMENT:
        case SELTYPE_TRACKSEGMENT: {
          a = pFindSel->m_slat;
          b = pFindSel->m_slat2;
          c = pFindSel->m_slon;
          d = pFindSel->m_slon2;

          if (IsSegmentSelected(a, b, c, d, slat, slon)) {
            if (ctx.show_nav_objects ||
                (fseltype == SELTYPE_ROUTESEGMENT &&
                 ((Route *)pFindSel->m_pData3)->m_bRtIsActive)) {
              ret_list.Append(pFindSel);
            }
          }

          break;
        }
        default:
          break;
      }
    }

    node = node->GetNext();
  }

  return ret_list;
}
