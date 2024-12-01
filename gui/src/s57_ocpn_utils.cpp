/***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   Copyright (C) 2024 Alec Leamas                                        *
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

/** \file s57_ocpn_util.cpp Implement s57_ocpn_util.h */

#include "s57_ocpn_utils.h"

#include "chartimg.h"
#include "chcanv.h"
#include "ocpn_plugin.h"
#include "pluginmanager.h"
#include "Quilt.h"
#include "s52plib.h"

extern PlugInManager *g_pi_manager;  // FIXME: MOve to header
extern s52plib *ps52plib;            // FIXME: MOve to header

bool s57_GetVisibleLightSectors(ChartCanvas *cc, double lat, double lon,
                                ViewPort &viewport,
                                std::vector<s57Sector_t> &sectorlegs) {
  if (!cc) return false;

  static float lastLat, lastLon;

  if (!ps52plib) return false;

  ChartPlugInWrapper *target_plugin_chart = NULL;
  s57chart *Chs57 = NULL;

  // Find the chart that is currently shown at the given lat/lon
  wxPoint calcPoint = viewport.GetPixFromLL(lat, lon);
  ChartBase *target_chart;
  if (cc->m_singleChart &&
      (cc->m_singleChart->GetChartFamily() == CHART_FAMILY_VECTOR))
    target_chart = cc->m_singleChart;
  else if (viewport.b_quilt)
    target_chart = cc->m_pQuilt->GetChartAtPix(viewport, calcPoint);
  else
    target_chart = NULL;

  if (target_chart) {
    if ((target_chart->GetChartType() == CHART_TYPE_PLUGIN) &&
        (target_chart->GetChartFamily() == CHART_FAMILY_VECTOR))
      target_plugin_chart = dynamic_cast<ChartPlugInWrapper *>(target_chart);
    else
      Chs57 = dynamic_cast<s57chart *>(target_chart);
  }

  bool newSectorsNeedDrawing = false;

  if (target_plugin_chart || Chs57) {
    ListOfObjRazRules *rule_list = NULL;
    ListOfPI_S57Obj *pi_rule_list = NULL;

    // Go get the array of all objects at the cursor lat/lon
    float selectRadius = 16 / (viewport.view_scale_ppm * 1852 * 60);

    if (Chs57)
      rule_list =
          Chs57->GetLightsObjRuleListVisibleAtLatLon(lat, lon, &viewport);
    else if (target_plugin_chart)
      pi_rule_list = g_pi_manager->GetLightsObjRuleListVisibleAtLatLon(
          target_plugin_chart, lat, lon, viewport);

    newSectorsNeedDrawing = s57_ProcessExtendedLightSectors(
        cc, target_plugin_chart, Chs57, rule_list, pi_rule_list, sectorlegs);

    if (rule_list) {
      rule_list->Clear();
      delete rule_list;
    }

    if (pi_rule_list) {
      pi_rule_list->Clear();
      delete pi_rule_list;
    }
  }

  return newSectorsNeedDrawing;
}

bool s57_ProcessExtendedLightSectors(ChartCanvas *cc,
                                     ChartPlugInWrapper *target_plugin_chart,
                                     s57chart *Chs57,
                                     ListOfObjRazRules *rule_list,
                                     ListOfPI_S57Obj *pi_rule_list,
                                     std::vector<s57Sector_t> &sectorlegs) {
  bool newSectorsNeedDrawing = false;

  bool bhas_red_green = false;
  bool bleading_attribute = false;

  int opacity = 100;
  if (cc->GetColorScheme() == GLOBAL_COLOR_SCHEME_DUSK) opacity = 50;
  if (cc->GetColorScheme() == GLOBAL_COLOR_SCHEME_NIGHT) opacity = 20;

  int yOpacity = (float)opacity *
                 1.3;  // Matched perception of white/yellow with red/green

  if (target_plugin_chart || Chs57) {
    sectorlegs.clear();

    wxPoint2DDouble objPos;

    char *curr_att = NULL;
    int n_attr = 0;
    wxArrayOfS57attVal *attValArray = NULL;

    ListOfObjRazRules::Node *snode = NULL;
    ListOfPI_S57Obj::Node *pnode = NULL;

    if (Chs57 && rule_list)
      snode = rule_list->GetLast();
    else if (target_plugin_chart && pi_rule_list)
      pnode = pi_rule_list->GetLast();

    while (1) {
      wxPoint2DDouble lightPosD(0, 0);
      bool is_light = false;
      if (Chs57) {
        if (!snode) break;

        ObjRazRules *current = snode->GetData();
        S57Obj *light = current->obj;
        if (!strcmp(light->FeatureName, "LIGHTS")) {
          objPos = wxPoint2DDouble(light->m_lat, light->m_lon);
          curr_att = light->att_array;
          n_attr = light->n_attr;
          attValArray = light->attVal;
          is_light = true;
        }
      } else if (target_plugin_chart) {
        if (!pnode) break;
        PI_S57Obj *light = pnode->GetData();
        if (!strcmp(light->FeatureName, "LIGHTS")) {
          objPos = wxPoint2DDouble(light->m_lat, light->m_lon);
          curr_att = light->att_array;
          n_attr = light->n_attr;
          attValArray = light->attVal;
          is_light = true;
        }
      }

      //  Ready to go
      int attrCounter;
      double sectr1 = -1;
      double sectr2 = -1;
      double valnmr = -1;
      wxString curAttrName;
      wxColor color;

      if (lightPosD.m_x == 0 && lightPosD.m_y == 0.0) lightPosD = objPos;

      if (is_light && (lightPosD == objPos)) {
        if (curr_att) {
          bool bviz = true;

          attrCounter = 0;
          int noAttr = 0;
          s57Sector_t sector;

          bleading_attribute = false;

          while (attrCounter < n_attr) {
            curAttrName = wxString(curr_att, wxConvUTF8, 6);
            noAttr++;

            S57attVal *pAttrVal = NULL;
            if (attValArray) {
              if (Chs57)
                pAttrVal = attValArray->Item(attrCounter);
              else if (target_plugin_chart)
                pAttrVal = attValArray->Item(attrCounter);
            }

            wxString value =
                s57chart::GetAttributeValueAsString(pAttrVal, curAttrName);

            if (curAttrName == _T("LITVIS")) {
              if (value.StartsWith(_T("obsc"))) bviz = false;
            }
            if (curAttrName == _T("SECTR1")) value.ToDouble(&sectr1);
            if (curAttrName == _T("SECTR2")) value.ToDouble(&sectr2);
            if (curAttrName == _T("VALNMR")) value.ToDouble(&valnmr);
            if (curAttrName == _T("COLOUR")) {
              if (value == _T("red(3)")) {
                color = wxColor(255, 0, 0, opacity);
                sector.iswhite = false;
                bhas_red_green = true;
              }

              if (value == _T("green(4)")) {
                color = wxColor(0, 255, 0, opacity);
                sector.iswhite = false;
                bhas_red_green = true;
              }
            }

            if (curAttrName == _T("EXCLIT")) {
              if (value.Find(_T("(3)"))) valnmr = 1.0;  // Fog lights.
            }

            if (curAttrName == _T("CATLIT")) {
              if (value.Upper().StartsWith(_T("DIRECT")) ||
                  value.Upper().StartsWith(_T("LEAD")))
                bleading_attribute = true;
            }

            attrCounter++;
            curr_att += 6;
          }

          if ((sectr1 >= 0) && (sectr2 >= 0)) {
            if (sectr1 > sectr2) {  // normalize
              sectr2 += 360.0;
            }

            sector.pos.m_x = objPos.m_y;  // lon
            sector.pos.m_y = objPos.m_x;

            sector.range =
                (valnmr > 0.0) ? valnmr : 2.5;  // Short default range.
            sector.sector1 = sectr1;
            sector.sector2 = sectr2;

            if (!color.IsOk()) {
              color = wxColor(255, 255, 0, yOpacity);
              sector.iswhite = true;
            }
            sector.color = color;
            sector.isleading = false;  // tentative judgment, check below

            if (bleading_attribute) sector.isleading = true;

            bool newsector = true;
            for (unsigned int i = 0; i < sectorlegs.size(); i++) {
              if (sectorlegs[i].pos == sector.pos &&
                  sectorlegs[i].sector1 == sector.sector1 &&
                  sectorlegs[i].sector2 == sector.sector2) {
                newsector = false;
                //  In the case of duplicate sectors, choose the instance with
                //  largest range. This applies to the case where day and night
                //  VALNMR are different, and so makes the vector result
                //  independent of the order of day/night light features.
                sectorlegs[i].range = wxMax(sectorlegs[i].range, sector.range);
              }
            }

            if (!bviz) newsector = false;

            if ((sector.sector2 == 360) && (sector.sector1 == 0))  // FS#1437
              newsector = false;

            if (newsector) {
              sectorlegs.push_back(sector);
              newSectorsNeedDrawing = true;
            }
          }
        }
      }

      if (Chs57)
        snode = snode->GetPrevious();
      else if (target_plugin_chart)
        pnode = pnode->GetPrevious();

    }  // end of while
  }

  //  Work with the sector legs vector to identify  and mark "Leading Lights"
  //  Sectors with CATLIT "Leading" or "Directional" attribute set have already
  //  been marked
  for (unsigned int i = 0; i < sectorlegs.size(); i++) {
    if (((sectorlegs[i].sector2 - sectorlegs[i].sector1) < 15)) {
      if (sectorlegs[i].iswhite && bhas_red_green)
        sectorlegs[i].isleading = true;
    }
  }

  return newSectorsNeedDrawing;
}

void s57_DrawExtendedLightSectors(ocpnDC &dc, ViewPort &viewport,
                                  std::vector<s57Sector_t> &sectorlegs) {
  float rangeScale = 0.0;

  if (sectorlegs.size() > 0) {
    std::vector<int> sectorangles;
    for (unsigned int i = 0; i < sectorlegs.size(); i++) {
      if (fabs(sectorlegs[i].sector1 - sectorlegs[i].sector2) < 0.3) continue;

      double endx, endy;
      ll_gc_ll(sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x,
               sectorlegs[i].sector1 + 180.0, sectorlegs[i].range, &endy,
               &endx);

      wxPoint end1 = viewport.GetPixFromLL(endy, endx);

      ll_gc_ll(sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x,
               sectorlegs[i].sector2 + 180.0, sectorlegs[i].range, &endy,
               &endx);

      wxPoint end2 = viewport.GetPixFromLL(endy, endx);

      wxPoint lightPos =
          viewport.GetPixFromLL(sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x);

      // Make sure arcs are well inside viewport.
      float rangePx = sqrtf(powf((float)(lightPos.x - end1.x), 2) +
                            powf((float)(lightPos.y - end1.y), 2));
      rangePx /= 3.0;
      if (rangeScale == 0.0) {
        rangeScale = 1.0;
        if (rangePx > viewport.pix_height / 3) {
          rangeScale *= (viewport.pix_height / 3) / rangePx;
        }
      }

      rangePx = rangePx * rangeScale;

      int penWidth = rangePx / 8;
      penWidth = wxMin(20, penWidth);
      penWidth = wxMax(5, penWidth);

      int legOpacity;
      wxPen *arcpen = wxThePenList->FindOrCreatePen(sectorlegs[i].color,
                                                    penWidth, wxPENSTYLE_SOLID);
      arcpen->SetCap(wxCAP_BUTT);
      dc.SetPen(*arcpen);

      float angle1, angle2;
      angle1 = -(sectorlegs[i].sector2 + 90.0) - viewport.rotation * 180.0 / PI;
      angle2 = -(sectorlegs[i].sector1 + 90.0) - viewport.rotation * 180.0 / PI;
      if (angle1 > angle2) {
        angle2 += 360.0;
      }
      int lpx = lightPos.x;
      int lpy = lightPos.y;
      int npoints = 0;
      wxPoint arcpoints[150];  // Size relates to "step" below.

      float step = 3.0;
      while ((step < 15) && ((rangePx * sin(step * PI / 180.)) < 10))
        step += 2.0;  // less points on small arcs

      // Make sure we start and stop exactly on the leg lines.
      int narc = (angle2 - angle1) / step;
      narc++;
      step = (angle2 - angle1) / (float)narc;

      if (sectorlegs[i].isleading && (angle2 - angle1 < 60)) {
        wxPoint yellowCone[3];
        yellowCone[0] = lightPos;
        yellowCone[1] = end1;
        yellowCone[2] = end2;
        arcpen = wxThePenList->FindOrCreatePen(wxColor(0, 0, 0, 0), 1,
                                               wxPENSTYLE_SOLID);
        dc.SetPen(*arcpen);
        wxColor c = sectorlegs[i].color;
        c.Set(c.Red(), c.Green(), c.Blue(), 0.6 * c.Alpha());
        dc.SetBrush(wxBrush(c));
        dc.StrokePolygon(3, yellowCone, 0, 0);
        legOpacity = 50;
      } else {
        for (float a = angle1; a <= angle2 + 0.1; a += step) {
          int x = lpx + (int)(rangePx * cos(a * PI / 180.));
          int y = lpy - (int)(rangePx * sin(a * PI / 180.));
          arcpoints[npoints].x = x;
          arcpoints[npoints].y = y;
          npoints++;
        }
        dc.StrokeLines(npoints, arcpoints);
        legOpacity = 128;
      }

      arcpen = wxThePenList->FindOrCreatePen(wxColor(0, 0, 0, legOpacity), 1,
                                             wxPENSTYLE_SOLID);
      dc.SetPen(*arcpen);

      // Only draw each leg line once.

      bool haveAngle1 = false;
      bool haveAngle2 = false;
      int sec1 = (int)sectorlegs[i].sector1;
      int sec2 = (int)sectorlegs[i].sector2;
      if (sec1 > 360) sec1 -= 360;
      if (sec2 > 360) sec2 -= 360;

      if ((sec2 == 360) && (sec1 == 0))  // FS#1437
        continue;

      for (unsigned int j = 0; j < sectorangles.size(); j++) {
        if (sectorangles[j] == sec1) haveAngle1 = true;
        if (sectorangles[j] == sec2) haveAngle2 = true;
      }

      if (!haveAngle1) {
        dc.StrokeLine(lightPos, end1);
        sectorangles.push_back(sec1);
      }

      if (!haveAngle2) {
        dc.StrokeLine(lightPos, end2);
        sectorangles.push_back(sec2);
      }
    }
  }
}

#ifdef ocpnUSE_GL
void s57_DrawExtendedLightSectorsGL(ocpnDC &dc, ViewPort &viewport,
                                    std::vector<s57Sector_t> &sectorlegs) {
  float rangeScale = 0.0;

  if (sectorlegs.size() > 0) {
    std::vector<int> sectorangles;
    for (unsigned int i = 0; i < sectorlegs.size(); i++) {
      if (fabs(sectorlegs[i].sector1 - sectorlegs[i].sector2) < 0.3) continue;

      double endx, endy;
      ll_gc_ll(sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x,
               sectorlegs[i].sector1 + 180.0, sectorlegs[i].range, &endy,
               &endx);

      wxPoint end1 = viewport.GetPixFromLL(endy, endx);

      ll_gc_ll(sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x,
               sectorlegs[i].sector2 + 180.0, sectorlegs[i].range, &endy,
               &endx);

      wxPoint end2 = viewport.GetPixFromLL(endy, endx);

      wxPoint lightPos =
          viewport.GetPixFromLL(sectorlegs[i].pos.m_y, sectorlegs[i].pos.m_x);

      // Make sure arcs are well inside viewport.
      float rangePx = sqrtf(powf((float)(lightPos.x - end1.x), 2) +
                            powf((float)(lightPos.y - end1.y), 2));
      rangePx /= 3.0;
      if (rangeScale == 0.0) {
        rangeScale = 1.0;
        if (rangePx > viewport.pix_height / 3) {
          rangeScale *= (viewport.pix_height / 3) / rangePx;
        }
      }

      rangePx = rangePx * rangeScale;

      float arcw = rangePx / 10;
      arcw = wxMin(20, arcw);
      arcw = wxMax(5, arcw);

      int legOpacity;

      float angle1, angle2;
      angle1 = -(sectorlegs[i].sector2 + 90.0) - viewport.rotation * 180.0 / PI;
      angle2 = -(sectorlegs[i].sector1 + 90.0) - viewport.rotation * 180.0 / PI;
      if (angle1 > angle2) {
        angle2 += 360.0;
      }
      int lpx = lightPos.x;
      int lpy = lightPos.y;

      if (sectorlegs[i].isleading && (angle2 - angle1 < 60)) {
        wxPoint yellowCone[3];
        yellowCone[0] = lightPos;
        yellowCone[1] = end1;
        yellowCone[2] = end2;
        wxPen *arcpen = wxThePenList->FindOrCreatePen(wxColor(0, 0, 0, 0), 1,
                                                      wxPENSTYLE_SOLID);
        dc.SetPen(*arcpen);
        wxColor c = sectorlegs[i].color;
        c.Set(c.Red(), c.Green(), c.Blue(), 0.6 * c.Alpha());
        dc.SetBrush(wxBrush(c));
        dc.StrokePolygon(3, yellowCone, 0, 0);
        legOpacity = 50;
      } else {
        // Center point
        wxPoint r(lpx, lpy);

        //  radius scaled to display
        float rad = rangePx;

        // float arcw = arc_width * canvas_pix_per_mm;
        //  On larger screens, make the arc_width 1.0 mm
        // if ( m_display_size_mm > 200)     //200 mm, about 8 inches
        // arcw = canvas_pix_per_mm;

        //      Enable anti-aliased lines, at best quality
        glEnable(GL_BLEND);

        float coords[8];
        coords[0] = -rad;
        coords[1] = rad;
        coords[2] = rad;
        coords[3] = rad;
        coords[4] = -rad;
        coords[5] = -rad;
        coords[6] = rad;
        coords[7] = -rad;

        GLShaderProgram *shader = pring_shader_program[0 /*GetCanvasIndex()*/];
        shader->Bind();

        // Get pointers to the attributes in the program.
        GLint mPosAttrib = glGetAttribLocation(shader->programId(), "aPos");

        // Disable VBO's (vertex buffer objects) for attributes.
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 0, coords);
        glEnableVertexAttribArray(mPosAttrib);

        //  Circle radius
        GLint radiusloc =
            glGetUniformLocation(shader->programId(), "circle_radius");
        glUniform1f(radiusloc, rad);

        //  Circle center point, physical
        GLint centerloc =
            glGetUniformLocation(shader->programId(), "circle_center");
        float ctrv[2];
        ctrv[0] = r.x;
        ctrv[1] = viewport.pix_height - r.y;
        glUniform2fv(centerloc, 1, ctrv);

        //  Circle color
        wxColour colorb = sectorlegs[i].color;
        float colorv[4];
        colorv[0] = colorb.Red() / float(256);
        colorv[1] = colorb.Green() / float(256);
        colorv[2] = colorb.Blue() / float(256);
        colorv[3] = colorb.Alpha() / float(256);

        GLint colloc =
            glGetUniformLocation(shader->programId(), "circle_color");
        glUniform4fv(colloc, 1, colorv);

        //  Border color
        float bcolorv[4];
        bcolorv[0] = 0;
        bcolorv[1] = 0;
        bcolorv[2] = 0;
        bcolorv[3] = 0;

        GLint bcolloc =
            glGetUniformLocation(shader->programId(), "border_color");
        glUniform4fv(bcolloc, 1, bcolorv);

        //  Border Width
        GLint borderWidthloc =
            glGetUniformLocation(shader->programId(), "border_width");
        glUniform1f(borderWidthloc, 2);

        //  Ring width
        GLint ringWidthloc =
            glGetUniformLocation(shader->programId(), "ring_width");
        glUniform1f(ringWidthloc, arcw);

        //  Visible sectors, rotated to vp orientation
        float sr1 =
            sectorlegs[i].sector1 + (viewport.rotation * 180 / PI) + 180;
        if (sr1 > 360.) sr1 -= 360.;
        float sr2 =
            sectorlegs[i].sector2 + (viewport.rotation * 180 / PI) + 180;
        if (sr2 > 360.) sr2 -= 360.;

        float sb, se;
        if (sr2 > sr1) {
          sb = sr1;
          se = sr2;
        } else {
          sb = sr1;
          se = sr2 + 360;
        }

        //  Shader can handle angles > 360.
        if ((sb < 0) || (se < 0)) {
          sb += 360.;
          se += 360.;
        }

        GLint sector1loc =
            glGetUniformLocation(shader->programId(), "sector_1");
        glUniform1f(sector1loc, (sb * PI / 180.));
        GLint sector2loc =
            glGetUniformLocation(shader->programId(), "sector_2");
        glUniform1f(sector2loc, (se * PI / 180.));

        // Rotate and translate
        mat4x4 I;
        mat4x4_identity(I);
        mat4x4_translate_in_place(I, r.x, r.y, 0);

        GLint matloc =
            glGetUniformLocation(shader->programId(), "TransformMatrix");
        glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat *)I);

        // Perform the actual drawing.
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Restore the per-object transform to Identity Matrix
        mat4x4 IM;
        mat4x4_identity(IM);
        GLint matlocf =
            glGetUniformLocation(shader->programId(), "TransformMatrix");
        glUniformMatrix4fv(matlocf, 1, GL_FALSE, (const GLfloat *)IM);

        glDisableVertexAttribArray(mPosAttrib);
        shader->UnBind();
      }

      wxPen *arcpen = wxThePenList->FindOrCreatePen(wxColor(0, 0, 0, 128), 1,
                                                    wxPENSTYLE_SOLID);
      dc.SetPen(*arcpen);

      // Only draw each leg line once.
      bool haveAngle1 = false;
      bool haveAngle2 = false;
      int sec1 = (int)sectorlegs[i].sector1;
      int sec2 = (int)sectorlegs[i].sector2;
      if (sec1 > 360) sec1 -= 360;
      if (sec2 > 360) sec2 -= 360;

      if ((sec2 == 360) && (sec1 == 0))  // FS#1437
        continue;

      for (unsigned int j = 0; j < sectorangles.size(); j++) {
        if (sectorangles[j] == sec1) haveAngle1 = true;
        if (sectorangles[j] == sec2) haveAngle2 = true;
      }

      if (!haveAngle1) {
        dc.StrokeLine(lightPos, end1);
        sectorangles.push_back(sec1);
      }

      if (!haveAngle2) {
        dc.StrokeLine(lightPos, end2);
        sectorangles.push_back(sec2);
      }
    }
  }
}

#endif  // ocpnUSE_GL

bool s57_CheckExtendedLightSectors(ChartCanvas *cc, int mx, int my,
                                   ViewPort &viewport,
                                   std::vector<s57Sector_t> &sectorlegs) {
  if (!cc) return false;

  double cursor_lat, cursor_lon;
  static float lastLat, lastLon;

  if (!ps52plib || !ps52plib->m_bExtendLightSectors) return false;

  ChartPlugInWrapper *target_plugin_chart = NULL;
  s57chart *Chs57 = NULL;

  ChartBase *target_chart = cc->GetChartAtCursor();
  if (target_chart) {
    if ((target_chart->GetChartType() == CHART_TYPE_PLUGIN) &&
        (target_chart->GetChartFamily() == CHART_FAMILY_VECTOR))
      target_plugin_chart = dynamic_cast<ChartPlugInWrapper *>(target_chart);
    else
      Chs57 = dynamic_cast<s57chart *>(target_chart);
  }

  cc->GetCanvasPixPoint(mx, my, cursor_lat, cursor_lon);

  if (lastLat == cursor_lat && lastLon == cursor_lon) return false;

  lastLat = cursor_lat;
  lastLon = cursor_lon;
  bool newSectorsNeedDrawing = false;

  if (target_plugin_chart || Chs57) {
    ListOfObjRazRules *rule_list = NULL;
    ListOfPI_S57Obj *pi_rule_list = NULL;

    // Go get the array of all objects at the cursor lat/lon
    float selectRadius = 16 / (viewport.view_scale_ppm * 1852 * 60);

    if (Chs57)
      rule_list = Chs57->GetObjRuleListAtLatLon(
          cursor_lat, cursor_lon, selectRadius, &viewport, MASK_POINT);
    else if (target_plugin_chart)
      pi_rule_list = g_pi_manager->GetPlugInObjRuleListAtLatLon(
          target_plugin_chart, cursor_lat, cursor_lon, selectRadius, viewport);

    newSectorsNeedDrawing = s57_ProcessExtendedLightSectors(
        cc, target_plugin_chart, Chs57, rule_list, pi_rule_list, sectorlegs);

    if (rule_list) {
      rule_list->Clear();
      delete rule_list;
    }

    if (pi_rule_list) {
      pi_rule_list->Clear();
      delete pi_rule_list;
    }
  }

  return newSectorsNeedDrawing;
}
