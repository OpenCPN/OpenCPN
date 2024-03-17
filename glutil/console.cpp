/******************************************************************************
 *
 * Project:  OpenCPN
 *
 * Purpose:  Simple CLI application to check OpenGL capabilities.
 *
 ***************************************************************************
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "config.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <fstream>

#if defined(__MSVC__)
#include "glew.h"
#elif defined(__WXOSX__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
typedef void (*_GLUfuncptr)();
#define GL_COMPRESSED_RGB_FXT1_3DFX 0x86B0
#elif defined(__WXQT__) || defined(__WXGTK__)
#include <GL/glew.h>
#include <GL/glu.h>
// #include <GL/glut.h>
#endif

#include <wx/cmdline.h>
#include <wx/json_defs.h>
#include <wx/jsonwriter.h>
#include <wx/glcanvas.h>
#include <wx/tokenzr.h>

static const char* USAGE = R"""(
Usage: opencpn-cli [options] <command> [arguments]

Options:

  -h, --help
      Show the help information.
  -d, --debug
      Print debug messages.
  -v, --verbose
      Verbose output.

Commands:
  opengl-info:
      Try to collect information about OpenGL subsystem and output it in JSON format.
)""";

using namespace std;

class TestGLCanvas : public wxGLCanvas {
public:
  TestGLCanvas(wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxPoint& pos = wxPoint(0, 0),
               const wxSize& size = wxSize(0, 0), long style = 0,
               const wxString& name = "OpenCPNGLUtilAppTestCanvas");
  TestGLCanvas(const TestGLCanvas&) = delete;
  TestGLCanvas& operator=(const TestGLCanvas&) = delete;

  ~TestGLCanvas() final;

  wxGLContext* m_glRC;

private:
  void InitGL();
};

TestGLCanvas::TestGLCanvas(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                           const wxSize& size, long style, const wxString& name)
    : wxGLCanvas(parent, id, nullptr, pos, size,
                 style | wxFULL_REPAINT_ON_RESIZE, name) {
  // Explicitly create a new rendering context instance for this canvas.
  m_glRC = new wxGLContext(this);
}

TestGLCanvas::~TestGLCanvas() { delete m_glRC; }

class GLUtilApp : public wxApp {
public:
  GLUtilApp() : wxApp() {
#ifdef __linux__
    // Handle e. g., wayland default display -- see #1166.
    if (wxGetEnv("WAYLAND_DISPLAY", nullptr)) setenv("GDK_BACKEND", "x11", 1);
#endif  // __linux__
    CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "program");
    SetAppName("opencpn-glutil");
  }

  void OnInitCmdLine(wxCmdLineParser& parser) override {
    parser.AddSwitch("h", "help", "Print help");
    parser.AddSwitch("v", "verbose", "Verbose output"); // Actually not used, but prevents wxWidgets from asserting 
    parser.AddSwitch("d", "debug", "Debug output");
    parser.AddParam("<command>", wxCMD_LINE_VAL_STRING,
                    wxCMD_LINE_PARAM_OPTIONAL);
    parser.AddParam("[arg]", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);
    parser.AddParam("[arg]", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

    delete wxLog::SetActiveTarget(new wxLogStderr(nullptr));
    wxLog::SetTimestamp("");
    wxLog::SetLogLevel(wxLOG_Warning);
  }

  bool OnInit() final {
    if (!wxApp::OnInit()) {
      return false;
    }
    return true;
  }

  int OnExit() final {
    delete wxLog::SetActiveTarget(nullptr);
    return wxApp::OnExit();
  }

  GLboolean QueryExtension(const char* extName) const {
    size_t extNameLen;

    extNameLen = strlen(extName);

    auto p = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    if (nullptr == p) {
      return GL_FALSE;
    }

    const char* end = p + strlen(p);

    while (p < end) {
      size_t n = strcspn(p, " ");
      if ((extNameLen == n) && (strncmp(extName, p, n) == 0)) {
        return GL_TRUE;
      }
      p += (n + 1);
    }
    return GL_FALSE;
  }

  void opengl_info(std::ostream & output) const {
    using namespace std;
    wxJSONWriter w;
    wxString out;
    wxJSONValue v;

    // Avoids wxGLCanvas, but not portable - eg. no glut easily available on
    // Windows
    // int argc = 0;
    // char **argv = 0;
    // glutInit(&argc, argv);
    // auto winid = glutCreateWindow("GLUT");
    wxLogDebug("Creating frame");
    wxFrame frame(nullptr, wxID_ANY, "GLCanvas Test", wxPoint(0, 0),
                   wxSize(0, 0));
    wxLogDebug("Showing frame");
    frame.Show(true);
    wxYield();
    wxLogDebug("Creating canvas");
    TestGLCanvas canvas(&frame, wxID_ANY, wxPoint(0, 0),
                                   frame.GetClientSize());
    wxYield();
    canvas.SetCurrent(*canvas.m_glRC);
    wxLogDebug("Collecting information");
#ifndef __WXOSX__
    auto ret = glewInit();
    v["GL_ERROR_STRING"] = wxString(glewGetErrorString(ret));
    v["GL_ERROR"] = glGetError();
    if (ret != GLEW_OK) {
      v["GL_USABLE"] = false;
      w.Write(v, out);
      cout << out << endl;
      exit(1);
    }
#endif
    v["GL_USABLE"] = true;
    v["GL_VERSION"] = wxString(glGetString(GL_VERSION));
    v["GL_RENDERER"] = wxString(glGetString(GL_RENDERER));
    v["GL_VENDOR"] = wxString(glGetString(GL_VENDOR));
    v["GL_SHADING_LANGUAGE_VERSION"] =
        wxString(glGetString(GL_SHADING_LANGUAGE_VERSION));
    v["GL_ARB_texture_non_power_of_two"] =
        QueryExtension("GL_ARB_texture_non_power_of_two");
    v["GL_OES_texture_npot"] = QueryExtension("GL_OES_texture_npot");
    v["GL_ARB_texture_rectangle"] = QueryExtension("GL_ARB_texture_rectangle");
    v["GL_EXT_framebuffer_object"] =
        QueryExtension("GL_EXT_framebuffer_object");
    wxStringTokenizer tkz(wxString(glGetString(GL_EXTENSIONS)), " ");
    while (tkz.HasMoreTokens())
    {
      v["GL_EXTENSIONS"].Append(tkz.GetNextToken());
    }
#ifdef __WXOSX__
    v["GL_ERROR"] = glGetError();
    v["GL_ERROR_STRING"] = wxString("No error");
#endif
    w.Write(v, out);
    output << out << endl;
    frame.Close();
    // glutDestroyWindow(winid);
  }

  void check_param_count(const wxCmdLineParser& parser, size_t count) const {
    if (parser.GetParamCount() < count) {
      std::cerr << USAGE << std::endl;
      exit(1);
    }
  }

  bool OnCmdLineParsed(wxCmdLineParser& parser) override {
    if (auto initializer = wxInitializer{}; !initializer) {
      std::cerr << "Failed to initialize the wxWidgets library, aborting.";
      exit(1);
    }
    wxAppConsole::OnCmdLineParsed(parser);
    if (argc == 1) {
      std::cout << "OpenCPN OpenGL Utility application. Use -h for help" << std::endl;
      exit(0);
    }
    wxString option_val;
    if (parser.Found("debug")) {
      wxLog::SetLogLevel(wxLOG_Debug);
    }
    if (parser.Found("help")) {
      std::cout << USAGE << std::endl;
      exit(0);
    }
    if (parser.GetParamCount() < 1) {
      std::cerr << USAGE << std::endl;
      exit(1);
    }
    if (const std::string command = parser.GetParam(0).ToStdString(); command == "opengl-info") {
      if (parser.GetParamCount() > 1) {
        std::ofstream myfile;
        myfile.open(parser.GetParam(1).ToStdString(), std::ios::out | std::ios::trunc);
        if (myfile.is_open()) {
          opengl_info(myfile);
          myfile.close();
        } else {
          std::cerr << "ERROR: Can't open " << parser.GetParam(1).ToStdString() << std::endl;
          exit(2);
        }
      } else {
        opengl_info(std::cout);
      }
    } else {
      std::cerr << USAGE << std::endl;
      exit(2);
    }
    exit(0);
  }
};

wxIMPLEMENT_APP(GLUtilApp);
