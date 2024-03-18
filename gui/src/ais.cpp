/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  AIS Decoder Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include <wx/wx.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>
#include <wx/wfstream.h>
#include <wx/imaglist.h>

#include "model/ais_decoder.h"
#include "model/ais_state_vars.h"
#include "model/ais_target_data.h"
#include "model/cutil.h"
#include "model/georef.h"
#include "model/own_ship.h"
#include "model/select.h"
#include "model/wx28compat.h"

#include "ais.h"
#include "AISTargetAlertDialog.h"
#include "AISTargetQueryDialog.h"
#include "chcanv.h"
#include "FontMgr.h"
#include "line_clip.h"
#include "navutil.h"  // for Select
#include "ocpn_frame.h"
#include "OCPNPlatform.h"
#include "ocpn_plugin.h"
#include "styles.h"

extern MyFrame *gFrame;
extern OCPNPlatform *g_Platform;

int g_ais_cog_predictor_width;
extern AISTargetQueryDialog *g_pais_query_dialog_active;

int ImportanceSwitchPoint = 100;

extern ArrayOfMmsiProperties g_MMSI_Props_Array;
extern bool g_bopengl;

extern float g_ShipScaleFactorExp;

float AISImportanceSwitchPoint = 0.0;

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double *)&lNaN)
#endif

wxString ais8_001_22_notice_names[] = {
    // 128] = {
    _("Caution Area: Marine mammals habitat (implies whales NOT "
      "observed)"),  // 0 - WARNING: extra text by Kurt
    _("Caution Area: Marine mammals in area - reduce speed"),        // 1
    _("Caution Area: Marine mammals in area - stay clear"),          // 2
    _("Caution Area: Marine mammals in area - report sightings"),    // 3
    _("Caution Area: Protected habitat - reduce speed"),             // 4
    _("Caution Area: Protected habitat - stay clear"),               // 5
    _("Caution Area: Protected habitat - no fishing or anchoring"),  // 6
    _("Caution Area: Derelicts (drifting objects)"),                 // 7
    _("Caution Area: Traffic congestion"),                           // 8
    _("Caution Area: Marine event"),                                 // 9
    _("Caution Area: Divers down"),                                  // 10
    _("Caution Area: Swim area"),                                    // 11
    _("Caution Area: Dredge operations"),                            // 12
    _("Caution Area: Survey operations"),                            // 13
    _("Caution Area: Underwater operation"),                         // 14
    _("Caution Area: Seaplane operations"),                          // 15
    _("Caution Area: Fishery - nets in water"),                      // 16
    _("Caution Area: Cluster of fishing vessels"),                   // 17
    _("Caution Area: Fairway closed"),                               // 18
    _("Caution Area: Harbour closed"),                               // 19
    _("Caution Area: Risk (define in Associated text field)"),       // 20
    _("Caution Area: Underwater vehicle operation"),                 // 21
    _("(reserved for future use)"),                                  // 22
    _("Environmental Caution Area: Storm front (line squall)"),      // 23
    _("Environmental Caution Area: Hazardous sea ice"),              // 24
    _("Environmental Caution Area: Storm warning (storm cell or line "
      "of storms)"),                              // 25
    _("Environmental Caution Area: High wind"),   // 26
    _("Environmental Caution Area: High waves"),  // 27
    _("Environmental Caution Area: Restricted visibility (fog, rain, "
      "etc.)"),                                                      // 28
    _("Environmental Caution Area: Strong currents"),                // 29
    _("Environmental Caution Area: Heavy icing"),                    // 30
    _("(reserved for future use)"),                                  // 31
    _("Restricted Area: Fishing prohibited"),                        // 32
    _("Restricted Area: No anchoring."),                             // 33
    _("Restricted Area: Entry approval required prior to transit"),  // 34
    _("Restricted Area: Entry prohibited"),                          // 35
    _("Restricted Area: Active military OPAREA"),                    // 36
    _("Restricted Area: Firing - danger area."),                     // 37
    _("Restricted Area: Drifting Mines"),                            // 38
    _("(reserved for future use)"),                                  // 39
    _("Anchorage Area: Anchorage open"),                             // 40
    _("Anchorage Area: Anchorage closed"),                           // 41
    _("Anchorage Area: Anchoring prohibited"),                       // 42
    _("Anchorage Area: Deep draft anchorage"),                       // 43
    _("Anchorage Area: Shallow draft anchorage"),                    // 44
    _("Anchorage Area: Vessel transfer operations"),                 // 45
    _("(reserved for future use)"),                                  // 46
    _("(reserved for future use)"),                                  // 47
    _("(reserved for future use)"),                                  // 48
    _("(reserved for future use)"),                                  // 49
    _("(reserved for future use)"),                                  // 50
    _("(reserved for future use)"),                                  // 51
    _("(reserved for future use)"),                                  // 52
    _("(reserved for future use)"),                                  // 53
    _("(reserved for future use)"),                                  // 54
    _("(reserved for future use)"),                                  // 55
    _("Security Alert - Level 1"),                                   // 56
    _("Security Alert - Level 2"),                                   // 57
    _("Security Alert - Level 3"),                                   // 58
    _("(reserved for future use)"),                                  // 59
    _("(reserved for future use)"),                                  // 60
    _("(reserved for future use)"),                                  // 61
    _("(reserved for future use)"),                                  // 62
    _("(reserved for future use)"),                                  // 63
    _("Distress Area: Vessel disabled and adrift"),                  // 64
    _("Distress Area: Vessel sinking"),                              // 65
    _("Distress Area: Vessel abandoning ship"),                      // 66
    _("Distress Area: Vessel requests medical assistance"),          // 67
    _("Distress Area: Vessel flooding"),                             // 68
    _("Distress Area: Vessel fire/explosion"),                       // 69
    _("Distress Area: Vessel grounding"),                            // 70
    _("Distress Area: Vessel collision"),                            // 71
    _("Distress Area: Vessel listing/capsizing"),                    // 72
    _("Distress Area: Vessel under assault"),                        // 73
    _("Distress Area: Person overboard"),                            // 74
    _("Distress Area: SAR area"),                                    // 75
    _("Distress Area: Pollution response area"),                     // 76
    _("(reserved for future use)"),                                  // 77
    _("(reserved for future use)"),                                  // 78
    _("(reserved for future use)"),                                  // 79
    _("Instruction: Contact VTS at this point/juncture"),            // 80
    _("Instruction: Contact Port Administration at this "
      "point/juncture"),                                          // 81
    _("Instruction: Do not proceed beyond this point/juncture"),  // 82
    _("Instruction: Await instructions prior to proceeding beyond this "
      "point/juncture"),                                 // 83
    _("Proceed to this location - await instructions"),  // 84
    _("Clearance granted - proceed to berth"),           // 85
    _("(reserved for future use)"),                      // 86
    _("(reserved for future use)"),                      // 87
    _("Information: Pilot boarding position"),           // 88
    _("Information: Icebreaker waiting area"),           // 89
    _("Information: Places of refuge"),                  // 90
    _("Information: Position of icebreakers"),           // 91
    _("Information: Location of response units"),        // 92
    _("VTS active target"),                              // 93
    _("Rogue or suspicious vessel"),                     // 94
    _("Vessel requesting non-distress assistance"),      // 95
    _("Chart Feature: Sunken vessel"),                   // 96
    _("Chart Feature: Submerged object"),                // 97
    _("Chart Feature: Semi-submerged object"),           // 98
    _("Chart Feature: Shoal area"),                      // 99
    _("Chart Feature: Shoal area due north"),            // 100
    _("Chart Feature: Shoal area due east"),             // 101
    _("Chart Feature: Shoal area due south"),            // 102
    _("Chart Feature: Shoal area due west"),             // 103
    _("Chart Feature: Channel obstruction"),             // 104
    _("Chart Feature: Reduced vertical clearance"),      // 105
    _("Chart Feature: Bridge closed"),                   // 106
    _("Chart Feature: Bridge partially open"),           // 107
    _("Chart Feature: Bridge fully open"),               // 108
    _("(reserved for future use)"),                      // 109
    _("(reserved for future use)"),                      // 110
    _("(reserved for future use)"),                      // 111
    _("Report from ship: Icing info"),                   // 112
    _("(reserved for future use)"),                      // 113
    _("Report from ship: Miscellaneous information - define in "
      "Associated text field"),                    // 114
    _("(reserved for future use)"),                // 115
    _("(reserved for future use)"),                // 116
    _("(reserved for future use)"),                // 117
    _("(reserved for future use)"),                // 118
    _("(reserved for future use)"),                // 119
    _("Route: Recommended route"),                 // 120
    _("Route: Alternative route"),                 // 121
    _("Route: Recommended route through ice"),     // 122
    _("(reserved for future use)"),                // 123
    _("(reserved for future use)"),                // 124
    _("Other - Define in associated text field"),  // 125
    _("Cancellation - cancel area as identified by Message Linkage "
      "ID"),                  // 126
    _("Undefined (default)")  //, // 127
};

static bool GetCanvasPointPix(ViewPort &vp, ChartCanvas *cp, double rlat,
                              double rlon, wxPoint *r) {
  if (cp != NULL) {
    return cp->GetCanvasPointPix(rlat, rlon, r);
  }
  *r = vp.GetPixFromLL(rlat, rlon);
  return true;
}

static wxPoint transrot(wxPoint pt, float sin_theta, float cos_theta,
                        wxPoint offset = wxPoint(0, 0)) {
  wxPoint ret;
  float px = (float)(pt.x * sin_theta) + (float)(pt.y * cos_theta);
  float py = (float)(pt.y * sin_theta) - (float)(pt.x * cos_theta);
  ret.x = (int)wxRound(px);
  ret.y = (int)wxRound(py);
  ret.x += offset.x;
  ret.y += offset.y;

  return ret;
}

static void transrot_pts(int n, wxPoint *pt, float sin_theta, float cos_theta,
                         wxPoint offset = wxPoint(0, 0)) {
  for (int i = 0; i < n; i++)
    pt[i] = transrot(pt[i], sin_theta, cos_theta, offset);
}

void AISDrawAreaNotices(ocpnDC &dc, ViewPort &vp, ChartCanvas *cp) {
  if (cp == NULL) return;
  if (!g_pAIS || !cp->GetShowAIS() || !g_bShowAreaNotices) return;

  wxDateTime now = wxDateTime::Now();
  now.MakeGMT();

  bool b_pens_set = false;
  wxPen pen_save;
  wxBrush brush_save;
  wxColour yellow;
  wxColour green;
  wxPen pen;
  wxBrush *yellow_brush = wxTheBrushList->FindOrCreateBrush(
      wxColour(0, 0, 0), wxBRUSHSTYLE_TRANSPARENT);
  wxBrush *green_brush = wxTheBrushList->FindOrCreateBrush(
      wxColour(0, 0, 0), wxBRUSHSTYLE_TRANSPARENT);
  ;
  wxBrush *brush;

  float vp_scale = vp.view_scale_ppm;

  for (const auto &target : g_pAIS->GetAreaNoticeSourcesList()) {
    auto target_data = target.second;
    if (!target_data->area_notices.empty()) {
      if (!b_pens_set) {
        pen_save = dc.GetPen();
        brush_save = dc.GetBrush();

        yellow = GetGlobalColor(_T ( "YELO1" ));
        yellow.Set(yellow.Red(), yellow.Green(), yellow.Blue(), 64);

        green = GetGlobalColor(_T ( "GREEN4" ));
        green.Set(green.Red(), green.Green(), green.Blue(), 64);

        pen.SetColour(yellow);
        pen.SetWidth(2);

        yellow_brush = wxTheBrushList->FindOrCreateBrush(
            yellow, wxBRUSHSTYLE_CROSSDIAG_HATCH);
        green_brush =
            wxTheBrushList->FindOrCreateBrush(green, wxBRUSHSTYLE_TRANSPARENT);
        brush = yellow_brush;

        b_pens_set = true;
      }

      for (auto &ani : target_data->area_notices) {
        Ais8_001_22 &area_notice = ani.second;

        if (area_notice.expiry_time > now) {
          std::vector<wxPoint> points;
          bool draw_polygon = false;

          switch (area_notice.notice_type) {
            case 0:
              pen.SetColour(green);
              brush = green_brush;
              break;
            case 1:
              pen.SetColour(yellow);
              brush = yellow_brush;
              break;
            default:
              pen.SetColour(yellow);
              brush = yellow_brush;
          }
          dc.SetPen(pen);
          dc.SetBrush(*brush);

          for (Ais8_001_22_SubAreaList::iterator sa =
                   area_notice.sub_areas.begin();
               sa != area_notice.sub_areas.end(); ++sa) {
            switch (sa->shape) {
              case AIS8_001_22_SHAPE_CIRCLE: {
                wxPoint target_point;
                GetCanvasPointPix(vp, cp, sa->latitude, sa->longitude,
                                  &target_point);
                points.push_back(target_point);
                if (sa->radius_m > 0.0)
                  dc.DrawCircle(target_point, sa->radius_m * vp_scale);
                break;
              }
              case AIS8_001_22_SHAPE_RECT: {
                wxPoint target_point;
                double lat = sa->latitude;
                double lon = sa->longitude;
                int orient_east = 90 + sa->orient_deg;
                if (orient_east > 360) orient_east -= 360;
                GetCanvasPointPix(vp, cp, lat, lon, &target_point);
                points.push_back(target_point);
                ll_gc_ll(lat, lon, orient_east, sa->e_dim_m / 1852.0, &lat,
                         &lon);
                GetCanvasPointPix(vp, cp, lat, lon, &target_point);
                points.push_back(target_point);
                ll_gc_ll(lat, lon, sa->orient_deg, sa->n_dim_m / 1852.0, &lat,
                         &lon);
                GetCanvasPointPix(vp, cp, lat, lon, &target_point);
                points.push_back(target_point);
                ll_gc_ll(sa->latitude, sa->longitude, sa->orient_deg,
                         sa->n_dim_m / 1852.0, &lat, &lon);
                GetCanvasPointPix(vp, cp, lat, lon, &target_point);
                points.push_back(target_point);
                draw_polygon = true;
                break;
              }
              case AIS8_001_22_SHAPE_SECTOR: {
                wxPoint target_point;
                double lat, lon;
                double lat1 = sa->latitude;
                double lon1 = sa->longitude;
                GetCanvasPointPix(vp, cp, lat1, lon1, &target_point);
                points.push_back(target_point);

                for (int i = 0; i < 18; ++i) {
                  ll_gc_ll(lat1, lon1, sa->left_bound_deg + i * (sa->right_bound_deg - sa->left_bound_deg) / 18 , sa->radius_m / 1852.0,
                         &lat, &lon);
                  GetCanvasPointPix(vp, cp, lat, lon, &target_point);
                  points.push_back(target_point);
                }
                // Last angle explicitly to avoid any rounding errors
                ll_gc_ll(lat1, lon1, sa->right_bound_deg , sa->radius_m / 1852.0,
                         &lat, &lon);
                GetCanvasPointPix(vp, cp, lat, lon, &target_point);
                points.push_back(target_point);

                draw_polygon = true;
                break;
              }
              case AIS8_001_22_SHAPE_POLYGON:
                draw_polygon = true;
                // FALL THROUGH
              case AIS8_001_22_SHAPE_POLYLINE: {
                double lat = sa->latitude;
                double lon = sa->longitude;
                for (int i = 0; i < 4; ++i) {
                  ll_gc_ll(lat, lon, sa->angles[i], sa->dists_m[i] / 1852.0,
                           &lat, &lon);
                  wxPoint target_point;
                  GetCanvasPointPix(vp, cp, lat, lon, &target_point);
                  points.push_back(target_point);
                }
                dc.DrawLines(points.size(), &points.front());
              }
            }
          }
          if (draw_polygon) dc.DrawPolygon(points.size(), &points.front());
        }
      }
    }
  }

  if (b_pens_set) {
    dc.SetPen(pen_save);
    dc.SetBrush(brush_save);
  }
}

static void TargetFrame(ocpnDC &dc, wxPen pen, int x, int y, int radius) {
  //    Constants?
  int gap2 = 2 * radius / 6;

  wxPen pen_save = dc.GetPen();

  dc.SetPen(pen);

  dc.DrawLine(x - radius, y + gap2, x - radius, y + radius);
  dc.DrawLine(x - radius, y + radius, x - gap2, y + radius);
  dc.DrawLine(x + gap2, y + radius, x + radius, y + radius);
  dc.DrawLine(x + radius, y + radius, x + radius, y + gap2);
  dc.DrawLine(x + radius, y - gap2, x + radius, y - radius);
  dc.DrawLine(x + radius, y - radius, x + gap2, y - radius);
  dc.DrawLine(x - gap2, y - radius, x - radius, y - radius);
  dc.DrawLine(x - radius, y - radius, x - radius, y - gap2);

  dc.SetPen(pen_save);
}

static void AtoN_Diamond(ocpnDC &dc, wxPen pen, int x, int y, int radius,
                         AisTargetData *td) {
  //    Constants?
  wxPen pen_save = dc.GetPen();

  wxPen aton_DrawPen;
  wxPen aton_WhiteBorderPen;
  wxBrush aton_Brush;

  int rad1a = radius / 2;  // size off topmarks of AtoN
  int rad2a = radius / 4;
  int rad3a =
      rad1a -
      1;  // slightly smaller size off topmarks to look better for the eye

  // Set the Pen for what is needed
  if ((td->NavStatus == ATON_VIRTUAL_OFFPOSITION) ||
      (td->NavStatus == ATON_REAL_OFFPOSITION))
    aton_DrawPen = wxPen(GetGlobalColor(_T ( "URED" )), pen.GetWidth());
  else
    aton_DrawPen = wxPen(GetGlobalColor(_T ( "UBLCK" )), pen.GetWidth());

  bool b_virt = (td->NavStatus == ATON_VIRTUAL) |
                (td->NavStatus == ATON_VIRTUAL_ONPOSITION) |
                (td->NavStatus == ATON_VIRTUAL_OFFPOSITION);

  if (b_virt) aton_DrawPen.SetStyle(wxPENSTYLE_SHORT_DASH);

  aton_WhiteBorderPen =
      wxPen(GetGlobalColor(_T ( "CHWHT" )), aton_DrawPen.GetWidth() + 2);

  // Draw Base Diamond. First with Thick White pen then custom pen io to get a
  // white border around the line.
  wxPoint diamond[5];
  diamond[0] = wxPoint(radius, 0);
  diamond[1] = wxPoint(0, -radius);
  diamond[2] = wxPoint(-radius, 0);
  diamond[3] = wxPoint(0, radius);
  diamond[4] = wxPoint(radius, 0);
  dc.SetPen(aton_WhiteBorderPen);
  dc.DrawLines(5, diamond, x, y);
  dc.SetPen(aton_DrawPen);
  dc.DrawLines(5, diamond, x, y);

  aton_DrawPen = wxPen(GetGlobalColor(_T ( "UBLCK" )),
                       pen.GetWidth());  // Change drawing pen to Solid and width 1
  aton_WhiteBorderPen =
      wxPen(GetGlobalColor(_T ( "CHWHT" )), aton_DrawPen.GetWidth() + 2);

  // draw cross inside
  wxPoint cross[5];
  cross[0] = wxPoint(-rad2a, 0);
  cross[1] = wxPoint(rad2a, 0);
  cross[2] = wxPoint(0, 0);
  cross[3] = wxPoint(0, rad2a);
  cross[4] = wxPoint(0, -rad2a);
  dc.SetPen(aton_WhiteBorderPen);
  dc.DrawLines(5, cross, x, y);
  dc.SetPen(aton_DrawPen);
  dc.DrawLines(5, cross, x, y);

  wxPoint TriPointUp[4];  // Declare triangles here for multiple use
  TriPointUp[0] = wxPoint(-rad1a, 0);
  TriPointUp[1] = wxPoint(rad1a, 0);
  TriPointUp[2] = wxPoint(0, -rad1a);
  TriPointUp[3] = wxPoint(-rad1a, 0);

  wxPoint TriPointDown[4];  // Declare triangles here for multiple use
  TriPointDown[0] = wxPoint(-rad1a, -rad1a);
  TriPointDown[1] = wxPoint(rad1a, -rad1a);
  TriPointDown[2] = wxPoint(0, 0);
  TriPointDown[3] = wxPoint(-rad1a, -rad1a);

  wxPoint CircleOpen[16];  // Workaround to draw transparent circles
  CircleOpen[0] = wxPoint(-1, 5);
  CircleOpen[1] = wxPoint(1, 5);
  CircleOpen[2] = wxPoint(3, 4);
  CircleOpen[3] = wxPoint(4, 3);
  CircleOpen[4] = wxPoint(5, 1);
  CircleOpen[5] = wxPoint(5, -1);
  CircleOpen[6] = wxPoint(4, -3);
  CircleOpen[7] = wxPoint(3, -4);
  CircleOpen[8] = wxPoint(1, -5);
  CircleOpen[9] = wxPoint(-1, -5);
  CircleOpen[10] = wxPoint(-3, -4);
  CircleOpen[11] = wxPoint(-4, -3);
  CircleOpen[12] = wxPoint(-5, -1);
  CircleOpen[13] = wxPoint(-4, 3);
  CircleOpen[14] = wxPoint(-3, 4);
  CircleOpen[15] = wxPoint(-1, 5);

  switch (td->ShipType) {
    case 9:
    case 20:  // Card. N
      dc.SetPen(aton_WhiteBorderPen);
      dc.DrawLines(4, TriPointUp, x, y - radius - 1);
      dc.DrawLines(4, TriPointUp, x, y - radius - rad1a - 3);
      dc.SetPen(aton_DrawPen);
      dc.DrawLines(4, TriPointUp, x, y - radius - 1);
      dc.DrawLines(4, TriPointUp, x, y - radius - rad1a - 3);
      break;
    case 10:
    case 21:  // Card E
      dc.SetPen(aton_WhiteBorderPen);
      dc.DrawLines(4, TriPointDown, x, y - radius - 1);
      dc.DrawLines(4, TriPointUp, x, y - radius - rad1a - 3);
      dc.SetPen(aton_DrawPen);
      dc.DrawLines(4, TriPointDown, x, y - radius - 1);
      dc.DrawLines(4, TriPointUp, x, y - radius - rad1a - 3);
      break;
    case 11:
    case 22:  // Card S
      dc.SetPen(aton_WhiteBorderPen);
      dc.DrawLines(4, TriPointDown, x, y - radius - 1);
      dc.DrawLines(4, TriPointDown, x, y - radius - rad1a - 3);
      dc.SetPen(aton_DrawPen);
      dc.DrawLines(4, TriPointDown, x, y - radius - 1);
      dc.DrawLines(4, TriPointDown, x, y - radius - rad1a - 3);
      break;
    case 12:
    case 23:  // Card W
      dc.SetPen(aton_WhiteBorderPen);
      dc.DrawLines(4, TriPointUp, x, y - radius - 1);
      dc.DrawLines(4, TriPointDown, x, y - radius - rad1a - 3);
      dc.SetPen(aton_DrawPen);
      dc.DrawLines(4, TriPointUp, x, y - radius - 1);
      dc.DrawLines(4, TriPointDown, x, y - radius - rad1a - 3);
      break;
    case 13:             // PortHand Beacon IALA-A
    case 24: {           // StarboardHand Beacon IALA-B
      wxPoint aRect[5];  // Square topmark
      aRect[0] = wxPoint(-rad3a, 0);
      aRect[1] = wxPoint(-rad3a, -rad3a - rad3a);
      aRect[2] = wxPoint(rad3a, -rad3a - rad3a);
      aRect[3] = wxPoint(rad3a, 0);
      aRect[4] = wxPoint(-rad3a, 0);
      dc.SetPen(aton_WhiteBorderPen);
      dc.DrawLines(5, aRect, x, y - radius - 1);
      dc.SetPen(aton_DrawPen);
      dc.DrawLines(5, aRect, x, y - radius - 1);
    } break;
    case 14:  // StarboardHand Beacon IALA-A
    case 25:  // PortHand Beacon IALA-B
      dc.SetPen(aton_WhiteBorderPen);
      dc.DrawLines(4, TriPointUp, x, y - radius);
      dc.SetPen(aton_DrawPen);
      dc.DrawLines(4, TriPointUp, x, y - radius);
      break;
    case 17:
    case 28:  // Isolated danger
      dc.SetPen(aton_WhiteBorderPen);
      dc.DrawLines(16, CircleOpen, x, y - radius - 5);
      dc.SetPen(aton_DrawPen);
      dc.DrawLines(16, CircleOpen, x, y - radius - 5);
      dc.SetPen(aton_WhiteBorderPen);
      dc.DrawLines(16, CircleOpen, x, y - radius - 16);
      dc.SetPen(aton_DrawPen);
      dc.DrawLines(16, CircleOpen, x, y - radius - 16);
      break;
    case 18:
    case 29:  // Safe water
      dc.SetPen(aton_WhiteBorderPen);
      dc.DrawLines(16, CircleOpen, x, y - radius - 5);
      dc.SetPen(aton_DrawPen);
      dc.DrawLines(16, CircleOpen, x, y - radius - 5);
      break;
    case 19:
    case 30: {                             // Special Mark
      cross[0] = wxPoint(-rad2a, -rad2a);  // reuse of cross array
      cross[1] = wxPoint(rad2a, rad2a);
      cross[2] = wxPoint(0, 0);
      cross[3] = wxPoint(-rad2a, rad2a);
      cross[4] = wxPoint(rad2a, -rad2a);
      dc.SetPen(aton_WhiteBorderPen);
      dc.DrawLines(5, cross, x, y - radius - rad3a);
      dc.SetPen(aton_DrawPen);
      dc.DrawLines(5, cross, x, y - radius - rad3a);
    } break;
    default:
      break;
  }
  dc.SetPen(pen_save);
}

static void Base_Square(ocpnDC &dc, wxPen pen, int x, int y, int radius) {
  //    Constants?
  int gap2 = 2 * radius / 6;
  int pen_width = pen.GetWidth();

  wxPen pen_save = dc.GetPen();

  dc.SetPen(pen);  // draw square

  dc.DrawLine(x - radius, y - radius, x - radius, y + radius);
  dc.DrawLine(x - radius, y + radius, x + radius, y + radius);
  dc.DrawLine(x + radius, y + radius, x + radius, y - radius);
  dc.DrawLine(x + radius, y - radius, x - radius, y - radius);

  if (pen_width > 1) {
    pen_width -= 1;
    pen.SetWidth(pen_width);
  }  // draw cross inside

  dc.DrawLine(x - gap2, y, x + gap2, y);
  dc.DrawLine(x, y - gap2, x, y + gap2);

  dc.SetPen(pen_save);
}

static void SART_Render(ocpnDC &dc, wxPen pen, int x, int y, int radius) {
  //    Constants
  int gap = (radius * 12) / 10;
  int pen_width = pen.GetWidth();

  wxPen pen_save = dc.GetPen();

  dc.SetPen(pen);

  wxBrush brush_save = dc.GetBrush();
  wxBrush *ppBrush = wxTheBrushList->FindOrCreateBrush(
      wxColour(0, 0, 0), wxBRUSHSTYLE_TRANSPARENT);
  dc.SetBrush(*ppBrush);

  dc.DrawCircle(x, y, radius);

  if (pen_width > 1) {
    pen_width -= 1;
    pen.SetWidth(pen_width);
  }  // draw cross inside

  dc.DrawLine(x - gap, y - gap, x + gap, y + gap);
  dc.DrawLine(x - gap, y + gap, x + gap, y - gap);

  dc.SetBrush(brush_save);
  dc.SetPen(pen_save);
}

// spherical coordinates is sufficient for visually plotting with relatively
// small distances and about 6x faster than ll_gc_ll
static void spherical_ll_gc_ll(float lat, float lon, float brg, float dist,
                               float *dlat, float *dlon) {
  float angr = brg / 180 * M_PI;
  float latr = lat * M_PI / 180;
  float D = dist / 3443;  // earth radius in nm
  float sD = sinf(D), cD = cosf(D);
  float sy = sinf(latr), cy = cosf(latr);
  float sa = sinf(angr), ca = cosf(angr);

  *dlon = lon + asinf(sa * sD / cy) * 180 / M_PI;
  *dlat = asinf(sy * cD + cy * sD * ca) * 180 / M_PI;
}

//  Global static AIS target rendering metrics
float AIS_scale_factor;
float AIS_nominal_target_size_mm;
float AIS_nominal_icon_size_pixels;
float AIS_pix_factor;
float AIS_user_scale_factor;
double AIS_nominal_line_width_pix;

float AIS_width_interceptbar_base;
float AIS_width_interceptbar_top;
float AIS_intercept_bar_circle_diameter;
float AIS_width_interceptline;
float AIS_width_cogpredictor_base;
float AIS_width_cogpredictor_line;
float AIS_width_target_outline;
float AIS_icon_diameter;
wxFont *AIS_NameFont;

static void AISSetMetrics() {
  AIS_scale_factor = g_current_monitor_dip_px_ratio;
  // Adapt for possible scaled display (Win)
  double DPIscale = 1.0;
  DPIscale = g_Platform->GetDisplayDIPMult(gFrame);

  //  Set the onscreen size of the symbol
  //  Compensate for various display resolutions
  //  Develop empirically, making a "diamond ATON" symbol about 4 mm square

  // By experience, it is found that specifying target size in pixels, then
  // bounding rendered size for high or lo resolution displays, gives the best
  // compromise.

  AIS_nominal_target_size_mm = 30.0 / g_Platform->GetDisplayDPmm();
  // nominal_target_size_mm = gFrame->GetPrimaryCanvas()->GetDisplaySizeMM()
  // / 60.0;

  AIS_nominal_target_size_mm = wxMin(AIS_nominal_target_size_mm, 10.0);
  AIS_nominal_target_size_mm = wxMax(AIS_nominal_target_size_mm, 5.0);

  AIS_nominal_icon_size_pixels =
      wxMax(4.0, g_Platform->GetDisplayDPmm() *
                     AIS_nominal_target_size_mm);  // nominal size, but not
                                                   // less than 4 pixel
  AIS_pix_factor = AIS_nominal_icon_size_pixels /
                   30.0;  // generic A/B icons are 30 units in size

  AIS_scale_factor *= AIS_pix_factor;

  AIS_user_scale_factor = g_ShipScaleFactorExp;
  if (g_ShipScaleFactorExp > 1.0)
    AIS_user_scale_factor = (log(g_ShipScaleFactorExp) + 1.0) *
                            1.2;  // soften the scale factor a bit

  AIS_scale_factor *= AIS_user_scale_factor;

  //  Establish some graphic element line widths dependent on the platform
  //  display resolution
  AIS_nominal_line_width_pix =
      wxMax(2, g_Platform->GetDisplayDPmm() / (4.0 / DPIscale));
  // 0.25 mm nominal, but not less than 2 pixels

  AIS_width_interceptbar_base = 3 * AIS_nominal_line_width_pix;
  AIS_width_interceptbar_top = 1.5 * AIS_nominal_line_width_pix;
  AIS_intercept_bar_circle_diameter = 3.5 * AIS_nominal_line_width_pix;
  AIS_width_interceptline = 2 * AIS_nominal_line_width_pix;
  AIS_width_cogpredictor_base = 3 * AIS_nominal_line_width_pix;
  AIS_width_cogpredictor_line = 1.3 * AIS_nominal_line_width_pix;
  AIS_width_target_outline = 1.4 * AIS_nominal_line_width_pix;
  AIS_icon_diameter = AIS_intercept_bar_circle_diameter * AIS_user_scale_factor * AIS_scale_factor;

  wxFont *font = FontMgr::Get().GetFont(_("AIS Target Name"), 12);
  double scaler = DPIscale;

  AIS_NameFont = FindOrCreateFont_PlugIn(
      font->GetPointSize() / scaler, font->GetFamily(), font->GetStyle(),
      font->GetWeight(), false, font->GetFaceName());
}

static void AISDrawTarget(AisTargetData *td, ocpnDC &dc, ViewPort &vp,
                          ChartCanvas *cp) {
  //      Target data must be valid
  if (NULL == td) return;

  static bool firstTimeUse = true;
  //  First time AIS received
  if (firstTimeUse) {
    g_AisFirstTimeUse = true;
    //   Show Status Bar CPA warning status
    cp->ToggleCPAWarn();
    g_AisFirstTimeUse = false;
    firstTimeUse = false;
  }

  //    Target is lost due to position report time-out, but still in Target List
  if (td->b_lost) return;

  //      Skip anchored/moored (interpreted as low speed) targets if requested
  //      unless the target is NUC or AtoN, in which case it is always
  //      displayed.
  if ((g_bHideMoored) && (td->SOG <= g_ShowMoored_Kts) &&
      (td->NavStatus != NOT_UNDER_COMMAND) &&
      ((td->Class == AIS_CLASS_A) || (td->Class == AIS_CLASS_B)))
    return;

  //      Target data position must have been valid once
  if (!td->b_positionOnceValid) return;

  // And we never draw ownship
  if (td->b_OwnShip) return;

  //    If target's speed is unavailable, use zero for further calculations
  float target_sog = td->SOG;
  if ((td->SOG > 102.2) && !td->b_SarAircraftPosnReport) target_sog = 0.;

  int drawit = 0;
  wxPoint TargetPoint, PredPoint;

  //   Always draw alert targets, even if they are off the screen
  if (td->n_alert_state == AIS_ALERT_SET) {
    drawit++;
  } else {
    //    Is target in Vpoint?
    if (vp.GetBBox().Contains(td->Lat, td->Lon))
      drawit++;  // yep
    else {
      //  If AIS tracks are shown, is the first point of the track on-screen?
      if (1 /*g_bAISShowTracks*/ && td->b_show_track) {
        if (td->m_ptrack.size() > 0) {
          const AISTargetTrackPoint &ptrack_point = td->m_ptrack.front();
          if (vp.GetBBox().Contains(ptrack_point.m_lat, ptrack_point.m_lon))
            drawit++;
        }
      }
    }
  }

  //    Calculate AIS target Position Predictor, using global static variable
  //    for length of vector

  float pred_lat, pred_lon;
  spherical_ll_gc_ll(td->Lat, td->Lon, td->COG,
                     target_sog * g_ShowCOG_Mins / 60., &pred_lat, &pred_lon);

  //    Is predicted point in the VPoint?
  if (vp.GetBBox().Contains(pred_lat, pred_lon))
    drawit++;  // yep
  else {
    LLBBox box;
    box.SetFromSegment(td->Lat, td->Lon, pred_lat, pred_lon);
    // And one more test to catch the case where target COG line crosses the
    // screen, but the target itself and its pred point are both off-screen
    if (!vp.GetBBox().IntersectOut(box)) drawit++;
  }

  //    Do the draw if conditions indicate
  if (!drawit) return;

  GetCanvasPointPix(vp, cp, td->Lat, td->Lon, &TargetPoint);
  GetCanvasPointPix(vp, cp, pred_lat, pred_lon, &PredPoint);

  bool b_hdgValid = true;

  float theta = (float)-PI / 2.;
  //    If the target reported a valid HDG, then use it for icon
  if ((int)(td->HDG) != 511) {
    theta = ((td->HDG - 90) * PI / 180.) + vp.rotation;
  } else {
    b_hdgValid = false;  // tentative judgement

    if (!g_bInlandEcdis) {
      // question: why can we not compute similar to above using COG instead of
      // HDG?
      //  Calculate the relative angle for this chart orientation
      //    Use a 100 pixel vector to calculate angle
      float angle_distance_nm = (100. / vp.view_scale_ppm) / 1852.;
      float angle_lat, angle_lon;
      spherical_ll_gc_ll(td->Lat, td->Lon, td->COG, angle_distance_nm,
                         &angle_lat, &angle_lon);

      wxPoint AnglePoint;
      GetCanvasPointPix(vp, cp, angle_lat, angle_lon, &AnglePoint);

      if (abs(AnglePoint.x - TargetPoint.x) > 0) {
        if (target_sog > g_SOGminCOG_kts) {
          theta = atan2f((double)(AnglePoint.y - TargetPoint.y),
                         (double)(AnglePoint.x - TargetPoint.x));
          b_hdgValid = true;
        } else
          theta = (float)-PI / 2.;
      } else {
        if (AnglePoint.y > TargetPoint.y)
          theta = (float)PI / 2.;  // valid COG 180
        else {
          theta = (float)-PI /
                  2.;  //  valid COG 000 or speed is too low to resolve course
          if (td->SOG >= g_SOGminCOG_kts)  //  valid COG 000 or speed is too
                                           //  low to resolve course
            b_hdgValid = true;
        }
      }
    }
  }

  // only need to compute this once;
  float sin_theta = sinf(theta), cos_theta = cosf(theta);

  wxDash dash_long[2];
  dash_long[0] = (int)(1.0 * gFrame->GetPrimaryCanvas()
                                 ->GetPixPerMM());  // Long dash  <---------+
  dash_long[1] =
      (int)(0.5 * gFrame->GetPrimaryCanvas()->GetPixPerMM());  // Short gap |

  int targetscale = 100;
  int idxCC = 0;
  if (cp != NULL) {
    idxCC = cp->m_canvasIndex;

    if (idxCC > AIS_TARGETDATA_MAX_CANVAS - 1)
      return;  // If more then n canvasses do not draw AIS anymore as we are
               // running out of array index
    if (cp->GetAttenAIS()) {
      if (td->NavStatus <= 15) {  // NavStatus > 15 is AtoN, and we don want
                                  // AtoN being counted for attenuation
        // with one tick per second targets can slink from 100 to 50 in abt 25
        // seconds
        if (td->importance < AISImportanceSwitchPoint)
          targetscale = td->last_scale[idxCC] - 2;
        // growing from 50 till 100% goes faster in 10 seconds
        if (td->importance > AISImportanceSwitchPoint)
          targetscale = td->last_scale[idxCC] + 5;
        if (targetscale > 100) targetscale = 100;
        if (targetscale < 50) targetscale = 50;
        td->last_scale[idxCC] = targetscale;
      }
    }
  }

  //  Draw the icon rotated to the COG
  wxPoint ais_real_size[6];
  bool bcan_draw_size = true;
  if (g_bDrawAISSize) {
    if (td->DimA + td->DimB == 0 || td->DimC + td->DimD == 0) {
      bcan_draw_size = false;
    } else {
      double ref_lat, ref_lon;
      ll_gc_ll(td->Lat, td->Lon, 0, 100. / 1852., &ref_lat, &ref_lon);
      wxPoint2DDouble b_point = vp.GetDoublePixFromLL(td->Lat, td->Lon);
      wxPoint2DDouble r_point = vp.GetDoublePixFromLL(ref_lat, ref_lon);
      double ppm = r_point.GetDistance(b_point) / 100.;
      double offwid = (td->DimC + td->DimD) * ppm * 0.25;
      double offlen = (td->DimA + td->DimB) * ppm * 0.15;
      ais_real_size[0].x = -td->DimD * ppm;
      ais_real_size[0].y = -td->DimB * ppm;
      ais_real_size[1].x = -td->DimD * ppm;
      ais_real_size[1].y = td->DimA * ppm - offlen;
      ais_real_size[2].x = -td->DimD * ppm + offwid;
      ais_real_size[2].y = td->DimA * ppm;
      ais_real_size[3].x = td->DimC * ppm - offwid;
      ais_real_size[3].y = td->DimA * ppm;
      ais_real_size[4].x = td->DimC * ppm;
      ais_real_size[4].y = td->DimA * ppm - offlen;
      ais_real_size[5].x = td->DimC * ppm;
      ais_real_size[5].y = -td->DimB * ppm;

      if (ais_real_size[4].x - ais_real_size[0].x < 16 ||
          ais_real_size[2].y - ais_real_size[0].y < 30)
        bcan_draw_size = false;  // drawing too small does not make sense
      else {
        bcan_draw_size = true;
        transrot_pts(6, ais_real_size, sin_theta, cos_theta);
      }
    }
  }

  wxPoint *iconPoints;
  int nPoints;
  wxPoint ais_quad_icon[4] = {wxPoint(-8, -6), wxPoint(0, 24), wxPoint(8, -6),
                              wxPoint(0, -6)};
  wxPoint ais_octo_icon[8] = {wxPoint(4, 8),  wxPoint(8, 4),   wxPoint(8, -4),
                              wxPoint(4, -8), wxPoint(-4, -8), wxPoint(-8, -4),
                              wxPoint(-8, 4), wxPoint(-4, 8)};

  if (!g_bInlandEcdis) {
    // to speed up we only calculate scale when not max or minimal
    if (targetscale == 50) {
      ais_quad_icon[0] = wxPoint(-4, -3);
      ais_quad_icon[1] = wxPoint(0, 12);
      ais_quad_icon[2] = wxPoint(4, -3);
      ais_quad_icon[3] = wxPoint(0, -3);
    } else if (targetscale != 100) {
      ais_quad_icon[0] =
          wxPoint((int)-8 * targetscale / 100, (int)-6 * targetscale / 100);
      ais_quad_icon[1] = wxPoint(0, (int)24 * targetscale / 100);
      ais_quad_icon[2] =
          wxPoint((int)8 * targetscale / 100, (int)-6 * targetscale / 100);
      ais_quad_icon[3] = wxPoint(0, (int)-6 * targetscale / 100);
    }

    //   If this is an AIS Class B target, so symbolize it differently
    if (td->Class == AIS_CLASS_B) ais_quad_icon[3].y = 0;

    if ((td->Class == AIS_GPSG_BUDDY) || (td->b_isFollower)) {
      ais_quad_icon[0] = wxPoint(-5, -12);
      ais_quad_icon[1] = wxPoint(-3, 12);
      ais_quad_icon[2] = wxPoint(3, 12);
      ais_quad_icon[3] = wxPoint(5, -12);
    } else if (td->Class == AIS_DSC) {
      ais_quad_icon[0].y = 0;
      ais_quad_icon[1].y = 8;
      ais_quad_icon[2].y = 0;
      ais_quad_icon[3].y = -8;
    } else if (td->Class == AIS_APRS) {
      ais_quad_icon[0] = wxPoint(-8, -8);
      ais_quad_icon[1] = wxPoint(-8, 8);
      ais_quad_icon[2] = wxPoint(8, 8);
      ais_quad_icon[3] = wxPoint(8, -8);
    }

    transrot_pts(4, ais_quad_icon, sin_theta, cos_theta);

    nPoints = 4;
    iconPoints = ais_quad_icon;

  } else {  // iENC
    if (b_hdgValid) {
      transrot_pts(4, ais_quad_icon, sin_theta, cos_theta);
      nPoints = 4;
      iconPoints = ais_quad_icon;
    } else {
      nPoints = 8;
      iconPoints = ais_octo_icon;
    }
  }

  wxColour UBLCK = GetGlobalColor(_T ( "UBLCK" ));
  dc.SetPen(wxPen(UBLCK));

  // Default color is green
  wxColour UINFG = GetGlobalColor(_T ( "UINFG" ));
  wxBrush target_brush = wxBrush(UINFG);

  // Euro Inland targets render slightly differently, unless in InlandENC mode
  if (td->b_isEuroInland && !g_bInlandEcdis)
    target_brush = wxBrush(GetGlobalColor(_T ( "TEAL1" )));

  // Target name comes from cache
  if (td->b_nameFromCache)
    target_brush = wxBrush(GetGlobalColor(_T ( "GREEN5" )));

  // and....
  wxColour URED = GetGlobalColor(_T ( "URED" ));
  if (!td->b_nameValid) target_brush = wxBrush(GetGlobalColor(_T ( "CHYLW" )));

  if ((td->Class == AIS_DSC) &&
      ((td->ShipType == 12) || (td->ShipType == 16)))  // distress(relayed)
    target_brush = wxBrush(URED);

  if (td->b_SarAircraftPosnReport) target_brush = wxBrush(UINFG);

  if ((td->n_alert_state == AIS_ALERT_SET) && (td->bCPA_Valid))
    target_brush = wxBrush(URED);

  if ((td->n_alert_state == AIS_ALERT_NO_DIALOG_SET) && (td->bCPA_Valid) &&
      (!td->b_isFollower))
    target_brush = wxBrush(URED);

  if (td->b_positionDoubtful)
    target_brush = wxBrush(GetGlobalColor(_T ( "UINFF" )));

  wxPen target_outline_pen(UBLCK, AIS_width_target_outline);

  //    Check for alarms here, maintained by AIS class timer tick
  if (((td->n_alert_state == AIS_ALERT_SET) && (td->bCPA_Valid)) ||
      (td->b_show_AIS_CPA && (td->bCPA_Valid))) {
    //  Calculate the point of CPA for target
    double tcpa_lat, tcpa_lon;
    ll_gc_ll(td->Lat, td->Lon, td->COG, target_sog * td->TCPA / 60., &tcpa_lat,
             &tcpa_lon);
    wxPoint tCPAPoint;
    wxPoint TPoint = TargetPoint;
    GetCanvasPointPix(vp, cp, tcpa_lat, tcpa_lon, &tCPAPoint);

    //  Draw the intercept line from target
    ClipResult res = cohen_sutherland_line_clip_i(
        &TPoint.x, &TPoint.y, &tCPAPoint.x, &tCPAPoint.y, 0, vp.pix_width, 0,
        vp.pix_height);

    if (res != Invisible) {
      wxPen ppPen2(URED, AIS_width_cogpredictor_line, wxPENSTYLE_USER_DASH);
      ppPen2.SetDashes(2, dash_long);
      dc.SetPen(ppPen2);

      dc.StrokeLine(TPoint.x, TPoint.y, tCPAPoint.x, tCPAPoint.y);
    }

    //  Calculate the point of CPA for ownship
    double ocpa_lat, ocpa_lon;

    //  Detect and handle the case where ownship COG is undefined....
    if (std::isnan(gCog) || std::isnan(gSog)) {
      ocpa_lat = gLat;
      ocpa_lon = gLon;
    } else {
      ll_gc_ll(gLat, gLon, gCog, gSog * td->TCPA / 60., &ocpa_lat, &ocpa_lon);
    }

    wxPoint oCPAPoint;

    GetCanvasPointPix(vp, cp, ocpa_lat, ocpa_lon, &oCPAPoint);
    GetCanvasPointPix(vp, cp, tcpa_lat, tcpa_lon, &tCPAPoint);

    //        Save a copy of these unclipped points
    wxPoint oCPAPoint_unclipped = oCPAPoint;
    wxPoint tCPAPoint_unclipped = tCPAPoint;

    //  Draw a line from target CPA point to ownship CPA point
    ClipResult ores = cohen_sutherland_line_clip_i(
        &tCPAPoint.x, &tCPAPoint.y, &oCPAPoint.x, &oCPAPoint.y, 0, vp.pix_width,
        0, vp.pix_height);

    if (ores != Invisible) {
      wxColour yellow = GetGlobalColor(_T ( "YELO1" ));
      dc.SetPen(wxPen(yellow, AIS_width_interceptbar_base));
      dc.StrokeLine(tCPAPoint.x, tCPAPoint.y, oCPAPoint.x, oCPAPoint.y);

      wxPen ppPen2(URED, AIS_width_interceptbar_top, wxPENSTYLE_USER_DASH);
      ppPen2.SetDashes(2, dash_long);
      dc.SetPen(ppPen2);
      dc.StrokeLine(tCPAPoint.x, tCPAPoint.y, oCPAPoint.x, oCPAPoint.y);

      //        Draw little circles at the ends of the CPA alert line
      wxBrush br(GetGlobalColor(_T ( "BLUE3" )));
      dc.SetBrush(br);
      dc.SetPen(wxPen(UBLCK, AIS_width_target_outline));

      //  Using the true ends, not the clipped ends
      dc.StrokeCircle(
          tCPAPoint_unclipped.x, tCPAPoint_unclipped.y,
          AIS_intercept_bar_circle_diameter * AIS_user_scale_factor);
      dc.StrokeCircle(
          oCPAPoint_unclipped.x, oCPAPoint_unclipped.y,
          AIS_intercept_bar_circle_diameter * AIS_user_scale_factor);
    }

    // Draw the intercept line from ownship
    wxPoint oShipPoint;
    GetCanvasPointPix(vp, cp, gLat, gLon, &oShipPoint);
    oCPAPoint = oCPAPoint_unclipped;  // recover the unclipped point

    ClipResult ownres = cohen_sutherland_line_clip_i(
        &oShipPoint.x, &oShipPoint.y, &oCPAPoint.x, &oCPAPoint.y, 0,
        vp.pix_width, 0, vp.pix_height);

    if (ownres != Invisible) {
      wxPen ppPen2(URED, AIS_width_interceptline, wxPENSTYLE_USER_DASH);
      ppPen2.SetDashes(2, dash_long);
      dc.SetPen(ppPen2);

      dc.StrokeLine(oShipPoint.x, oShipPoint.y, oCPAPoint.x, oCPAPoint.y);
    }  // TR : till here

    dc.SetPen(wxPen(UBLCK));
    dc.SetBrush(wxBrush(URED));
  }

  //  Highlight the AIS target symbol if an alert dialog is currently open for
  //  it
  if (cp != NULL) {
    auto alert_dlg_active =
        dynamic_cast<AISTargetAlertDialog *>(g_pais_alert_dialog_active);
    if (alert_dlg_active && alert_dlg_active->IsShown() && cp) {
      if (alert_dlg_active->Get_Dialog_MMSI() == td->MMSI)
        cp->JaggyCircle(dc, wxPen(URED, 2), TargetPoint.x, TargetPoint.y, 100);
    }
  }

  //  Highlight the AIS target symbol if a query dialog is currently open for it
  if (g_pais_query_dialog_active && g_pais_query_dialog_active->IsShown()) {
    if (g_pais_query_dialog_active->GetMMSI() == td->MMSI)
      TargetFrame(dc, wxPen(UBLCK, 2), TargetPoint.x, TargetPoint.y, 25);
  }

  //       Render the COG line if the speed is greater than moored speed defined
  //       by ais options dialog
  if ((g_bShowCOG) && (target_sog > g_SOGminCOG_kts) && td->b_active) {
    int pixx = TargetPoint.x;
    int pixy = TargetPoint.y;
    int pixx1 = PredPoint.x;
    int pixy1 = PredPoint.y;

    //  Don't draw the COG line  and predictor point if zoomed far out.... or if
    //  target lost/inactive
    float l = sqrtf(powf((float)(PredPoint.x - TargetPoint.x), 2) +
                    powf((float)(PredPoint.y - TargetPoint.y), 2));

    if (l > 24) {
      ClipResult res = cohen_sutherland_line_clip_i(
          &pixx, &pixy, &pixx1, &pixy1, 0, vp.pix_width, 0, vp.pix_height);

      if (res != Invisible) {
        //    Draw a wider coloured line
        if (targetscale >= 75) {
          wxPen wide_pen(target_brush.GetColour(), AIS_width_cogpredictor_base);
          dc.SetPen(wide_pen);
          dc.StrokeLine(pixx, pixy, pixx1, pixy1);
        }

        if (AIS_width_cogpredictor_base > 1) {
          //    Draw narrow black line
          wxPen narrow_pen(UBLCK, AIS_width_cogpredictor_line);
          if (targetscale < 75) {
            narrow_pen.SetWidth(1);
            narrow_pen.SetStyle(wxPENSTYLE_USER_DASH);
            wxDash dash_dot[2];
            dash_dot[0] = 2;
            dash_dot[1] = 2;
            narrow_pen.SetDashes(2, dash_dot);
          }
          dc.SetPen(narrow_pen);
          dc.StrokeLine(pixx, pixy, pixx1, pixy1);
        }

        if (dc.GetDC()) {
          dc.SetBrush(target_brush);
          dc.StrokeCircle(PredPoint.x, PredPoint.y, 5 * targetscale / 100);
        } else {
#ifdef ocpnUSE_GL

// #ifndef USE_ANDROID_GLES2
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)

          glPushMatrix();
          glTranslated(PredPoint.x, PredPoint.y, 0);
          glScalef(AIS_scale_factor, AIS_scale_factor, AIS_scale_factor);
          // draw circle
          float points[] = {0.0f,       5.0f,       2.5f,  4.330127f, 4.330127f,
                            2.5f,       5.0f,       0,     4.330127f, -2.5f,
                            2.5f,       -4.330127f, 0,     -5.1f,     -2.5f,
                            -4.330127f, -4.330127f, -2.5f, -5.0f,     0,
                            -4.330127f, 2.5f,       -2.5f, 4.330127f, 0,
                            5.0f};
          if (targetscale <= 75) {
            for (unsigned int i = 0; i < (sizeof points) / (sizeof *points);
                 i++)
              points[i] = points[i] / 2;
          }

          wxColour c = target_brush.GetColour();
          glColor3ub(c.Red(), c.Green(), c.Blue());

          glBegin(GL_TRIANGLE_FAN);
          for (unsigned int i = 0; i < (sizeof points) / (sizeof *points);
               i += 2)
            glVertex2i(points[i], points[i + 1]);
          glEnd();

          glColor3ub(0, 0, 0);
          glLineWidth(AIS_width_target_outline);
          glBegin(GL_LINE_LOOP);
          for (unsigned int i = 0; i < (sizeof points) / (sizeof *points);
               i += 2)
            glVertex2i(points[i], points[i + 1]);
          glEnd();
          glPopMatrix();
#else

          dc.SetBrush(target_brush);
          dc.StrokeCircle(PredPoint.x, PredPoint.y,
                          AIS_intercept_bar_circle_diameter *
                              AIS_user_scale_factor * targetscale / 100);
#endif
#endif
        }
      }

      //      Draw RateOfTurn Vector
      if ((td->ROTAIS != 0) && (td->ROTAIS != -128) && (!g_bShowScaled)) {
        float cog_angle = td->COG * PI / 180.;

        float theta2 = theta;  // ownship drawn angle
        if (td->SOG >= g_SOGminCOG_kts)
          theta2 = cog_angle - (PI / 2);  // actual cog angle

        float nv = 10;
        if (td->ROTAIS > 0)
          theta2 += (float)PI / 2;
        else
          theta2 -= (float)PI / 2;

        int xrot = (int)round(pixx1 + (nv * cosf(theta2)));
        int yrot = (int)round(pixy1 + (nv * sinf(theta2)));
        dc.StrokeLine(pixx1, pixy1, xrot, yrot);
      }
    }
  }

  //        Actually Draw the target
  if (td->Class == AIS_ARPA) {
    wxPen target_pen(UBLCK, 2);

    dc.SetPen(target_pen);
    dc.SetBrush(target_brush);
    dc.StrokeCircle(TargetPoint.x, TargetPoint.y, 1.8 * AIS_icon_diameter);

    dc.StrokeCircle(TargetPoint.x, TargetPoint.y, 1);
    //        Draw the inactive cross-out line
    if (!td->b_active) {
      dc.SetPen(wxPen(UBLCK, 2));
      dc.StrokeLine(TargetPoint.x - 14, TargetPoint.y, TargetPoint.x + 14,
                    TargetPoint.y);
      dc.SetPen(wxPen(UBLCK, 1));
    }

  } else if (td->Class == AIS_METEO) {  // Meteorologic
    wxPen met(UBLCK, (wxMax(target_outline_pen.GetWidth(), 2.5)));
    dc.SetPen(met);
    dc.SetBrush(wxBrush(UBLCK, wxBRUSHSTYLE_TRANSPARENT));
    double met_radius = 1.8 * AIS_icon_diameter;
    dc.StrokeCircle(TargetPoint.x, TargetPoint.y, met_radius);

    /* Inscribed "W" in the circle. */
    dc.SetPen(wxPen(wxMax(target_outline_pen.GetWidth(), 1)));
    // Left part
    dc.StrokeLine(TargetPoint.x, TargetPoint.y - met_radius / 4,
                  TargetPoint.x - met_radius / 3,
                  TargetPoint.y + met_radius / 2);
    dc.StrokeLine(
        TargetPoint.x - met_radius / 3, TargetPoint.y + met_radius / 2,
        TargetPoint.x - met_radius / 2, TargetPoint.y - met_radius / 2);
    // Right part
    dc.StrokeLine(TargetPoint.x, TargetPoint.y - met_radius / 4,
                  TargetPoint.x + met_radius / 3,
                  TargetPoint.y + met_radius / 2);
    dc.StrokeLine(
        TargetPoint.x + met_radius / 3, TargetPoint.y + met_radius / 2,
        TargetPoint.x + met_radius / 2, TargetPoint.y - met_radius / 2);

  } else if (td->Class == AIS_ATON) {  // Aid to Navigation
    AtoN_Diamond(dc, wxPen(UBLCK, AIS_width_target_outline), TargetPoint.x, TargetPoint.y, AIS_icon_diameter * 1.5, td);
  } else if (td->Class == AIS_BASE) {  // Base Station
    Base_Square(dc, wxPen(UBLCK, AIS_width_target_outline), TargetPoint.x, TargetPoint.y, AIS_icon_diameter);
  } else if (td->Class == AIS_SART) {  // SART Target
    if (td->NavStatus == 14)           // active
      SART_Render(dc, wxPen(URED, AIS_width_target_outline), TargetPoint.x, TargetPoint.y, AIS_icon_diameter);
    else
      SART_Render(dc, wxPen(GetGlobalColor(_T ( "UGREN" )), AIS_width_target_outline), TargetPoint.x,
                  TargetPoint.y, AIS_icon_diameter);

  } else if (td->b_SarAircraftPosnReport) {
    int airtype = (td->MMSI % 1000) / 100;  // xxxyyy5zz >> helicopter
    int ar = airtype == 5 ? 15 : 9;         // array size
    wxPoint SarIcon[15];
    wxPoint SarRot[15];
    double scaleplus = 1.4;
    if (airtype == 5) {
      SarIcon[0] = wxPoint(0, 9) * AIS_scale_factor * scaleplus;
      SarIcon[1] = wxPoint(1, 1) * AIS_scale_factor * scaleplus;
      SarIcon[2] = wxPoint(2, 1) * AIS_scale_factor * scaleplus;
      SarIcon[3] = wxPoint(9, 8) * AIS_scale_factor * scaleplus;
      SarIcon[4] = wxPoint(9, 7) * AIS_scale_factor * scaleplus;
      SarIcon[5] = wxPoint(3, 0) * AIS_scale_factor * scaleplus;
      SarIcon[6] = wxPoint(3, -5) * AIS_scale_factor * scaleplus;
      SarIcon[7] = wxPoint(9, -12) * AIS_scale_factor * scaleplus;
      SarIcon[8] = wxPoint(9, -13) * AIS_scale_factor * scaleplus;
      SarIcon[9] = wxPoint(2, -5) * AIS_scale_factor * scaleplus;
      SarIcon[10] = wxPoint(1, -15) * AIS_scale_factor * scaleplus;
      SarIcon[11] = wxPoint(3, -16) * AIS_scale_factor * scaleplus;
      SarIcon[12] = wxPoint(4, -18) * AIS_scale_factor * scaleplus;
      SarIcon[13] = wxPoint(1, -18) * AIS_scale_factor * scaleplus;
      SarIcon[14] = wxPoint(0, -19) * AIS_scale_factor * scaleplus;
    } else {
      SarIcon[0] = wxPoint(0, 12) * AIS_scale_factor;
      SarIcon[1] = wxPoint(4, 2) * AIS_scale_factor;
      SarIcon[2] = wxPoint(16, -2) * AIS_scale_factor;
      SarIcon[3] = wxPoint(16, -8) * AIS_scale_factor;
      SarIcon[4] = wxPoint(4, -8) * AIS_scale_factor;
      SarIcon[5] = wxPoint(3, -16) * AIS_scale_factor;
      SarIcon[6] = wxPoint(10, -18) * AIS_scale_factor;
      SarIcon[7] = wxPoint(10, -22) * AIS_scale_factor;
      SarIcon[8] = wxPoint(0, -22) * AIS_scale_factor;
    }

    if (airtype == 5) {  // helicopter
      // Draw icon as two halves
      //  First half

      for (int i = 0; i < ar; i++) SarRot[i] = SarIcon[i];
      transrot_pts(ar, SarRot, sin_theta, cos_theta);

      wxPen tri_pen(target_brush.GetColour(), 1);
      dc.SetPen(tri_pen);
      dc.SetBrush(target_brush);

      // Manual tesselation
      int mappings[14][3] = {
          {0, 1, 10},   {0, 10, 14},  {1, 2, 9},   {1, 9, 10}, {10, 13, 14},
          {10, 11, 13}, {11, 12, 13}, {1, 14, 10}, {2, 5, 9},  {5, 6, 9},
          {2, 3, 5},    {3, 4, 5},    {6, 7, 8},   {6, 9, 8}};

      int nmap = 14;
      for (int i = 0; i < nmap; i++) {
        wxPoint ais_tri_icon[3];
        for (int j = 0; j < 3; j++) ais_tri_icon[j] = SarRot[mappings[i][j]];
        dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
      }

      dc.SetPen(target_outline_pen);
      dc.SetBrush(wxBrush(UBLCK, wxBRUSHSTYLE_TRANSPARENT));
      dc.StrokePolygon(ar, SarRot, TargetPoint.x, TargetPoint.y);

      // second half

      for (int i = 0; i < ar; i++)
        SarRot[i] =
            wxPoint(-SarIcon[i].x, SarIcon[i].y);  // mirror the icon (x -> -x)

      transrot_pts(ar, SarRot, sin_theta, cos_theta);

      dc.SetPen(tri_pen);
      dc.SetBrush(target_brush);

      for (int i = 0; i < nmap; i++) {
        wxPoint ais_tri_icon[3];
        for (int j = 0; j < 3; j++) ais_tri_icon[j] = SarRot[mappings[i][j]];
        dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
      }

      dc.SetPen(target_outline_pen);
      dc.SetBrush(wxBrush(UBLCK, wxBRUSHSTYLE_TRANSPARENT));
      dc.StrokePolygon(ar, SarRot, TargetPoint.x, TargetPoint.y);
    } else {
      // Draw icon as two halves
      //  First half

      for (int i = 0; i < ar; i++) SarRot[i] = SarIcon[i];
      transrot_pts(ar, SarRot, sin_theta, cos_theta);

      wxPen tri_pen(target_brush.GetColour(), 1);
      dc.SetPen(tri_pen);
      dc.SetBrush(target_brush);

      // Manual tesselation
      int mappings[7][3] = {{0, 1, 4}, {1, 2, 3}, {1, 3, 4}, {0, 4, 5},
                            {0, 5, 8}, {5, 6, 7}, {5, 7, 8}};
      for (int i = 0; i < 7; i++) {
        wxPoint ais_tri_icon[3];
        for (int j = 0; j < 3; j++) ais_tri_icon[j] = SarRot[mappings[i][j]];
        dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
      }

      dc.SetPen(target_outline_pen);
      dc.SetBrush(wxBrush(UBLCK, wxBRUSHSTYLE_TRANSPARENT));
      dc.StrokePolygon(ar, SarRot, TargetPoint.x, TargetPoint.y);

      // second half

      for (int i = 0; i < ar; i++)
        SarRot[i] =
            wxPoint(-SarIcon[i].x, SarIcon[i].y);  // mirror the icon (x -> -x)

      transrot_pts(ar, SarRot, sin_theta, cos_theta);

      dc.SetPen(tri_pen);
      dc.SetBrush(target_brush);

      for (int i = 0; i < 7; i++) {
        wxPoint ais_tri_icon[3];
        for (int j = 0; j < 3; j++) ais_tri_icon[j] = SarRot[mappings[i][j]];
        dc.StrokePolygon(3, ais_tri_icon, TargetPoint.x, TargetPoint.y);
      }

      dc.SetPen(target_outline_pen);
      dc.SetBrush(wxBrush(UBLCK, wxBRUSHSTYLE_TRANSPARENT));
      dc.StrokePolygon(ar, SarRot, TargetPoint.x, TargetPoint.y);
    }

    //        Draw the inactive cross-out line
    if (!td->b_active) {
      dc.SetPen(wxPen(UBLCK, 3));
      dc.StrokeLine(TargetPoint.x - 16, TargetPoint.y, TargetPoint.x + 16,
                    TargetPoint.y);
    }

  } else {  // ship class A or B or a Buddy or DSC
    wxPen target_pen(UBLCK, 1);
    dc.SetPen(target_pen);

    wxPoint Point = TargetPoint;
    if (g_bDrawAISRealtime &&
        (td->Class == AIS_CLASS_A || td->Class == AIS_CLASS_B) &&
        td->SOG > g_AIS_RealtPred_Kts && td->SOG < 102.2) {
      wxDateTime now = wxDateTime::Now();
      now.MakeGMT();
      int target_age = now.GetTicks() - td->PositionReportTicks;

      float lat, lon;
      spherical_ll_gc_ll(td->Lat, td->Lon, td->COG,
                         td->SOG * target_age / 3600.0, &lat, &lon);

      GetCanvasPointPix(vp, cp, lat, lon, &Point);

      wxBrush realtime_brush = wxBrush(GetGlobalColor("GREY1"));
      dc.SetBrush(realtime_brush);
      dc.StrokePolygon(nPoints, iconPoints, Point.x, Point.y, AIS_scale_factor);
    }
    dc.SetBrush(target_brush);

    if (dc.GetDC()) {
      dc.StrokePolygon(nPoints, iconPoints, TargetPoint.x, TargetPoint.y,
                       AIS_scale_factor);
    } else {
#ifdef ocpnUSE_GL
// #ifndef USE_ANDROID_GLES2
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)

      wxColour c = target_brush.GetColour();
      glColor3ub(c.Red(), c.Green(), c.Blue());

      glPushMatrix();
      glTranslated(TargetPoint.x, TargetPoint.y, 0);
      glScalef(AIS_scale_factor, AIS_scale_factor, AIS_scale_factor);

      glBegin(GL_TRIANGLE_FAN);

      if (nPoints == 4) {
        glVertex2i(ais_quad_icon[3].x, ais_quad_icon[3].y);
        glVertex2i(ais_quad_icon[0].x, ais_quad_icon[0].y);
        glVertex2i(ais_quad_icon[1].x, ais_quad_icon[1].y);
        glVertex2i(ais_quad_icon[2].x, ais_quad_icon[2].y);
      } else {
        for (int i = 0; i < 8; i++) {
          glVertex2i(iconPoints[i].x, iconPoints[i].y);
        }
      }

      glEnd();
      glLineWidth(AIS_width_target_outline);

      glColor3ub(UBLCK.Red(), UBLCK.Green(), UBLCK.Blue());

      glBegin(GL_LINE_LOOP);
      for (int i = 0; i < nPoints; i++)
        glVertex2i(iconPoints[i].x, iconPoints[i].y);
      glEnd();
      glPopMatrix();

#else
      dc.SetPen(target_outline_pen);
      dc.DrawPolygon(nPoints, iconPoints, TargetPoint.x, TargetPoint.y,
                     AIS_scale_factor);
#endif
#endif
    }
    // Draw stroke "inverted v" for GPS Follower
    if (td->b_isFollower) {
      wxPoint ais_follow_stroke[3];
      ais_follow_stroke[0] = wxPoint(-3, -20) * AIS_scale_factor;
      ais_follow_stroke[1] = wxPoint(0, 0) * AIS_scale_factor;
      ais_follow_stroke[2] = wxPoint(3, -20) * AIS_scale_factor;

      transrot_pts(3, ais_follow_stroke, sin_theta, cos_theta);

      int penWidth = wxMax(target_outline_pen.GetWidth(), 2);
      dc.SetPen(wxPen(UBLCK, penWidth));
      dc.StrokeLine(ais_follow_stroke[0].x + TargetPoint.x,
                    ais_follow_stroke[0].y + TargetPoint.y,
                    ais_follow_stroke[1].x + TargetPoint.x,
                    ais_follow_stroke[1].y + TargetPoint.y);
      dc.StrokeLine(ais_follow_stroke[1].x + TargetPoint.x,
                    ais_follow_stroke[1].y + TargetPoint.y,
                    ais_follow_stroke[2].x + TargetPoint.x,
                    ais_follow_stroke[2].y + TargetPoint.y);
    }

    if (g_bDrawAISSize && bcan_draw_size) {
      dc.SetPen(target_outline_pen);
      dc.SetBrush(wxBrush(UBLCK, wxBRUSHSTYLE_TRANSPARENT));
      if (!g_bInlandEcdis) {
        dc.StrokePolygon(6, ais_real_size, TargetPoint.x, TargetPoint.y, 1.0);
      } else {
        if (b_hdgValid) {
          dc.StrokePolygon(6, ais_real_size, TargetPoint.x, TargetPoint.y, 1.0);
        }
      }
    }

    dc.SetBrush(wxBrush(GetGlobalColor(_T ( "SHIPS" ))));
    int navstatus = td->NavStatus;

    // HSC usually have correct ShipType but navstatus == 0...
    // Class B can have (HSC)ShipType but never navstatus.
    if (((td->ShipType >= 40) && (td->ShipType < 50)) &&
        (navstatus == UNDERWAY_USING_ENGINE || td->Class == AIS_CLASS_B))
      navstatus = HSC;

    if (targetscale > 90) {
      switch (navstatus) {
        case MOORED:
        case AT_ANCHOR: {
          dc.StrokeCircle(TargetPoint.x, TargetPoint.y, 4 * AIS_scale_factor);
          break;
        }
        case RESTRICTED_MANOEUVRABILITY: {
          wxPoint diamond[4];
          diamond[0] = wxPoint(4, 0) * AIS_scale_factor;
          diamond[1] = wxPoint(0, -6) * AIS_scale_factor;
          diamond[2] = wxPoint(-4, 0) * AIS_scale_factor;
          diamond[3] = wxPoint(0, 6) * AIS_scale_factor;
          dc.StrokePolygon(4, diamond, TargetPoint.x,
                           TargetPoint.y - (11 * AIS_scale_factor));
          dc.StrokeCircle(TargetPoint.x, TargetPoint.y, 4 * AIS_scale_factor);
          dc.StrokeCircle(TargetPoint.x,
                          TargetPoint.y - (22 * AIS_scale_factor),
                          4 * AIS_scale_factor);
          break;
          break;
        }
        case CONSTRAINED_BY_DRAFT: {
          wxPoint can[4] = {wxPoint(-3, 0) * AIS_scale_factor,
                            wxPoint(3, 0) * AIS_scale_factor,
                            wxPoint(3, -16) * AIS_scale_factor,
                            wxPoint(-3, -16) * AIS_scale_factor};
          dc.StrokePolygon(4, can, TargetPoint.x, TargetPoint.y);
          break;
        }
        case NOT_UNDER_COMMAND: {
          dc.StrokeCircle(TargetPoint.x, TargetPoint.y, 4 * AIS_scale_factor);
          dc.StrokeCircle(TargetPoint.x, TargetPoint.y - 9,
                          4 * AIS_scale_factor);
          break;
        }
        case FISHING: {
          wxPoint tri[3];
          tri[0] = wxPoint(-4, 0) * AIS_scale_factor;
          tri[1] = wxPoint(4, 0) * AIS_scale_factor;
          tri[2] = wxPoint(0, -9) * AIS_scale_factor;
          dc.StrokePolygon(3, tri, TargetPoint.x, TargetPoint.y);
          tri[0] = wxPoint(0, -9) * AIS_scale_factor;
          tri[1] = wxPoint(4, -18) * AIS_scale_factor;
          tri[2] = wxPoint(-4, -18) * AIS_scale_factor;
          dc.StrokePolygon(3, tri, TargetPoint.x, TargetPoint.y);
          break;
        }
        case AGROUND: {
          dc.StrokeCircle(TargetPoint.x, TargetPoint.y, 4 * AIS_scale_factor);
          dc.StrokeCircle(TargetPoint.x, TargetPoint.y - 9,
                          4 * AIS_scale_factor);
          dc.StrokeCircle(TargetPoint.x, TargetPoint.y - 18,
                          4 * AIS_scale_factor);
          break;
        }
        case HSC:
        case WIG: {
          dc.SetBrush(target_brush);

          wxPoint arrow1[3] = {wxPoint(-4, 20) * AIS_scale_factor,
                               wxPoint(0, 27) * AIS_scale_factor,
                               wxPoint(4, 20) * AIS_scale_factor};
          transrot_pts(3, arrow1, sin_theta, cos_theta, TargetPoint);
          dc.StrokePolygon(3, arrow1);

          wxPoint arrow2[3] = {wxPoint(-4, 27) * AIS_scale_factor,
                               wxPoint(0, 34) * AIS_scale_factor,
                               wxPoint(4, 27) * AIS_scale_factor};
          transrot_pts(3, arrow2, sin_theta, cos_theta, TargetPoint);
          dc.StrokePolygon(3, arrow2);
          break;
        }
      }
    }  // end if (targetscale > 75)

    //        Draw the inactive cross-out line
    if (!td->b_active) {
      wxPoint p1 = transrot(wxPoint((int)-14 * targetscale / 100, 0), sin_theta,
                            cos_theta, TargetPoint);
      wxPoint p2 = transrot(wxPoint((int)14 * targetscale / 100, 0), sin_theta,
                            cos_theta, TargetPoint);

      dc.SetPen(wxPen(UBLCK, 2));
      dc.StrokeLine(p1.x, p1.y, p2.x, p2.y);
    }

    //    European Inland AIS define a "stbd-stbd" meeting sign, a blue paddle.
    //    Symbolize it if set by most recent message
    //    Blue paddel is used while "not engaged"(1) or "engaged"(2)  (3 ==
    //    "reserved")
    if (td->blue_paddle && td->blue_paddle < 3) {
      wxPoint ais_flag_icon[4];
      int penWidth = 2;

      if (g_bInlandEcdis) {
        if (b_hdgValid) {
          ais_flag_icon[0] = wxPoint(-4, 4);
          ais_flag_icon[1] = wxPoint(-4, 11);
          ais_flag_icon[2] = wxPoint(-11, 11);
          ais_flag_icon[3] = wxPoint(-11, 4);
          transrot_pts(4, ais_flag_icon, sin_theta, cos_theta, TargetPoint);
        } else {
          ais_flag_icon[0] = wxPoint(TargetPoint.x - 4, TargetPoint.y + 4);
          ais_flag_icon[1] = wxPoint(TargetPoint.x - 4, TargetPoint.y - 3);
          ais_flag_icon[2] = wxPoint(TargetPoint.x + 3, TargetPoint.y - 3);
          ais_flag_icon[3] = wxPoint(TargetPoint.x + 3, TargetPoint.y + 4);
        }

        dc.SetPen(wxPen(GetGlobalColor(_T ( "CHWHT" )), penWidth));

      } else {
        ais_flag_icon[0] =
            wxPoint((int)-8 * targetscale / 100, (int)-6 * targetscale / 100);
        ais_flag_icon[1] =
            wxPoint((int)-2 * targetscale / 100, (int)18 * targetscale / 100);
        ais_flag_icon[2] = wxPoint((int)-2 * targetscale / 100, 0);
        ais_flag_icon[3] =
            wxPoint((int)-2 * targetscale / 100, (int)-6 * targetscale / 100);
        transrot_pts(4, ais_flag_icon, sin_theta, cos_theta, TargetPoint);

        if (targetscale < 100) penWidth = 1;
        dc.SetPen(wxPen(GetGlobalColor(_T ( "CHWHT" )), penWidth));
      }
      if (td->blue_paddle == 1) {
        ais_flag_icon[1] = ais_flag_icon[0];
        ais_flag_icon[2] = ais_flag_icon[3];
      }

      dc.SetBrush(wxBrush(GetGlobalColor(_T ( "UINFB" ))));
      dc.StrokePolygon(4, ais_flag_icon);
    }
  }

  if ((g_bShowAISName) && (targetscale > 75)) {
    int true_scale_display = (int)(floor(vp.chart_scale / 100.) * 100);
    if (true_scale_display <
        g_Show_Target_Name_Scale) {  // from which scale to display name

      wxString tgt_name = td->GetFullName();
      tgt_name = tgt_name.substr(0, tgt_name.find(_T ( "Unknown" ), 0));

      if (tgt_name != wxEmptyString) {
        dc.SetFont(*AIS_NameFont);
        dc.SetTextForeground(FontMgr::Get().GetFontColor(_("AIS Target Name")));

        int w, h;
        dc.GetTextExtent(_T("W"), &w, &h);
        h *= g_Platform->GetDisplayDIPMult(gFrame);
        w *= g_Platform->GetDisplayDIPMult(gFrame);

        if ((td->COG > 90) && (td->COG < 180))
          dc.DrawText(tgt_name, TargetPoint.x + w, TargetPoint.y - h);
        else
          dc.DrawText(tgt_name, TargetPoint.x + w,
                      TargetPoint.y /*+ (0.5 * h)*/);

      }  // If name do not empty
    }    // if scale
  }

  //  Draw tracks if enabled
  //  Check the Special MMSI Properties array
  bool b_noshow = false;
  bool b_forceshow = false;
  for (unsigned int i = 0; i < g_MMSI_Props_Array.GetCount(); i++) {
    if (td->MMSI == g_MMSI_Props_Array[i]->MMSI) {
      MmsiProperties *props = g_MMSI_Props_Array[i];
      if (TRACKTYPE_NEVER == props->TrackType) {
        b_noshow = true;
        break;
      } else if (TRACKTYPE_ALWAYS == props->TrackType) {
        b_forceshow = true;
        break;
      } else
        break;
    }
  }

  int TrackLength = td->m_ptrack.size();
  if (((!b_noshow && td->b_show_track) || b_forceshow) && (TrackLength > 1)) {
    //  create vector of x-y points
    int TrackPointCount;
    wxPoint *TrackPoints = 0;
    TrackPoints = new wxPoint[TrackLength];
    auto it = td->m_ptrack.begin();
    for (TrackPointCount = 0;
         it != td->m_ptrack.end() && (TrackPointCount < TrackLength);
         TrackPointCount++, ++it) {
      const AISTargetTrackPoint &ptrack_point = *it;
      GetCanvasPointPix(vp, cp, ptrack_point.m_lat, ptrack_point.m_lon,
                        &TrackPoints[TrackPointCount]);
    }

    wxColour c = GetGlobalColor(_T ( "CHMGD" ));
    dc.SetPen(wxPen(c, 1.5 * AIS_nominal_line_width_pix));

    // Check for any persistently tracked target
    // Render persistently tracked targets slightly differently.
    std::map<int, Track *>::iterator itt;
    itt = g_pAIS->m_persistent_tracks.find(td->MMSI);
    if (itt != g_pAIS->m_persistent_tracks.end()) {
      auto *ptrack = itt->second;
      if (ptrack->m_Colour == wxEmptyString) {
        c = GetGlobalColor(_T ( "TEAL1" ));
        dc.SetPen(wxPen(c, 2.0 * AIS_nominal_line_width_pix));
      } else {
        for (unsigned int i = 0;
             i < sizeof(::GpxxColorNames) / sizeof(wxString); i++) {
          if (ptrack->m_Colour == ::GpxxColorNames[i]) {
            c = ::GpxxColors[i];
            dc.SetPen(wxPen(c, 2.0 * AIS_nominal_line_width_pix));
            break;
          }
        }
      }
    }

#ifdef ocpnUSE_GL
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)

    if (!dc.GetDC()) {
      glLineWidth(2);
      glColor3ub(c.Red(), c.Green(), c.Blue());
      glBegin(GL_LINE_STRIP);

      for (TrackPointCount = 0; TrackPointCount < TrackLength;
           TrackPointCount++)
        glVertex2i(TrackPoints[TrackPointCount].x,
                   TrackPoints[TrackPointCount].y);

      glEnd();
    } else {
      dc.DrawLines(TrackPointCount, TrackPoints);
    }
#else
    dc.DrawLines(TrackPointCount, TrackPoints);
#endif

#else
    if (dc.GetDC()) dc.StrokeLines(TrackPointCount, TrackPoints);

#endif

    delete[] TrackPoints;

  }  // Draw tracks
}

void AISDraw(ocpnDC &dc, ViewPort &vp, ChartCanvas *cp) {
  if (!g_pAIS) return;

  // Toggling AIS display on and off
  if (cp != NULL) {
    if (!cp->GetShowAIS()) return;
  }

  AISSetMetrics();

  const auto &current_targets = g_pAIS->GetTargetList();

  //      Iterate over the AIS Target Hashmap but only for the main chartcanvas.
  //      For secundairy canvasses we use the same value for the AIS importance
  bool go = false;

  if (cp == NULL) {
    go = true;
  } else if (cp->m_canvasIndex == 0) {
    go = true;
  }

  if (go) {
    for (const auto &it : current_targets) {
      // calculate the importancefactor for each target
      auto td = it.second;
      double So, Cpa, Rang, Siz = 0.0;
      So = g_ScaledNumWeightSOG / 12 *
           td->SOG;  // 0 - 12 knts gives 0 - g_ScaledNumWeightSOG weight
      if (So > g_ScaledNumWeightSOG) So = g_ScaledNumWeightSOG;

      if (td->bCPA_Valid) {
        Cpa = g_ScaledNumWeightCPA - g_ScaledNumWeightCPA / 4 * td->CPA;
        // if TCPA is positief (target is coming closer), make weight of CPA
        // bigger
        if (td->TCPA > .0) Cpa = Cpa + Cpa * g_ScaledNumWeightTCPA / 100;
        if (Cpa < .0) Cpa = .0;  // if CPA is > 4
      } else
        Cpa = .0;

      Rang = g_ScaledNumWeightRange / 10 * td->Range_NM;
      if (Rang > g_ScaledNumWeightRange) Rang = g_ScaledNumWeightRange;
      Rang = g_ScaledNumWeightRange - Rang;

      Siz = g_ScaledNumWeightSizeOfT / 30 * (td->DimA + td->DimB);
      if (Siz > g_ScaledNumWeightSizeOfT) Siz = g_ScaledNumWeightSizeOfT;
      td->importance = (float)So + Cpa + Rang + Siz;
    }
  }

  // If needed iterate over all targets, check if they fit in the viewport and
  // if yes add the importancefactor to a sorted list
  AISImportanceSwitchPoint = 0.0;

  float *Array = new float[g_ShowScaled_Num];
  for (int i = 0; i < g_ShowScaled_Num; i++) Array[i] = 0.0;

  int LowestInd = 0;
  if (cp != NULL) {
    if (cp->GetAttenAIS()) {
      for (const auto &it : current_targets) {
        auto td = it.second;
        if (vp.GetBBox().Contains(td->Lat, td->Lon)) {
          if (td->importance > AISImportanceSwitchPoint) {
            Array[LowestInd] = td->importance;

            AISImportanceSwitchPoint = Array[0];
            LowestInd = 0;
            for (int i = 1; i < g_ShowScaled_Num; i++) {
              if (Array[i] < AISImportanceSwitchPoint) {
                AISImportanceSwitchPoint = Array[i];
                LowestInd = i;
              }
            }
          }
        }
      }
    }
  }
  delete[] Array;

  //    Draw all targets in three pass loop, sorted on SOG, GPSGate & DSC on top
  //    This way, fast targets are not obscured by slow/stationary targets
  for (const auto &it : current_targets) {
    auto td = it.second;
    if ((td->SOG < g_SOGminCOG_kts) &&
        !((td->Class == AIS_GPSG_BUDDY) || (td->Class == AIS_DSC))) {
      AISDrawTarget(td.get(), dc, vp, cp);
    }
  }

  for (const auto &it : current_targets) {
    auto td = it.second;
    if ((td->SOG >= g_SOGminCOG_kts) &&
        !((td->Class == AIS_GPSG_BUDDY) || (td->Class == AIS_DSC))) {
      AISDrawTarget(td.get(), dc, vp, cp);  // yes this is a doubling of code;(
      if (td->importance > 0) AISDrawTarget(td.get(), dc, vp, cp);
    }
  }

  for (const auto &it : current_targets) {
    auto td = it.second;
    if ((td->Class == AIS_GPSG_BUDDY) || (td->Class == AIS_DSC))
      AISDrawTarget(td.get(), dc, vp, cp);
  }
}

bool AnyAISTargetsOnscreen(ChartCanvas *cc, ViewPort &vp) {
  if (!g_pAIS) return false;

  if (!cc->GetShowAIS()) return false;  //

  //      Iterate over the AIS Target Hashmap
  for (const auto &it : g_pAIS->GetTargetList()) {
    auto td = it.second;
    if (vp.GetBBox().Contains(td->Lat, td->Lon)) return true;  // yep
  }

  return false;
}
