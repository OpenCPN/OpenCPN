/***************************************************************************
 *
 * Project:  OpenCPN
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

#include "OCPNListCtrl.h"
#include "model/ais_state_vars.h"
#include "model/ais_target_data.h"
#include "model/navutil_base.h"
#include "model/own_ship.h"

OCPNListCtrl::OCPNListCtrl(AISTargetListDialog* parent, wxWindowID id,
                           const wxPoint& pos, const wxSize& size, long style)
    : wxListCtrl(parent, id, pos, size, style) {
  m_parent = parent;
}

OCPNListCtrl::~OCPNListCtrl() {
  g_AisTargetList_column_spec.Clear();
  for (int i = 0; i < tlSOG + 1; i++) {
    wxListItem item;
    GetColumn(i, item);
    wxString sitem;
    sitem.Printf(_T("%d;"), item.m_width);
    g_AisTargetList_column_spec += sitem;
  }

#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
  int i_columns = GetColumnCount();
  wxArrayInt a_order(i_columns);
  a_order = GetColumnsOrder();
  g_AisTargetList_column_order.Clear();
  for (int i = 0; i < i_columns; i++) {
    wxString sitem;
    sitem.Printf(_T("%d;"), a_order[i]);
    g_AisTargetList_column_order += sitem;
  }
#endif
}

wxString OCPNListCtrl::OnGetItemText(long item, long column) const {
  wxString ret;

  if (m_parent->m_pListCtrlAISTargets) {
    auto pAISTarget = m_parent->GetpTarget(item);
    if (pAISTarget) ret = GetTargetColumnData(pAISTarget.get(), column);
  }

  return ret;
}

int OCPNListCtrl::OnGetItemColumnImage(long item, long column) const {
  return -1;
}

wxString OCPNListCtrl::GetTargetColumnData(AisTargetData* pAISTarget,
                                           long column) const {
  wxString ret;

  if (pAISTarget) {
    switch (column) {
      case tlTRK:
        if ((pAISTarget->Class == AIS_ATON) ||
            (pAISTarget->Class == AIS_BASE) || (pAISTarget->Class == AIS_METEO))
          ret = _("-");
        else if (pAISTarget->b_show_track && !pAISTarget->b_NoTrack)
          ret = _("Yes");
        else
          ret = _("No");
        break;

      case tlNAME:
        if ((!pAISTarget->b_nameValid && (pAISTarget->Class == AIS_BASE)) ||
            (pAISTarget->Class == AIS_SART))
          ret = _("-");
        else {
          wxString uret = trimAISField(pAISTarget->ShipName);
          if (uret == _T("Unknown"))
            ret = wxGetTranslation(uret);
          else
            ret = uret;

          if (strlen(pAISTarget->ShipNameExtension))
            ret.Append(wxString(pAISTarget->ShipNameExtension, wxConvUTF8));
        }
        break;

      case tlCALL:
        ret = trimAISField(pAISTarget->CallSign);
        break;

      case tlMMSI:
        if (pAISTarget->Class != AIS_GPSG_BUDDY)
          ret.Printf(_T("%09d"), abs(pAISTarget->MMSI));
        else
          ret.Printf(_T("   nil   "));
        break;

      case tlCLASS:
        if (pAISTarget->b_SarAircraftPosnReport) {
          int airtype = (pAISTarget->MMSI % 1000) / 100;
          ret = airtype == 5 ? _("SAR Helicopter") : _("SAR Aircraft");
        } else
          ret = wxGetTranslation(pAISTarget->Get_class_string(true));
        break;

      case tlTYPE:
        if ((pAISTarget->Class == AIS_BASE) ||
            (pAISTarget->Class == AIS_SART) ||
            (pAISTarget->Class == AIS_METEO) ||
            pAISTarget->b_SarAircraftPosnReport)
          ret = _("-");
        else
          ret = wxGetTranslation(pAISTarget->Get_vessel_type_string(false));
        break;

      case tlNAVSTATUS: {
        if (pAISTarget->Class == AIS_SART) {
          if (pAISTarget->NavStatus == RESERVED_14)
            ret = _("Active");
          else if (pAISTarget->NavStatus == UNDEFINED)
            ret = _("Testing");
        } else {
          if ((pAISTarget->NavStatus <= 20) && (pAISTarget->NavStatus >= 0))
            ret = wxGetTranslation(ais_get_status(pAISTarget->NavStatus));
          else
            ret = _("-");
        }

        if ((pAISTarget->Class == AIS_ATON) ||
            (pAISTarget->Class == AIS_BASE) ||
            (pAISTarget->Class == AIS_CLASS_B) ||
            (pAISTarget->Class == AIS_METEO) ||
            pAISTarget->b_SarAircraftPosnReport)
          ret = _("-");
        break;
      }

      case tlBRG: {
        if (pAISTarget->b_positionOnceValid && bGPSValid &&
            (pAISTarget->Brg >= 0.) && (fabs(pAISTarget->Lat) < 85.)) {
          int brg = (int)wxRound(pAISTarget->Brg);
          if (pAISTarget->Brg > 359.5) brg = 0;

          ret.Printf(_T("%03d"), brg);
        } else
          ret = _("-");
        break;
      }

      case tlCOG: {
        if ((pAISTarget->COG >= 360.0) || (pAISTarget->Class == AIS_ATON) ||
            (pAISTarget->Class == AIS_BASE) || (pAISTarget->Class == AIS_METEO))
          ret = _("-");
        else {
          int crs = wxRound(pAISTarget->COG);
          if (crs == 360)
            ret.Printf(_T("  000"));
          else
            ret.Printf(_T("  %03d"), crs);
        }
        break;
      }

      case tlSOG: {
        if (((pAISTarget->SOG > 100.) &&
             !pAISTarget->b_SarAircraftPosnReport) ||
            (pAISTarget->Class == AIS_ATON) ||
            (pAISTarget->Class == AIS_BASE) || (pAISTarget->Class == AIS_METEO))
          ret = _("-");
        else
          ret.Printf(_T("%5.1f"), toUsrSpeed(pAISTarget->SOG));
        break;
      }
      case tlCPA: {
        if ((!pAISTarget->bCPA_Valid) || (pAISTarget->Class == AIS_ATON) ||
            (pAISTarget->Class == AIS_BASE) || (pAISTarget->Class == AIS_METEO))
          ret = _("-");
        else
          ret.Printf(_T("%5.2f"), toUsrDistance(pAISTarget->CPA));
        break;
      }
      case tlTCPA: {
        if ((!pAISTarget->bCPA_Valid) || (pAISTarget->Class == AIS_ATON) ||
            (pAISTarget->Class == AIS_BASE) || (pAISTarget->Class == AIS_METEO))
          ret = _("-");
        else
          ret.Printf(_T("%5.0f"), pAISTarget->TCPA);
        break;
      }
      case tlRNG: {
        if (pAISTarget->b_positionOnceValid && bGPSValid &&
            (pAISTarget->Range_NM >= 0.))
          ret.Printf(_T("%5.2f"), toUsrDistance(pAISTarget->Range_NM));
        else
          ret = _("-");
        break;
      }
      case tlFLAG: {
        ret = wxGetTranslation(pAISTarget->GetCountryCode(true));

        if (ret.IsEmpty()) {
          ret = _("-");
        }
        break;
      }

      default:
        break;
    }
  }

  return ret;
}
