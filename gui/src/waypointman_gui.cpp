/***************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022 Alec Leamas                                        *
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
 ******************A********************************************************/

/**
 * \file
 *
 * Implement waypointman_gui.h -- WayPointman drawing stuff
 */

#include "gl_headers.h"

#include <wx/arrstr.h>
#include <wx/bitmap.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/gdicmn.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/utils.h>

#include "waypointman_gui.h"

#include "model/base_platform.h"
#include "model/config_vars.h"
#include "model/cutil.h"
#include "model/MarkIcon.h"
#include "model/route_point.h"
#include "model/svg_utils.h"

#include "ocpn_plugin.h"
#include "styles.h"

static int CompareMarkIcons(MarkIcon *mi1, MarkIcon *mi2) {
  return (mi1->icon_name.CmpNoCase(mi2->icon_name));
}

void WayPointmanGui::ProcessUserIcons(ocpnStyle::Style *style,
                                      double displayDPmm) {
  wxString msg;
  msg.Printf("DPMM: %g   ScaleFactorExp: %g", displayDPmm,
             g_MarkScaleFactorExp);
  wxLogMessage(msg);

  wxString UserIconPath = g_BasePlatform->GetPrivateDataDir();
  wxChar sep = wxFileName::GetPathSeparator();
  if (UserIconPath.Last() != sep) UserIconPath.Append(sep);
  UserIconPath.Append("UserIcons/");

  wxLogMessage("Looking for UserIcons at " + UserIconPath);

  if (wxDir::Exists(UserIconPath)) {
    wxLogMessage("Loading UserIcons from " + UserIconPath);
    wxArrayString FileList;

    wxBitmap default_bm = wxBitmap(1, 1);  // empty

    int n_files = wxDir::GetAllFiles(UserIconPath, &FileList, "", wxDIR_FILES);

    for (int ifile = 0; ifile < n_files; ifile++) {
      wxString name =
          g_bUserIconsFirst ? FileList[n_files - ifile - 1] : FileList[ifile];

      wxFileName fn(name);
      wxString iconname = fn.GetName();
      wxBitmap icon1;
      if (fn.GetExt().Lower() == "xpm") {
        if (icon1.LoadFile(name, wxBITMAP_TYPE_XPM)) {
          wxLogMessage("Adding icon: " + iconname);
          wxImage image = icon1.ConvertToImage();
          ProcessIcon(image, iconname, iconname, g_bUserIconsFirst);
        }
      }
      if (fn.GetExt().Lower() == "png") {
        if (icon1.LoadFile(name, wxBITMAP_TYPE_PNG)) {
          wxLogMessage("Adding icon: " + iconname);
          wxImage image = icon1.ConvertToImage();
          ProcessIcon(image, iconname, iconname, g_bUserIconsFirst);
        }
      }
      if (fn.GetExt().Lower() == "svg") {
        unsigned int w, h;
        SVGDocumentPixelSize(name, w, h);

        // This is to be a mark icon
        // Make it a nominal max size
        double bm_size_nom = wxMin(wxMax(w, h), floor(displayDPmm * 20));
        // We want certain minimal size for the icons, 15px (approx 3mm) be it
        bm_size_nom = wxMax(bm_size_nom, 15);

        bm_size_nom /= OCPN_GetWinDIPScaleFactor();
        bm_size_nom *= g_MarkScaleFactorExp;

        MarkIcon *pmi = NULL;
        double aspect =
            1.0;  // Use default aspect ratio of 1 if width/height are missing.
        if (w != 0 && h != 0) {
          aspect = h / w;
        }

        // Make the rendered icon square, if necessary
        if (fabs(aspect - 1.0) > .05) {
          wxImage image =
              LoadSVG(name, (int)bm_size_nom, (int)bm_size_nom, &default_bm)
                  .ConvertToImage();

          if (image.IsOk()) {
            wxRect rClip = CropImageOnAlpha(image);
            wxImage imageClip = image.GetSubImage(rClip);
            imageClip.Rescale(bm_size_nom, bm_size_nom / aspect,
                              wxIMAGE_QUALITY_BICUBIC);
            pmi = ProcessIcon(imageClip, iconname, iconname, g_bUserIconsFirst);
          }
        } else {
          const unsigned int bm_size = bm_size_nom;  // horizontal
          wxImage iconSVG = LoadSVG(name, bm_size, bm_size, &default_bm, false)
                                .ConvertToImage();
          wxRect rClip = CropImageOnAlpha(iconSVG);
          wxImage imageClip = iconSVG.GetSubImage(rClip);
          pmi = ProcessIcon(iconSVG, iconname, iconname, g_bUserIconsFirst);
        }

        if (pmi) pmi->preScaled = true;
      }
    }
  }
}

MarkIcon *WayPointmanGui::ProcessIcon(wxImage image, const wxString &key,
                                      const wxString &description,
                                      bool add_in_front) {
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
    if (add_in_front)
      m_waypoint_man.m_pIconArray->Insert(pmi, 0);
    else {
      m_waypoint_man.m_pIconArray->Add(pmi);
    }
  }

  wxBitmap *pbm = new wxBitmap(image);
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
  for (unsigned int i = 0; i < m_waypoint_man.m_pIconArray->GetCount(); i++) {
    MarkIcon *pmi = (MarkIcon *)m_waypoint_man.m_pIconArray->Item(i);
    delete pmi->piconBitmap;
    delete pmi;
  }
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
  iconDir.append("uidata");
  appendOSDirSlash(&iconDir);
  iconDir.append("markicons");
  appendOSDirSlash(&iconDir);

  MarkIcon *pmi = 0;

  // Add the legacy icons to their own sorted array
  if (m_waypoint_man.m_pLegacyIconArray)
    m_waypoint_man.m_pLegacyIconArray->Clear();
  else
    m_waypoint_man.m_pLegacyIconArray =
        new SortedArrayOfMarkIcon(CompareMarkIcons);

  pmi = ProcessLegacyIcon(iconDir + "Symbol-Empty.svg", "empty", "Empty",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Symbol-Triangle.svg", "triangle",
                          "Triangle", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "1st-Active-Waypoint.svg", "activepoint",
                          "Active WP", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Marks-Boarding-Location.svg", "boarding",
                          "Boarding Location", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Hazard-Airplane.svg", "airplane",
                          "Airplane", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "1st-Anchorage.svg", "anchorage",
                          "Anchorage", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Symbol-Anchor2.svg", "anchor", "Anchor",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Marks-Boundary.svg", "boundary",
                          "Boundary Mark", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Marks-Buoy-TypeA.svg", "buoy1",
                          "buoy Type A", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Marks-Buoy-TypeB.svg", "buoy2",
                          "buoy Type B", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Activity-Campfire.svg", "campfire",
                          "Campfire", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Activity-Camping.svg", "camping",
                          "Camping Spot", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Sea-Floor-Coral.svg", "coral", "Coral",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Activity-Fishing.svg", "fishhaven",
                          "Fish Haven", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Activity-Fishing.svg", "fishing",
                          "Fishing Spot", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Activity-Fishing.svg", "fish", "Fish",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Marks-Mooring-Buoy.svg", "float", "Float",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Service-Food.svg", "food", "Food",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Service-Fuel-Pump-Diesel-Petrol.svg",
                          "fuel", "Fuel", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Marks-Light-Green.svg", "greenlite",
                          "Green Light", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Sea-Floor-Sea-Weed.svg", "kelp", "Kelp",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Marks-Light-TypeA.svg", "light",
                          "Light Type A", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Marks-Light-TypeB.svg", "light1",
                          "Light Type B", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Marks-Light-Vessel.svg", "litevessel",
                          "litevessel", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "1st-Man-Overboard.svg", "mob", "MOB",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Marks-Mooring-Buoy.svg", "mooring",
                          "Mooring buoy", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Marks-Mooring-Buoy-Super.svg", "oilbuoy",
                          "Oil buoy", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Hazard-Oil-Platform.svg", "platform",
                          "Platform", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Marks-Light-Red-Green.svg", "redgreenlite",
                          "Red/Green Light", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Marks-Light-Red.svg", "redlite",
                          "Red Light", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Hazard-Rock-Exposed.svg", "rock1",
                          "Rock (exposed)", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Hazard-Rock-Awash.svg", "rock2",
                          "Rock, (awash)", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Hazard-Sandbar.svg", "sand", "Sand",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Activity-Diving-Scuba-Flag.svg", "scuba",
                          "Scuba", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Hazard-Sandbar.svg", "shoal", "Shoal",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Hazard-Snag.svg", "snag", "Snag",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Symbol-Square.svg", "square", "Square",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "1st-Diamond.svg", "diamond", "Diamond",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Symbol-Circle.svg", "circle", "Circle",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Hazard-Wreck1.svg", "wreck1", "Wreck A",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Hazard-Wreck2.svg", "wreck2", "Wreck B",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Symbol-X-Small-Blue.svg", "xmblue",
                          "Blue X", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Symbol-X-Small-Green.svg", "xmgreen",
                          "Green X", displayDPmm);
  if (pmi) pmi->preScaled = true;
  pmi = ProcessLegacyIcon(iconDir + "Symbol-X-Small-Red.svg", "xmred", "Red X",
                          displayDPmm);
  if (pmi) pmi->preScaled = true;

  // Add the extended icons to their own sorted array
  if (m_waypoint_man.m_pExtendedIconArray) {
    m_waypoint_man.m_pExtendedIconArray->Clear();
  } else {
    m_waypoint_man.m_pExtendedIconArray =
        new SortedArrayOfMarkIcon(CompareMarkIcons);
  }

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

            if( fn.GetExt().Lower() == "svg" ) {
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
        if( fn.GetExt().Lower() == "svg" ) {
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

    if (fn.GetExt().Lower() == "svg") {
      unsigned int w, h;

      SVGDocumentPixelSize(name, w, h);

      // We want certain minimal size for the icons
      w = wxMax(wxMax(w, h), 15);
      bm_size = w * g_MarkScaleFactorExp;
      bm_size /= OCPN_GetWinDIPScaleFactor();
#ifdef __ANDROID__
      //  Set the onscreen size of the symbol
      //  Compensate for various display resolutions
      //  Develop empirically, making a "diamond" symbol about 4 mm square
      //  Android uses "density buckets", so simple math produces poor results.
      //  Thus, these factors have been empirically tweaked to provide good
      //  results on a variety of devices
      float nominal_legacy_icon_size_pixels =
          wxMax(4.0, floor(displayDPmm * 12.0));
      // legacy icon size
      float pix_factor = nominal_legacy_icon_size_pixels / 68.0;
      bm_size *= pix_factor;
#endif

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
    pmi = (MarkIcon *)m_waypoint_man.m_pExtendedIconArray->Item(i);

    //  Do not add any icons from the extended array if they have already been
    //  used as legacy substitutes
    bool noAdd = false;
    auto legacy_count = m_waypoint_man.m_pLegacyIconArray->GetCount();
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
      dim_image = m_waypoint_man.CreateDimImage(pmi->iconImage, .50);
      pmi->iconImage = dim_image;
    } else if (m_waypoint_man.m_cs == GLOBAL_COLOR_SCHEME_NIGHT) {
      dim_image = m_waypoint_man.CreateDimImage(pmi->iconImage, .20);
      pmi->iconImage = dim_image;
    }
  }
  ReloadRoutepointIcons();
}

void WayPointmanGui::SetColorScheme(ColorScheme cs, double displayDPmm) {
  m_waypoint_man.m_cs = cs;
  ReloadAllIcons(displayDPmm);
}

MarkIcon *WayPointmanGui::ProcessLegacyIcon(wxString fileName,
                                            const wxString &key,
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
  float nominal_legacy_icon_size_pixels = wxMax(4.0, floor(displayDPmm * 12.0));
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
  bm_size = w * g_MarkScaleFactorExp;  // SVGPixelsToDisplay(w);
  bm_size /= OCPN_GetWinDIPScaleFactor();
#endif

  wxBitmap bm = LoadSVG(fileName, (int)bm_size, (int)bm_size);
  if (!bm.IsOk()) return NULL;

  wxImage image =
      LoadSVG(fileName, (int)bm_size, (int)bm_size).ConvertToImage();

  wxRect rClip = CropImageOnAlpha(image);
  wxImage imageClip = image.GetSubImage(rClip);

  MarkIcon *pmi = 0;

  bool newIcon = true;

  // avoid adding duplicates
  for (unsigned int i = 0; i < m_waypoint_man.m_pLegacyIconArray->GetCount();
       i++) {
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
  for (RoutePoint *pr : *m_waypoint_man.m_pWayPointList) {
    pr->ReLoadIcon();
  }
}

unsigned int WayPointmanGui::GetIconTexture(const wxBitmap *pbm, int &glw,
                                            int &glh) {
#ifdef ocpnUSE_GL
  int index = m_waypoint_man.GetIconIndex(pbm);
  MarkIcon *pmi = (MarkIcon *)m_waypoint_man.m_pIconArray->Item(index);

  if (!pmi->icon_texture) {
    /* make rgba texture */
    wxImage image = pbm->ConvertToImage();
    unsigned char *d = image.GetData();
    if (d == 0) {
      // don't create a texture with junk
      return 0;
    }

    glGenTextures(1, &pmi->icon_texture);
    glBindTexture(GL_TEXTURE_2D, pmi->icon_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    int w = image.GetWidth(), h = image.GetHeight();

    pmi->tex_w = NextPow2(w);
    pmi->tex_h = NextPow2(h);

    unsigned char *a = image.GetAlpha();

    unsigned char mr, mg, mb;
    if (!a) image.GetOrFindMaskColour(&mr, &mg, &mb);

    unsigned char *e = new unsigned char[4 * w * h];
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        unsigned char r, g, b;
        int off = (y * w + x);
        r = d[off * 3 + 0];
        g = d[off * 3 + 1];
        b = d[off * 3 + 2];
        e[off * 4 + 0] = r;
        e[off * 4 + 1] = g;
        e[off * 4 + 2] = b;

        e[off * 4 + 3] =
            a ? a[off] : ((r == mr) && (g == mg) && (b == mb) ? 0 : 255);
      }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pmi->tex_w, pmi->tex_h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, e);

    delete[] e;
  }

  glw = pmi->tex_w;
  glh = pmi->tex_h;

  return pmi->icon_texture;
#else
  return 0;
#endif
}
