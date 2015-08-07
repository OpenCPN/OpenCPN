/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin Freinds
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2014 by David S. Register                               *
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

#include <map>

#include <wx/geometry.h>

#ifdef __OCPN__ANDROID__
#include <qopengl.h>
#include "GL/gl_private.h"
#else
#include "GL/gl.h"
#endif


#include "TexFont.h"

//----------------------------------------------------------------------------------------------------------
//    Grib Overlay Specification
//----------------------------------------------------------------------------------------------------------

class GribOverlay {
public:
    GribOverlay( void )
    {
        m_iTexture = 0;
        m_pDCBitmap = NULL, m_pRGBA = NULL;
    }

    ~GribOverlay( void )
    {
#ifdef ocpnUSE_GL
        if(m_iTexture) 
        {
          glDeleteTextures( 1, &m_iTexture );
        }
#endif
        delete m_pDCBitmap, delete[] m_pRGBA;
    }

    unsigned int m_iTexture; /* opengl mode */

    wxBitmap *m_pDCBitmap; /* dc mode */
    unsigned char *m_pRGBA;

    int m_width;
    int m_height;

    double m_dwidth, m_dheight;
};

#define MAX_PARTICLE_HISTORY 8
#include <list>
struct Particle {
    int m_Duration;

    // history is a ringbuffer.. because so many particles are
    // used, it is a slight optimization over std::list
    int m_HistoryPos, m_HistorySize, m_Run;
    struct ParticleNode {
        float m_Pos[2];
        float m_Screen[2];
        wxUint8 m_Color[3];
    } m_History[MAX_PARTICLE_HISTORY];
};

struct ParticleMap {
public:
    ParticleMap(int settings)
    : m_Setting(settings), history_size(0), array_size(0),
      color_array(NULL), vertex_array(NULL) 
    {
       // XXX should be done in default PlugIn_ViewPort CTOR
        last_viewport.bValid = false;
    }

    ~ParticleMap() {
        delete [] color_array;
        delete [] vertex_array;
    }

    std::list<Particle> m_Particles;

    // particles are rebuilt whenever any of these fields change
    time_t m_Reference_Time;
    int m_Setting;
    int history_size;

    unsigned int array_size;
    unsigned char *color_array;
    float *vertex_array;

    PlugIn_ViewPort last_viewport;
};

class LineBuffer {
public:
    LineBuffer() { count = 0; lines = NULL; }
    ~LineBuffer() { delete [] lines; }

    void pushLine( float x0, float y0, float x1, float y1 );
    void pushPetiteBarbule( int b );
    void pushGrandeBarbule( int b );
    void pushTriangle( int b );
    void Finalize();

    int count;
    float *lines;

private:
    std::list <float> buffer;
};

//----------------------------------------------------------------------------------------------------------
//    Grib Overlay Factory Specification
//----------------------------------------------------------------------------------------------------------

class GRIBUICtrlBar;
class GribRecord;
class GribTimelineRecordSet;

class GRIBOverlayFactory : public wxEvtHandler {
public:
    GRIBOverlayFactory( GRIBUICtrlBar &dlg );
    ~GRIBOverlayFactory();

    void SetSettings( bool hiDefGraphics, bool GradualColors )
    {
      m_hiDefGraphics = hiDefGraphics;
      m_bGradualColors = GradualColors;
      ClearCachedData();
    }

    void SetMessage( wxString message ) { m_Message = message; }
    void SetTimeZone( int TimeZone ) { m_TimeZone = TimeZone; }
    void SetParentSize( int w, int h ) { m_ParentSize.SetWidth(w) ; m_ParentSize.SetHeight(h) ;}

    void SetGribTimelineRecordSet( GribTimelineRecordSet *pGribTimelineRecordSet1 );
    bool RenderGribOverlay( wxDC &dc, PlugIn_ViewPort *vp );
    bool RenderGLGribOverlay( wxGLContext *pcontext, PlugIn_ViewPort *vp );

    void Reset();
    void ClearCachedData( void );
    void ClearCachedLabel( void ) { m_labelCache.clear(); }
    void ClearParticles() { delete m_ParticleMap; m_ParticleMap = NULL; }

    GribTimelineRecordSet *m_pGribTimelineRecordSet;

    void DrawMessageZoomOut( PlugIn_ViewPort *vp );
    wxColour GetGraphicColor(int config, double val);

    wxSize  m_ParentSize;

private:
    void InitColorsTable( );

    void SettingsIdToGribId(int i, int &idx, int &idy, bool &polar);
    bool DoRenderGribOverlay( PlugIn_ViewPort *vp );
    void RenderGribBarbedArrows( int config, GribRecord **pGR, PlugIn_ViewPort *vp );
    void RenderGribIsobar( int config, GribRecord **pGR, wxArrayPtrVoid **pIsobarArray, PlugIn_ViewPort *vp );
    void RenderGribDirectionArrows( int config, GribRecord **pGR, PlugIn_ViewPort *vp );
    void RenderGribOverlayMap( int config, GribRecord **pGR, PlugIn_ViewPort *vp);
    void RenderGribNumbers( int config, GribRecord **pGR, PlugIn_ViewPort *vp );
    void RenderGribParticles( int settings, GribRecord **pGR, PlugIn_ViewPort *vp );
    void DrawLineBuffer(LineBuffer &buffer);
    void OnParticleTimer( wxTimerEvent & event );

    wxString GetRefString( GribRecord *rec, int map );
    void DrawMessageWindow( wxString msg, int x, int y , wxFont *mfont);

    void drawDoubleArrow( int x, int y, double ang, wxColour arrowColor, int arrowWidth, int arrowSizeIdx );
    void drawSingleArrow( int x, int y, double ang, wxColour arrowColor, int arrowWidth, int arrowSizeIdx );
    void drawWindArrowWithBarbs( int settings, int x, int y, double vkn, double ang,
                                 bool south, wxColour arrowColor, double rotate_angle );
    void drawLineBuffer(LineBuffer &buffer, int x, int y, double ang, bool south=false);

    void DrawNumbers( wxPoint p, double value, int settings, wxColour back_color );

    wxString getLabelString(double value, int settings);
    wxImage &getLabel(double value, int settings, wxColour back_colour);


#ifdef ocpnUSE_GL
    void DrawGLTexture( GLuint texture, int width, int height,
                        int xd, int yd, double dwidth, double dheight,
                        PlugIn_ViewPort *vp );
    bool CreateGribGLTexture( GribOverlay *pGO, int config, GribRecord *pGR,
                              PlugIn_ViewPort *vp, int grib_pixel_size );
#endif
    wxImage CreateGribImage( int config, GribRecord *pGR, PlugIn_ViewPort *vp,
                             int grib_pixel_size, const wxPoint &porg );

    double m_last_vp_scale;

    GribOverlay *m_pOverlay[GribOverlaySettings::SETTINGS_COUNT];

    wxString m_Message;
    wxString m_Message_Hiden;
    int  m_TimeZone;

    wxDC *m_pdc;
#if wxUSE_GRAPHICS_CONTEXT
    wxGraphicsContext *m_gdc;
#endif

    wxFont *m_dFont_map;
    wxFont *m_dFont_war;

    bool m_hiDefGraphics;
    bool m_bGradualColors;

    std::map < double , wxImage > m_labelCache;

    TexFont m_TexFontMessage, m_TexFontNumbers;

    GRIBUICtrlBar &m_dlg;
    GribOverlaySettings &m_Settings;

    ParticleMap *m_ParticleMap;
    wxTimer m_tParticleTimer;
    bool m_bUpdateParticles;

    LineBuffer m_WindArrowCache[14];
    LineBuffer m_SingleArrow[2], m_DoubleArrow[2];
};
