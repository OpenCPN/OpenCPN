/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  XyGrib model definitions
 * Author:   Ronan Demoment
 *
 ***************************************************************************
 *   Copyright (C) 2024 by R. Demoment                                     *
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
 *
 */

#include <wx/wx.h>
#include "XyGribModelDef.h"

// ******** Atmospheric model definitions **********************************

// GFS Model definition
wxString gfsResolutions[] = {"0.25", "0.5", "1.0"};
wxString gfsReqNames[] = {"gfs_p25_", "gfs_p50_", "gfs_1p0_"};
int gfsInterval[] = {3, 6, 12};
AtmModelDef_t xyGribGFSModelDef = {"GFS",
                                   sizeof(gfsResolutions) / sizeof(wxString),
                                   gfsResolutions,
                                   gfsReqNames,
                                   sizeof(gfsInterval) / sizeof(int),
                                   gfsInterval,
                                   10,
                                   XYGRIB_RUN_ALL,
                                   true,   // Wind
                                   true,   // Gusts
                                   true,   // Pressure
                                   false,  // Alternative pressure
                                   true,   // Air temp
                                   true,   // CAPE
                                   true,   // Reflectivity
                                   true,   // Cloud cover
                                   true};  // Rain fall

// ICON Model definition
wxString iconResolutions[] = {"0.25"};
wxString iconReqNames[] = {"icon_p25_"};
int iconInterval[] = {3, 6, 12};
AtmModelDef_t xyGribICONModelDef = {"ICON",
                                    sizeof(iconResolutions) / sizeof(wxString),
                                    iconResolutions,
                                    iconReqNames,
                                    sizeof(iconInterval) / sizeof(int),
                                    iconInterval,
                                    8,
                                    XYGRIB_RUN_0_12,
                                    true,   // Wind
                                    true,   // Gusts
                                    true,   // Pressure
                                    false,  // Alternative pressure
                                    true,   // Air temp
                                    true,   // CAPE
                                    false,  // Reflectivity
                                    true,   // Cloud cover
                                    true};  // Rain fall

// ARPEGE Model definition
wxString arpegeResolutions[] = {"0.5"};
wxString arpegeReqNames[] = {"arpege_p50_"};
int arpegeInterval[] = {3, 6, 12};
AtmModelDef_t xyGribARPEGEModelDef = {
    "ARPEGE",
    sizeof(arpegeResolutions) / sizeof(wxString),
    arpegeResolutions,
    arpegeReqNames,
    sizeof(arpegeInterval) / sizeof(int),
    arpegeInterval,
    4,
    XYGRIB_RUN_0_12,
    true,   // Wind
    true,   // Gusts
    true,   // Pressure
    false,  // Alternative pressure
    true,   // Air temp
    true,   // CAPE
    false,  // Reflectivity
    true,   // Cloud cover
    true};  // Rain fall

// ECMWF Model definition
wxString ecmwfResolutions[] = {"0.5"};
wxString ecmwfReqNames[] = {"ecmwf_p50_"};
int ecmwfInterval[] = {24};
AtmModelDef_t xyGribECMWFModelDef = {
    "ECMWF",
    sizeof(ecmwfResolutions) / sizeof(wxString),
    ecmwfResolutions,
    ecmwfReqNames,
    sizeof(ecmwfInterval) / sizeof(int),
    ecmwfInterval,
    10,
    XYGRIB_RUN_0_12,
    false,   // Wind
    false,   // Gusts
    false,   // Pressure
    true,    // Alternative pressure
    false,   // Air temp
    false,   // CAPE
    false,   // Reflectivity
    false,   // Cloud cover
    false};  // Rain fall

// ICON-EU Model definition
wxString iconEuResolutions[] = {"0.06"};
wxString iconEuReqNames[] = {"icon_eu_p06_"};
int iconEuInterval[] = {1, 3, 6, 12};
AtmModelDef_t xyGribICONEUModelDef = {
    "ICON-EU",
    sizeof(iconEuResolutions) / sizeof(wxString),
    iconEuResolutions,
    iconEuReqNames,
    sizeof(iconEuInterval) / sizeof(int),
    iconEuInterval,
    4,
    XYGRIB_RUN_ALL,
    true,   // Wind
    true,   // Gusts
    true,   // Pressure
    false,  // Alternative pressure
    true,   // Air temp
    true,   // CAPE
    false,  // Reflectivity
    true,   // Cloud cover
    true};  // Rain fall

// AREPGE-HD Model definition
wxString arpegeHDResolutions[] = {"0.06"};
wxString arpegeHDReqNames[] = {"arpege_eu_p10_"};
int arpegeHDInterval[] = {1, 3, 6, 12};
AtmModelDef_t xyGribARPEGEHDModelDef = {
    "ARPEGE-HD",
    sizeof(arpegeHDResolutions) / sizeof(wxString),
    arpegeHDResolutions,
    arpegeHDReqNames,
    sizeof(arpegeHDInterval) / sizeof(int),
    arpegeHDInterval,
    4,
    XYGRIB_RUN_ALL,
    true,   // Wind
    true,   // Gusts
    true,   // Pressure
    false,  // Alternative pressure
    true,   // Air temp
    false,  // CAPE
    false,  // Reflectivity
    true,   // Cloud cover
    true};  // Rain fall

// AROME Model definition
wxString aromeResolutions[] = {"0.025"};
wxString aromeReqNames[] = {"arome_p025_"};
int aromeInterval[] = {1, 3, 6, 12};
AtmModelDef_t xyGribAROMEModelDef = {
    "AROME",
    sizeof(aromeResolutions) / sizeof(wxString),
    aromeResolutions,
    aromeReqNames,
    sizeof(aromeInterval) / sizeof(int),
    aromeInterval,
    2,
    XYGRIB_RUN_ALL,
    true,   // Wind
    true,   // Gusts
    true,   // Pressure
    false,  // Alternative pressure
    true,   // Air temp
    false,  // CAPE
    false,  // Reflectivity
    true,   // Cloud cover
    true};  // Rain fall

// NAM-CONUS Model definition
wxString namConusResolutions[] = {"0.11"};
wxString namConusReqNames[] = {"nam_conus_12km_"};
int namConusInterval[] = {1, 3, 6, 12};
AtmModelDef_t xyGribNAMCONUSModelDef = {
    "NAM-CONUS",
    sizeof(namConusResolutions) / sizeof(wxString),
    namConusResolutions,
    namConusReqNames,
    sizeof(namConusInterval) / sizeof(int),
    namConusInterval,
    2,
    XYGRIB_RUN_ALL,
    true,   // Wind
    true,   // Gusts
    true,   // Pressure
    false,  // Alternative pressure
    true,   // Air temp
    true,   // CAPE
    true,   // Reflectivity
    true,   // Cloud cover
    true};  // Rain fall

// NAM-CACBN Model definition
wxString namCacbnResolutions[] = {"0.11"};
wxString namCacbnReqNames[] = {"nam_cacbn_12km_"};
int namCacbnInterval[] = {3};
AtmModelDef_t xyGribNAMCACBNModelDef = {
    "NAM-CACBN",
    sizeof(namCacbnResolutions) / sizeof(wxString),
    namCacbnResolutions,
    namCacbnReqNames,
    sizeof(namCacbnInterval) / sizeof(int),
    namCacbnInterval,
    4,
    XYGRIB_RUN_ALL,
    true,   // Wind
    false,  // Gusts
    true,   // Pressure
    false,  // Alternative pressure
    true,   // Air temp
    true,   // CAPE
    true,   // Reflectivity
    true,   // Cloud cover
    true};  // Rain fall

// NAM-PACIFIC Model definition
wxString namPacificResolutions[] = {"0.11"};
wxString namPacificReqNames[] = {"nam_pacific_12km_"};
int namPacificInterval[] = {3};
AtmModelDef_t xyGribNAMPACIFICModelDef = {
    "NAM-PACIFIC",
    sizeof(namPacificResolutions) / sizeof(wxString),
    namPacificResolutions,
    namPacificReqNames,
    sizeof(namPacificInterval) / sizeof(int),
    namPacificInterval,
    4,
    XYGRIB_RUN_ALL,
    true,   // Wind
    false,  // Gusts
    true,   // Pressure
    false,  // Alternative pressure
    true,   // Air temp
    true,   // CAPE
    true,   // Reflectivity
    true,   // Cloud cover
    true};  // Rain fall

AtmModelDef_t *xygribAtmModelList[] = {&xyGribGFSModelDef,
                                       &xyGribICONModelDef,
                                       &xyGribARPEGEModelDef,
                                       &xyGribECMWFModelDef,
                                       &xyGribICONEUModelDef,
                                       &xyGribARPEGEHDModelDef,
                                       &xyGribAROMEModelDef,
                                       &xyGribNAMCONUSModelDef,
                                       &xyGribNAMCACBNModelDef,
                                       &xyGribNAMPACIFICModelDef,
                                       nullptr};

// ******** Wave model definitions *****************************************

WaveModelDef_t xyGribWW3ModelDef = {"WW3", "ww3_p50_",
                                    true,   // Significant height
                                    true};  // Wind waves

WaveModelDef_t xyGribGWAMModelDef = {"GWAM", "gwam_p25_",
                                     true,   // Significant height
                                     true};  // Wind waves

WaveModelDef_t xyGribEWAMModelDef = {"EWAM", "ewam_p05_",
                                     true,   // Significant height
                                     true};  // Wind waves

WaveModelDef_t *xygribWaveModelList[] = {
    &xyGribWW3ModelDef, &xyGribGWAMModelDef, &xyGribEWAMModelDef, nullptr};