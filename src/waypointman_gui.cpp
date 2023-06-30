
/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  implement waypointman_gui.h: WayPointman drawing stuff
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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
 ******************A********************************************************/

#include <wx/arrstr.h>
#include <wx/bitmap.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/gdicmn.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/utils.h>

#include "base_platform.h"
#include "MarkIcon.h"
#include "route_point.h"
#include "styles.h"
#include "svg_utils.h"
#include "waypointman_gui.h"
#include "ocpn_plugin.h"

extern BasePlatform* g_BasePlatform;
extern float g_MarkScaleFactorExp;
extern ocpnStyle::StyleManager *g_StyleManager;

static int CompareMarkIcons(MarkIcon *mi1, MarkIcon *mi2) {
  return (mi1->icon_name.CmpNoCase(mi2->icon_name));
}


void WayPointmanGui::ProcessUserIcons(ocpnStyle::Style *style,
                                      double displayDPmm) {
  wxString msg;
  msg.Printf(_T("DPMM: %g   ScaleFactorExp: %g"), displayDPmm,
             g_MarkScaleFactorExp);
  wxLogMessage(msg);

  wxString UserIconPath = g_BasePlatform->GetPrivateDataDir();
  wxChar sep = wxFileName::GetPathSeparator();
  if (UserIconPath.Last() != sep) UserIconPath.Append(sep);
  UserIconPath.Append(_T("UserIcons/"));

  wxLogMessage(_T("Looking for UserIcons at ") + UserIconPath);

  if (wxDir::Exists(UserIconPath)) {
    wxLogMessage(_T("Loading UserIcons from ") + UserIconPath);
    wxArrayString FileList;

    int n_files =
        wxDir::GetAllFiles(UserIconPath, &FileList, _T(""), wxDIR_FILES);

    for (int ifile = 0; ifile < n_files; ifile++) {
      wxString name = FileList[ifile];

      wxFileName fn(name);
      wxString iconname = fn.GetName();
      wxBitmap icon1;

      if (fn.GetExt().Lower() == _T("xpm")) {
        if (icon1.LoadFile(name, wxBITMAP_TYPE_XPM)) {
          wxLogMessage(_T("Adding icon: ") + iconname);
          ProcessIcon(icon1, iconname, iconname);
        }
      }
      if (fn.GetExt().Lower() == _T("png")) {
        if (icon1.LoadFile(name, wxBITMAP_TYPE_PNG)) {
          wxLogMessage(_T("Adding icon: ") + iconname);
          ProcessIcon(icon1, iconname, iconname);
        }
      }
      if (fn.GetExt().Lower() == _T("svg")) {
        unsigned int w, h;
        SVGDocumentPixelSize(name, w, h);
        w = wxMax(wxMax(w, h), 15);  // We want certain minimal size for the
                                     // icons, 15px (approx 3mm) be it
        const unsigned int bm_size = w; //SVGPixelsToDisplay(w);
        wxBitmap iconSVG = LoadSVG(name, bm_size, bm_size);
        MarkIcon *pmi = ProcessIcon(iconSVG, iconname, iconname);
        if (pmi) pmi->preScaled = true;
      }
    }
  }
}

MarkIcon* WayPointmanGui::ProcessIcon(wxBitmap pimage, const wxString& key,
                                      const wxString& description) {
  MarkIcon *pmi = 0;

  bool newIcon = true;

  // avoid adding duplicates
  for (unsigned int i = 0; i < m_waypoint_man.m_pIconArray->GetCount(); i++) {
    pmi = (MarkIcon *)m_waypoint_man.m_pIconArray->Item(i);
    if (pmi->icon_name.IsSameAs(key)) {
      newIcon = false;
      delete pmi->piconBitmap;
      break;
    }
  }

  if (newIcon) {
    pmi = new MarkIcon;
    pmi->icon_name = key;  // Used for sorting
    m_waypoint_man.m_pIconArray->Add(pmi);
  }

  wxBitmap *pbm = new wxBitmap(pimage);
  pmi->icon_name = key;
  pmi->icon_description = description;
  pmi->piconBitmap = NULL;
  pmi->icon_texture = 0; /* invalidate */
  pmi->preScaled = false;
  pmi->iconImage = pbm->ConvertToImage();
  pmi->m_blistImageOK = false;
  delete pbm;

  return pmi;
}

void WayPointmanGui::ProcessIcons(ocpnStyle::Style *style, double displayDPmm) {
  m_waypoint_man.m_pIconArray->Clear();

  ProcessDefaultIcons(displayDPmm);

  // Load user defined icons.
  // Done after default icons are initialized,
  // so that user may substitute an icon by using the same name in the Usericons
  // file.
  ProcessUserIcons(style, displayDPmm);

  if (NULL != m_waypoint_man.pmarkicon_image_list) {
    m_waypoint_man.pmarkicon_image_list->RemoveAll();
    delete m_waypoint_man.pmarkicon_image_list;
    m_waypoint_man.pmarkicon_image_list = NULL;
  }

  // First find the largest bitmap size, to use as the base size for lists of
  // icons
  int w = 0;
  int h = 0;

  for (unsigned int i = 0; i < m_waypoint_man.m_pIconArray->GetCount(); i++) {
    MarkIcon *pmi = (MarkIcon *)m_waypoint_man.m_pIconArray->Item(i);
    w = wxMax(w, pmi->iconImage.GetWidth());
    h = wxMax(h, pmi->iconImage.GetHeight());
  }

  m_waypoint_man.m_bitmapSizeForList = wxMax(w, h);
  m_waypoint_man.m_bitmapSizeForList =
      wxMin(100, m_waypoint_man.m_bitmapSizeForList);
}

void WayPointmanGui::ProcessDefaultIcons(double displayDPmm) {
  wxString iconDir = g_BasePlatform->GetSharedDataDir();
  appendOSDirSlash(&iconDir);
  iconDir.append(_T("uidata"));
  appendOSDirSlash(&iconDir);
  iconDir.append(_T("markicons"));
  appendOSDirSlash(&iconDir);

  MarkIcon *pmi = 0;

  // Add the legacy icons to their own sorted array
  if (m_waypoint_man.m_pLegacyIconArray)
    m_waypoint_man.m_pLegacyIconArray->Clear();
  else
   m_waypoint_man.m_pLegacyIconArray =
       new SortedArrayOfMarkIcon(CompareMarkIcons);

  pmi = ProcessLegacyIcon(iconDir + _T("Symbol-Empty.svg"), _T("empty"),
                          _T("Empty"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Symbol-Triangle.svg"), _T("triangle"),
                          _T("Triangle"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("1st-Active-Waypoint.svg"),
                          _T("activepoint"), _T("Active WP"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Marks-Boarding-Location.svg"),
                          _T("boarding"), _T("Boarding Location"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Hazard-Airplane.svg"), _T("airplane"),
                          _T("Airplane"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("1st-Anchorage.svg"), _T("anchorage"),
                          _T("Anchorage"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Symbol-Anchor2.svg"), _T("anchor"),
                          _T("Anchor"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Marks-Boundary.svg"), _T("boundary"),
                          _T("Boundary Mark"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Marks-Buoy-TypeA.svg"), _T("bouy1"),
                          _T("Bouy Type A"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Marks-Buoy-TypeB.svg"), _T("bouy2"),
                          _T("Bouy Type B"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Activity-Campfire.svg"), _T("campfire"),
                          _T("Campfire"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Activity-Camping.svg"), _T("camping"),
                          _T("Camping Spot"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Sea-Floor-Coral.svg"), _T("coral"),
                          _T("Coral"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Activity-Fishing.svg"), _T("fishhaven"),
                          _T("Fish Haven"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Activity-Fishing.svg"), _T("fishing"),
                          _T("Fishing Spot"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Activity-Fishing.svg"), _T("fish"),
                          _T("Fish"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Marks-Mooring-Buoy.svg"), _T("float"),
                          _T("Float"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Service-Food.svg"), _T("food"),
                          _T("Food"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Service-Fuel-Pump-Diesel-Petrol.svg"),
                          _T("fuel"), _T("Fuel"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Marks-Light-Green.svg"),
                          _T("greenlite"), _T("Green Light"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Sea-Floor-Sea-Weed.svg"), _T("kelp"),
                          _T("Kelp"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Marks-Light-TypeA.svg"), _T("light"),
                          _T("Light Type A"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Marks-Light-TypeB.svg"), _T("light1"),
                          _T("Light Type B"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Marks-Light-Vessel.svg"),
                          _T("litevessel"), _T("litevessel"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("1st-Man-Overboard.svg"), _T("mob"),
                          _T("MOB"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Marks-Mooring-Buoy.svg"), _T("mooring"),
                          _T("Mooring Bouy"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Marks-Mooring-Buoy-Super.svg"),
                          _T("oilbouy"), _T("Oil Bouy"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Hazard-Oil-Platform.svg"),
                          _T("platform"), _T("Platform"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Marks-Light-Red-Green.svg"),
                          _T("redgreenlite"), _T("Red/Green Light"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Marks-Light-Red.svg"), _T("redlite"),
                          _T("Red Light"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Hazard-Rock-Exposed.svg"), _T("rock1"),
                          _T("Rock (exposed)"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Hazard-Rock-Awash.svg"), _T("rock2"),
                          _T("Rock, (awash)"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Hazard-Sandbar.svg"), _T("sand"),
                          _T("Sand"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Activity-Diving-Scuba-Flag.svg"),
                          _T("scuba"), _T("Scuba"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Hazard-Sandbar.svg"), _T("shoal"),
                          _T("Shoal"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Hazard-Snag.svg"), _T("snag"),
                          _T("Snag"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Symbol-Square.svg"), _T("square"),
                          _T("Square"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("1st-Diamond.svg"), _T("diamond"),
                          _T("Diamond"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Symbol-Circle.svg"), _T("circle"),
                          _T("Circle"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Hazard-Wreck1.svg"), _T("wreck1"),
                          _T("Wreck A"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Hazard-Wreck2.svg"), _T("wreck2"),
                          _T("Wreck B"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Symbol-X-Small-Blue.svg"), _T("xmblue"),
                          _T("Blue X"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Symbol-X-Small-Green.svg"),
                          _T("xmgreen"), _T("Green X"), displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + _T("Symbol-X-Small-Red.svg"), _T("xmred"),
                          _T("Red X"), displayDPmm);
  if (pmi) pmi->preScaled = true;

  // Add the extended icons to their own sorted array
  if (m_waypoint_man.m_pExtendedIconArray)
    m_waypoint_man.m_pExtendedIconArray->Clear();
  else
    m_waypoint_man.m_pExtendedIconArray =
        new SortedArrayOfMarkIcon(CompareMarkIcons);

#if 0
    wxArrayString FileList;
    double bm_size = -1;

    int n_files = wxDir::GetAllFiles( iconDir, &FileList );

    // If the scale factor is not unity, measure the first icon in the list
    //  So that we may apply the scale factor exactly to all
    if( fabs(g_ChartScaleFactorExp - 1.0) > 0.1){

        for( int ifile = 0; ifile < n_files; ifile++ ) {
            wxString name = FileList[ifile];

            wxFileName fn( name );

            if( fn.GetExt().Lower() == _T("svg") ) {
                wxBitmap bmt = LoadSVG(name, -1, -1 );
                bm_size = bmt.GetWidth() * g_ChartScaleFactorExp;
                break;
            }
        }
    }

    for( int ifile = 0; ifile < n_files; ifile++ ) {
        wxString name = FileList[ifile];

        wxFileName fn( name );
        wxString iconname = fn.GetName();
        wxBitmap icon1;
        if( fn.GetExt().Lower() == _T("svg") ) {
            wxImage iconSVG = LoadSVG( name, (int)bm_size, (int)bm_size );
            MarkIcon * pmi = ProcessExtendedIcon( iconSVG, iconname, iconname );
            if(pmi)
                pmi->preScaled = true;
        }
    }
#else

  wxArrayString FileList;
  // nominal size, but not less than 4 pixel
  double bm_size = wxMax(4.0, floor(displayDPmm * 12.0));
  bm_size /= OCPN_GetWinDIPScaleFactor();
  bm_size *= g_MarkScaleFactorExp;

  int n_files = wxDir::GetAllFiles(iconDir, &FileList);

  g_BasePlatform->ShowBusySpinner();

  for (int ifile = 0; ifile < n_files; ifile++) {
    wxString name = FileList[ifile];

    wxFileName fn(name);
    wxString iconname = fn.GetName();
    wxBitmap icon1;

    if (fn.GetExt().Lower() == _T("svg")) {
      unsigned int w, h;

      SVGDocumentPixelSize(name, w, h);

      // We want certain minimal size for the icons
      float nominal_icon_size_pixels = wxMax(4.0,
                                      floor(displayDPmm * 12.0));

      w = wxMax(wxMax(w, h), nominal_icon_size_pixels);

      bm_size = w * g_MarkScaleFactorExp;
      bm_size /= OCPN_GetWinDIPScaleFactor();

      wxBitmap bmp = LoadSVG(name, (int)bm_size, (int)bm_size);
      if (bmp.IsOk()) {
        wxImage iconSVG = bmp.ConvertToImage();

        MarkIcon *pmi = ProcessExtendedIcon(iconSVG, iconname, iconname);
        if (pmi) pmi->preScaled = true;
      } else {
        wxLogMessage("Failed loading mark icon " + name);
      }
    }
  }
  g_BasePlatform->HideBusySpinner();
#endif

  // Walk the two sorted lists, adding icons to the un-sorted master list

  auto size = m_waypoint_man.m_pLegacyIconArray->GetCount();
  for (unsigned int i = 0; i < size; i++) {
    pmi = (MarkIcon *)m_waypoint_man.m_pLegacyIconArray->Item(i);
    m_waypoint_man.m_pIconArray->Add(pmi);
  }

  size = m_waypoint_man.m_pExtendedIconArray->GetCount();
  for (unsigned int i = 0; i < size; i++) {
    pmi = (MarkIcon *) m_waypoint_man.m_pExtendedIconArray->Item(i);

    //  Do not add any icons from the extended array if they have already been
    //  used as legacy substitutes
    bool noAdd = false;
    auto legacy_count =  m_waypoint_man.m_pLegacyIconArray->GetCount();
    for (unsigned int j = 0; j < legacy_count; j++) {
      MarkIcon *pmiLegacy =
          (MarkIcon *)m_waypoint_man.m_pLegacyIconArray->Item(j);
      if (pmiLegacy->icon_name.IsSameAs(pmi->icon_name)) {
        noAdd = true;
        break;
      }
    }
    if (!noAdd) m_waypoint_man.m_pIconArray->Add(pmi);
  }
}

void WayPointmanGui::ReloadAllIcons(double displayDPmm) {
  ProcessIcons(g_StyleManager->GetCurrentStyle(), displayDPmm);

  for (unsigned int i = 0; i < m_waypoint_man.m_pIconArray->GetCount(); i++) {
    MarkIcon *pmi = (MarkIcon *)m_waypoint_man.m_pIconArray->Item(i);
    wxImage dim_image;
    if (m_waypoint_man.m_cs == GLOBAL_COLOR_SCHEME_DUSK) {
      dim_image =  m_waypoint_man.CreateDimImage(pmi->iconImage, .50);
      pmi->iconImage = dim_image;
    } else if (m_waypoint_man.m_cs == GLOBAL_COLOR_SCHEME_NIGHT) {
      dim_image =  m_waypoint_man.CreateDimImage(pmi->iconImage, .20);
      pmi->iconImage = dim_image;
    }
  }
  ReloadRoutepointIcons();
}

void WayPointmanGui::SetColorScheme(ColorScheme cs, double displayDPmm) {
  m_waypoint_man.m_cs = cs;
  ReloadAllIcons(displayDPmm);
}

MarkIcon *WayPointmanGui::ProcessLegacyIcon(wxString fileName, const wxString &key,
                                            const wxString &description,
                                            double displayDPmm) {
  double bm_size = -1.0;

#ifndef ocpnUSE_wxBitmapBundle
#ifndef __ANDROID__
  if (fabs(g_MarkScaleFactorExp - 1.0) > 0.1) {
    wxBitmap img = LoadSVG(fileName, -1, -1);
    bm_size = img.GetWidth() * g_MarkScaleFactorExp;
    bm_size /= OCPN_GetWinDIPScaleFactor();
  }
#else
  //  Set the onscreen size of the symbol
  //  Compensate for various display resolutions
  //  Develop empirically, making a "diamond" symbol about 4 mm square
  //  Android uses "density buckets", so simple math produces poor results.
  //  Thus, these factors have been empirically tweaked to provide good results
  //  on a variety of devices
  float nominal_legacy_icon_size_pixels = wxMax(4.0,
                                                floor(displayDPmm * 12.0));
  // legacy icon size
  float pix_factor = nominal_legacy_icon_size_pixels / 68.0;

  unsigned int w, h;
  SVGDocumentPixelSize(fileName, w, h);

  bm_size = w * pix_factor * g_MarkScaleFactorExp;

#endif
#else
  unsigned int w, h;
  SVGDocumentPixelSize(fileName, w, h);
  w = wxMax(wxMax(w, h), 15);  // We want certain minimal size for the icons,
                               // 15px (approx 3mm) be it
  bm_size = w * g_MarkScaleFactorExp; //SVGPixelsToDisplay(w);
  bm_size /= OCPN_GetWinDIPScaleFactor();
#endif

  wxImage image =
      LoadSVG(fileName, (int)bm_size, (int)bm_size).ConvertToImage();

  wxRect rClip = CropImageOnAlpha(image);
  wxImage imageClip = image.GetSubImage(rClip);

  MarkIcon *pmi = 0;

  bool newIcon = true;

  // avoid adding duplicates
  for (unsigned int i = 0; i < m_waypoint_man.m_pLegacyIconArray->GetCount(); i++) {
    pmi = (MarkIcon *)m_waypoint_man.m_pLegacyIconArray->Item(i);
    if (pmi->icon_name.IsSameAs(key)) {
      newIcon = false;
      delete pmi->piconBitmap;
      break;
    }
  }

  if (newIcon) {
    pmi = new MarkIcon;
    pmi->icon_name = key;  // Used for sorting
    m_waypoint_man.m_pLegacyIconArray->Add(pmi);
  }

  pmi->icon_name = key;
  pmi->icon_description = description;
  pmi->piconBitmap = NULL;
  pmi->icon_texture = 0; /* invalidate */
  pmi->preScaled = false;
  pmi->iconImage = imageClip;
  pmi->m_blistImageOK = false;

  return pmi;
}

MarkIcon *WayPointmanGui::ProcessExtendedIcon(wxImage &image,
                                              const wxString &key,
                                              const wxString &description) {
  MarkIcon *pmi = 0;

  bool newIcon = true;

  // avoid adding duplicates
  auto size = m_waypoint_man.m_pExtendedIconArray->GetCount();
  for (unsigned int i = 0; i < size; i++) {
    pmi = (MarkIcon *)m_waypoint_man.m_pExtendedIconArray->Item(i);
    if (pmi->icon_name.IsSameAs(key)) {
      newIcon = false;
      delete pmi->piconBitmap;
      break;
    }
  }

  if (newIcon) {
    pmi = new MarkIcon;
    pmi->icon_name = key;  // Used for sorting
    m_waypoint_man.m_pExtendedIconArray->Add(pmi);
  }

  wxRect rClip = CropImageOnAlpha(image);
  wxImage imageClip = image.GetSubImage(rClip);

  pmi->icon_name = key;
  pmi->icon_description = description;
  pmi->piconBitmap = new wxBitmap(imageClip);
  pmi->icon_texture = 0; /* invalidate */
  pmi->preScaled = false;
  pmi->iconImage = imageClip;
  pmi->m_blistImageOK = false;

  return pmi;
}

wxRect WayPointmanGui::CropImageOnAlpha(wxImage &image) {
  const int w = image.GetWidth();
  const int h = image.GetHeight();

  wxRect rv = wxRect(0, 0, w, h);
  if (!image.HasAlpha()) return rv;

  unsigned char *pAlpha = image.GetAlpha();

  int leftCrop = w;
  int topCrop = h;
  int rightCrop = w;
  int bottomCrop = h;

  // Horizontal
  for (int i = 0; i < h; i++) {
    int lineStartIndex = i * w;

    int j = 0;
    while ((j < w) && (pAlpha[lineStartIndex + j] == 0)) j++;
    leftCrop = wxMin(leftCrop, j);

    int k = w - 1;
    while (k && (pAlpha[lineStartIndex + k] == 0)) k--;
    rightCrop = wxMin(rightCrop, image.GetWidth() - k - 2);
  }

  // Vertical
  for (int i = 0; i < w; i++) {
    int columnStartIndex = i;

    int j = 0;
    while ((j < h) && (pAlpha[columnStartIndex + (j * w)] == 0)) j++;
    topCrop = wxMin(topCrop, j);

    int k = h - 1;
    while (k && (pAlpha[columnStartIndex + (k * w)] == 0)) k--;
    bottomCrop = wxMin(bottomCrop, h - k - 2);
  }

  int xcrop = wxMin(rightCrop, leftCrop);
  int ycrop = wxMin(topCrop, bottomCrop);
  int crop = wxMin(xcrop, ycrop);

  rv.x = wxMax(crop, 0);
  rv.width = wxMax(1, w - (2 * crop));
  rv.width = wxMin(rv.width, w);
  rv.y = rv.x;
  rv.height = rv.width;

  return rv;
}

void WayPointmanGui::ReloadRoutepointIcons() {
  //    Iterate on the RoutePoint list, requiring each to reload icon

  wxRoutePointListNode *node = m_waypoint_man.m_pWayPointList->GetFirst();
  while (node) {
    RoutePoint *pr = node->GetData();
    pr->ReLoadIcon();
    node = node->GetNext();
  }
}


