/***************************************************************************
 *   Copyright (C) 2010 - 2025 by David S. Register                        *
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

#include <wx/app.h>

#include "model/cmdline.h"
#include "model/gui_vars.h"

#include "chart_ctx_factory.h"
#include "displays.h"
#include "navutil.h"
#include "ocpn_platform.h"
#include "o_senc.h"
#include "senc_manager.h"
#include "s52plib.h"
#include "s57registrar_mgr.h"

void LoadS57() {
  if (ps52plib)  // already loaded?
    return;

  //  Start a SENC Thread manager
  g_SencThreadManager = new SENCThreadManager();

  //      Set up a useable CPL library error handler for S57 stuff
  // FIXME (dave) Verify after moving LoadS57
  // CPLSetErrorHandler(MyCPLErrorHandler);

  //      Init the s57 chart object, specifying the location of the required csv
  //      files
  g_csv_locn = g_Platform->GetSharedDataDir();
  g_csv_locn.Append("s57data");

  if (g_bportable) {
    g_csv_locn = ".";
    appendOSDirSlash(&g_csv_locn);
    g_csv_locn.Append("s57data");
  }

  //      If the config file contains an entry for SENC file prefix, use it.
  //      Otherwise, default to PrivateDataDir
  if (g_SENCPrefix.IsEmpty()) {
    g_SENCPrefix = g_Platform->GetPrivateDataDir();
    appendOSDirSlash(&g_SENCPrefix);
    g_SENCPrefix.Append("SENC");
  }

  if (g_bportable) {
    wxFileName f(g_SENCPrefix);
    if (f.MakeRelativeTo(g_Platform->GetPrivateDataDir()))
      g_SENCPrefix = f.GetFullPath();
    else
      g_SENCPrefix = "SENC";
  }

  //      If the config file contains an entry for PresentationLibraryData, use
  //      it. Otherwise, default to conditionally set spot under g_pcsv_locn
  wxString plib_data;
  bool b_force_legacy = false;

  if (g_UserPresLibData.IsEmpty()) {
    plib_data = g_csv_locn;
    appendOSDirSlash(&plib_data);
    plib_data.Append("S52RAZDS.RLE");
  } else {
    plib_data = g_UserPresLibData;
    b_force_legacy = true;
  }

  ps52plib = new s52plib(plib_data, b_force_legacy);

  //  If the library load failed, try looking for the s57 data elsewhere

  //  First, look in UserDataDir
  /*    From wxWidgets documentation

   wxStandardPaths::GetUserDataDir
   wxString GetUserDataDir() const
   Return the directory for the user-dependent application data files:
   * Unix: ~/.appname
   * Windows: C:\Documents and Settings\username\Application Data\appname
   * Mac: ~/Library/Application Support/appname
   */

  if (!ps52plib->m_bOK) {
    delete ps52plib;

    wxStandardPaths &std_path = g_Platform->GetStdPaths();

    wxString look_data_dir;
    look_data_dir.Append(std_path.GetUserDataDir());
    appendOSDirSlash(&look_data_dir);
    wxString tentative_SData_Locn = look_data_dir;
    look_data_dir.Append("s57data");

    plib_data = look_data_dir;
    appendOSDirSlash(&plib_data);
    plib_data.Append("S52RAZDS.RLE");

    wxLogMessage("Looking for s57data in " + look_data_dir);
    ps52plib = new s52plib(plib_data);

    if (ps52plib->m_bOK) {
      g_csv_locn = look_data_dir;
      ///???            g_SData_Locn = tentative_SData_Locn;
    }
  }

  //  And if that doesn't work, look again in the original SData Location
  //  This will cover the case in which the .ini file entry is corrupted or
  //  moved

  if (!ps52plib->m_bOK) {
    delete ps52plib;

    wxString look_data_dir;
    look_data_dir = g_Platform->GetSharedDataDir();
    look_data_dir.Append("s57data");

    plib_data = look_data_dir;
    appendOSDirSlash(&plib_data);
    plib_data.Append("S52RAZDS.RLE");

    wxLogMessage("Looking for s57data in " + look_data_dir);
    ps52plib = new s52plib(plib_data);

    if (ps52plib->m_bOK) g_csv_locn = look_data_dir;
  }

  if (ps52plib->m_bOK) {
    wxLogMessage("Using s57data in " + g_csv_locn);
    m_pRegistrarMan =
        new s57RegistrarMgr(g_csv_locn, g_Platform->GetLogFilePtr());

    //    Preset some object class visibilites for "User Standard" disply
    //    category
    //  They may be overridden in LoadS57Config
    for (unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount();
         iPtr++) {
      OBJLElement *pOLE = (OBJLElement *)(ps52plib->pOBJLArray->Item(iPtr));
      if (!strncmp(pOLE->OBJLName, "DEPARE", 6)) pOLE->nViz = 1;
      if (!strncmp(pOLE->OBJLName, "LNDARE", 6)) pOLE->nViz = 1;
      if (!strncmp(pOLE->OBJLName, "COALNE", 6)) pOLE->nViz = 1;
    }

    pConfig->LoadS57Config();
    ps52plib->SetPLIBColorScheme(global_color_scheme, ChartCtxFactory());

    if (wxTheApp->GetTopWindow()) {
      ps52plib->SetDisplayWidth(g_monitor_info[g_current_monitor].width);
      ps52plib->SetPPMM(g_BasePlatform->GetDisplayDPmm());
      double dip_factor =
          g_BasePlatform->GetDisplayDIPMult(wxTheApp->GetTopWindow());
      ps52plib->SetDIPFactor(dip_factor);
      ps52plib->SetContentScaleFactor(OCPN_GetDisplayContentScaleFactor());
    }

    // preset S52 PLIB scale factors
    ps52plib->SetScaleFactorExp(
        g_Platform->GetChartScaleFactorExp(g_ChartScaleFactor));
    ps52plib->SetScaleFactorZoomMod(g_chart_zoom_modifier_vector);

#ifdef ocpnUSE_GL

    // Setup PLIB OpenGL options, if enabled
    if (g_bopengl) {
      if (GL_Caps) {
        wxString renderer = wxString(GL_Caps->Renderer.c_str());
        ps52plib->SetGLRendererString(renderer);
      }

      ps52plib->SetGLOptions(
          glChartCanvas::s_b_useStencil, glChartCanvas::s_b_useStencilAP,
          glChartCanvas::s_b_useScissorTest, glChartCanvas::s_b_useFBO,
          g_b_EnableVBO, g_texture_rectangle_format, 1, 1);
    }
#endif

  } else {
    wxLogMessage("   S52PLIB Initialization failed, disabling Vector charts.");
    delete ps52plib;
    ps52plib = NULL;
  }
}
