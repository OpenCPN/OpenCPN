/**************************************************************************
 *   Copyright (C) 2010 - 2024 by David S. Register                        *
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

/**
 * \file
 * PlugIn Object Definition/API
 */

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#ifndef DECL_EXP
#if defined(__WXMSW__) || defined(__CYGWIN__)
#define DECL_EXP __declspec(dllexport)
#elif defined __GNUC__ && __GNUC__ >= 4
#define DECL_EXP __attribute__((visibility("default")))
#elif defined __WXOSX__
#define DECL_EXP __attribute__((visibility("default")))
#else
#define DECL_EXP
#endif
#endif

#if defined(__WXMSW__) && defined(MAKING_PLUGIN)
#define DECL_IMP __declspec(dllimport)
#else
#define DECL_IMP
#endif

#include <wx/xml/xml.h>
#include <wx/dcmemory.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/menuitem.h>
#include <wx/gdicmn.h>

#ifdef ocpnUSE_SVG
#include <wx/bitmap.h>
#endif  // ocpnUSE_SVG

#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>

class wxGLContext;

//    This is the most modern API Version number
//    It is expected that the API will remain downward compatible, meaning that
//    PlugIns conforming to API Version less than the most modern will also
//    be correctly supported.
#define API_VERSION_MAJOR 1
#define API_VERSION_MINOR 20

//    Fwd Definitions
class wxFileConfig;
class wxNotebook;
class wxFont;
class wxAuiManager;
class wxScrolledWindow;
class wxGLCanvas;

//---------------------------------------------------------------------------------------------------------
//
//    Bitfield PlugIn Capabilities flag definition
//
//---------------------------------------------------------------------------------------------------------
/** Receive callbacks to render custom overlay graphics on the chart.
    Used for drawing additional navigation data, markers, or custom
   visualizations. */
#define WANTS_OVERLAY_CALLBACK 0x00000001
/** Receive updates when cursor moves over chart.
    Enables plugins to show information about chart features at cursor position.
 */
#define WANTS_CURSOR_LATLON 0x00000002
/**
 * Receive notification when user left-clicks plugin's toolbar buttons.
 *
 * Required for plugins that need to respond to their toolbar button actions.
 * \ref opencpn_plugin::OnToolbarToolCallback() will be called with the button
 * ID when user left-clicks a toolbar button.
 *
 * @see opencpn_plugin::OnToolbarToolCallback
 */
#define WANTS_TOOLBAR_CALLBACK 0x00000004
/**
 * Plugin will add one or more toolbar buttons.
 *
 * Enables plugin to extend OpenCPN toolbar with custom functionality.
 */
#define INSTALLS_TOOLBAR_TOOL 0x00000008
/** Plugin requires persistent configuration storage.
    Enables access to the config file for saving and loading settings. */
#define WANTS_CONFIG 0x00000010
/** Plugin will add pages to the toolbox/settings dialog.
    Allows plugin to provide custom configuration UI in OpenCPN settings. */
#define INSTALLS_TOOLBOX_PAGE 0x00000020
/** Plugin will add items to chart context menu.
    Enables extending the right-click menu with custom actions. */
#define INSTALLS_CONTEXTMENU_ITEMS 0x00000040
/** Receive raw NMEA 0183 sentences from all active ports.
    Used for plugins that need to process navigation data directly. */
#define WANTS_NMEA_SENTENCES 0x00000080
/** Receive decoded NMEA events with parsed data.
    Provides easy access to specific navigation data without parsing raw
   sentences. */
#define WANTS_NMEA_EVENTS 0x00000100
/** Receive AIS target information and updates.
    Required for plugins that monitor or process vessel traffic data. */
#define WANTS_AIS_SENTENCES 0x00000200
/** Plugin uses wxAuiManager for window management.
    Needed for plugins that create dockable windows or panels. */
#define USES_AUI_MANAGER 0x00000400
/** Plugin will add page(s) to global preferences dialog.
    Allows plugin to integrate configuration UI with main preferences. */
#define WANTS_PREFERENCES 0x00000800
/** Plugin provides new chart type for standard (non-GL) view.
    Used by plugins that implement custom chart formats. */
#define INSTALLS_PLUGIN_CHART 0x00001000
/** Receive callbacks during chart viewport painting.
    Enables custom drawing in standard (non-GL) chart display. */
#define WANTS_ONPAINT_VIEWPORT 0x00002000
/** Enable message passing between plugins.
    Required for plugins that need to communicate with other plugins. */
#define WANTS_PLUGIN_MESSAGING 0x00004000
#define WANTS_OPENGL_OVERLAY_CALLBACK 0x00008000
#define WANTS_DYNAMIC_OPENGL_OVERLAY_CALLBACK 0x00010000
/** Delay full plugin initialization until system is ready.
    Useful for plugins that need complete system initialization. */
#define WANTS_LATE_INIT 0x00020000
/** Plugin provides new chart type for OpenGL view.
    Used by plugins that implement custom chart formats with OpenGL support. */
#define INSTALLS_PLUGIN_CHART_GL 0x00040000
/** Receive mouse events (clicks, movement, etc).
    Enables plugins to respond to user mouse interaction. */
#define WANTS_MOUSE_EVENTS 0x00080000
/** Receive information about vector chart objects.
    Enables access to S57 chart feature data and attributes. */
#define WANTS_VECTOR_CHART_OBJECT_INFO 0x00100000
/** Receive keyboard events from main window.
    Enables plugins to implement keyboard shortcuts or commands. */
#define WANTS_KEYBOARD_EVENTS 0x00200000
/** Receive notification just before OpenCPN shutdown.
    Allows plugins to clean up resources and save state. */
#define WANTS_PRESHUTDOWN_HOOK 0x00400000

//---------------------------------------------------------------------------------------------------------
//
//    Overlay priorities
//
//---------------------------------------------------------------------------------------------------------
#define OVERLAY_LEGACY 0
#define OVERLAY_OVER_SHIPS 64
#define OVERLAY_OVER_EMBOSS 96
#define OVERLAY_OVER_UI 128

//----------------------------------------------------------------------------------------------------------
//    Some PlugIn API interface object class definitions
//----------------------------------------------------------------------------------------------------------
/**
 * Enumeration of color schemes.
 */
enum PI_ColorScheme {
  PI_GLOBAL_COLOR_SCHEME_RGB,    //!< RGB color scheme, unmodified colors
  PI_GLOBAL_COLOR_SCHEME_DAY,    //!< Day color scheme, optimized for bright
                                 //!< ambient light
  PI_GLOBAL_COLOR_SCHEME_DUSK,   //!< Dusk color scheme, optimized for low
                                 //!< ambient light
  PI_GLOBAL_COLOR_SCHEME_NIGHT,  //!< Night color scheme, optimized for dark
                                 //!< conditions with minimal impact on night
                                 //!< vision
  PI_N_COLOR_SCHEMES  //!< Number of color schemes, used for bounds checking
};

/**
 * Contains view parameters and status information for a chart display viewport.
 */
class PlugIn_ViewPort {
public:
  double clat;  //!< Center latitude of the viewport in decimal degrees
  double clon;  //!< Center longitude of the viewport in decimal degrees
  double view_scale_ppm;  //!< Display scale in pixels per meter
  double skew;            //!< Display skew angle in radians
  double rotation;        //!< Display rotation angle in radians

  float chart_scale;  //!< Conventional chart displayed scale (e.g., 1:50000)

  int pix_width;   //!< Viewport width in pixels
  int pix_height;  //!< Viewport height in pixels
  wxRect rv_rect;  //!< Rectangle defining the rendered view area
  bool b_quilt;    //!< True if the viewport is in quilt mode (showing multiple
                   //!< charts)
  int m_projection_type;  //!< Chart projection type (PROJECTION_MERCATOR, etc.)

  double lat_min;  //!< Minimum latitude of the viewport
  double lat_max;  //!< Maximum latitude of the viewport
  double lon_min;  //!< Minimum longitude of the viewport
  double lon_max;  //!< Maximum longitude of the viewport

  bool
      bValid;  //!< True if this viewport is valid and can be used for rendering
};

/**
 * Basic position fix information.
 */
class PlugIn_Position_Fix {
public:
  double Lat;  //!< Latitude in decimal degrees
  double Lon;  //!< Longitude in decimal degrees
  double Cog;  //!< Course over ground in degrees [0-360).
  double Sog;  //!< Speed over ground in knots
  double Var;  //!< Magnetic variation in degrees, typically from RMC message
  time_t FixTime;  //!< UTC time of fix as time_t value
  int nSats;       //!< Number of satellites used in the fix
};
/**
 * Extended position fix information.
 *
 * This class provides position and navigation data that may come from various
 * sources:
 * - GNSS receiver (primary source)
 * - Last known position (when GNSS signal is lost)
 * - User-defined position (when manually moved on map)
 * - Dead reckoning (calculated from last known position and movement)
 */
class PlugIn_Position_Fix_Ex {
public:
  /**
   * Latitude in decimal degrees.
   * May represent last known position rather than current true position if:
   * - GNSS signal is lost
   * - Position has been manually set by user on map
   */
  double Lat;

  /**
   * Longitude in decimal degrees.
   * May represent last known position rather than current true position if:
   * - GNSS signal is lost
   * - Position has been manually set by user on map
   */
  double Lon;

  /**
   * Course over ground in degrees [0-360).
   */
  double Cog;

  /**
   * Speed over ground in knots.
   * May be NaN if speed cannot be determined.
   */
  double Sog;

  /** Magnetic variation in degrees, typically from RMC message */
  double Var;

  /**
   * Heading magnetic in degrees [0-360).
   * May be NaN if heading sensor data is not available.
   */
  double Hdm;

  /**
   * Heading true in degrees [0-360).
   * May be NaN if true heading cannot be calculated (requires both magnetic
   * heading and variation).
   */
  double Hdt;

  /**
   * UTC time of fix.
   * - If GNSS available: Time from most recent GNSS message
   * - If GNSS watchdog expired: Current system time
   */
  time_t FixTime;

  /**
   * Number of satellites used in the fix.
   * Will be 0 if:
   * - GNSS watchdog has expired
   * - Position is not from GNSS
   */
  int nSats;
};

/**
 * Information about the currently active route leg.
 */
class Plugin_Active_Leg_Info {
public:
  double Xte;        //!< Cross track error in nautical miles, negative values
                     //!< indicate left side of track
  double Btw;        //!< Bearing to waypoint in degrees true
  double Dtw;        //!< Distance to waypoint in nautical miles
  wxString wp_name;  //!< Name of destination waypoint for the active leg
  bool arrival;      //!< True when vessel is within the arrival circle of the
                     //!< destination waypoint
};

/**
 * Enumeration of AIS alarm states.
 */
enum plugin_ais_alarm_type {
  PI_AIS_NO_ALARM = 0,       //!< No alarm condition exists
  PI_AIS_ALARM_SET,          //!< Alarm has been set but not acknowledged
  PI_AIS_ALARM_ACKNOWLEDGED  //!< Alarm has been acknowledged by the user
};

/**
 * AIS target information accessible to plugins.
 */
class PlugIn_AIS_Target {
public:
  int MMSI;                //!< Maritime Mobile Service Identity number
  int Class;               //!< AIS class (Class A: 0, Class B: 1)
  int NavStatus;           //!< Navigational status (0-15 as per ITU-R M.1371)
  double SOG;              //!< Speed over ground in knots
  double COG;              //!< Course over ground in degrees
  double HDG;              //!< Heading in degrees true
  double Lon;              //!< Longitude in decimal degrees
  double Lat;              //!< Latitude in decimal degrees
  int ROTAIS;              //!< Rate of turn as indicated in AIS message
  char CallSign[8];        //!< Call sign, includes NULL terminator
  char ShipName[21];       //!< Ship name, includes NULL terminator
  unsigned char ShipType;  //!< Ship type as per ITU-R M.1371
  int IMO;                 //!< IMO ship identification number

  double Range_NM;  //!< Range to target in nautical miles
  double Brg;       //!< Bearing to target in degrees true

  //      Per target collision parameters
  bool bCPA_Valid;  //!< True if CPA calculation is valid
  double TCPA;      //!< Time to Closest Point of Approach in minutes
  double CPA;       //!< Closest Point of Approach in nautical miles

  plugin_ais_alarm_type alarm_state;  //!< Current alarm state for this target
};

/**
 * Enumeration of chart types.
 */
typedef enum ChartTypeEnumPI {
  PI_CHART_TYPE_UNKNOWN = 0,  //!< Chart type is unknown or undefined
  PI_CHART_TYPE_DUMMY,  //!< Dummy chart used when no actual chart is available
  PI_CHART_TYPE_DONTCARE,  //!< Chart type is not important for the operation
  PI_CHART_TYPE_KAP,       //!< BSB/KAP raster chart format
  PI_CHART_TYPE_GEO,       //!< GEO raster chart format
  PI_CHART_TYPE_S57,       //!< S-57 vector chart format
  PI_CHART_TYPE_CM93,      //!< CM93 vector chart format
  PI_CHART_TYPE_CM93COMP,  //!< CM93 Composite vector chart format
  PI_CHART_TYPE_PLUGIN     //!< Chart type provided by a plugin
} _ChartTypeEnumPI;

/**
 * Enumeration of chart families (broad categories).
 */
typedef enum ChartFamilyEnumPI {
  PI_CHART_FAMILY_UNKNOWN = 0,  //!< Chart family is unknown
  PI_CHART_FAMILY_RASTER,       //!< Raster chart formats (KAP, GEO, etc.)
  PI_CHART_FAMILY_VECTOR,       //!< Vector chart formats (S-57, CM93, etc.)
  PI_CHART_FAMILY_DONTCARE  //!< Chart family is not important for the operation
} _ChartFamilyEnumPI;

/**
 * Enumeration of depth unit types used in charts.
 */
typedef enum ChartDepthUnitTypePI {
  PI_DEPTH_UNIT_UNKNOWN,  //!< Depth unit is unknown or not specified
  PI_DEPTH_UNIT_FEET,     //!< Depths shown in feet
  PI_DEPTH_UNIT_METERS,   //!< Depths shown in meters
  PI_DEPTH_UNIT_FATHOMS   //!< Depths shown in fathoms
} _ChartDepthUnitTypePI;

/**
 * Enumeration of chart projection types.
 */
typedef enum OcpnProjTypePI {
  PI_PROJECTION_UNKNOWN,   //!< Projection type is unknown or undefined
  PI_PROJECTION_MERCATOR,  //!< Mercator projection, standard for navigation
                           //!< charts
  PI_PROJECTION_TRANSVERSE_MERCATOR,  //!< Transverse Mercator projection, used
                                      //!< for some coastal charts
  PI_PROJECTION_POLYCONIC,  //!< Polyconic projection, sometimes used for inland
                            //!< charts

  PI_PROJECTION_ORTHOGRAPHIC,   //!< Orthographic projection, showing Earth as
                                //!< viewed from space
  PI_PROJECTION_POLAR,          //!< Polar projection, used for polar regions
  PI_PROJECTION_STEREOGRAPHIC,  //!< Stereographic projection, used for polar
                                //!< and specialty charts
  PI_PROJECTION_GNOMONIC,       //!< Gnomonic projection, in which great circles
                                //!< appear as straight lines
  PI_PROJECTION_EQUIRECTANGULAR  //!< Equirectangular/Plate Carrée projection,
                                 //!< simple lat/lon grid
} _OcpnProjTypePI;

/**
 * Geographic extent structure defining a bounding box.
 */
typedef struct _ExtentPI {
  double SLAT;  //!< Southern latitude boundary in decimal degrees
  double WLON;  //!< Western longitude boundary in decimal degrees
  double NLAT;  //!< Northern latitude boundary in decimal degrees
  double ELON;  //!< Eastern longitude boundary in decimal degrees
} ExtentPI;

//    PlugInChartBase::Init()  init_flags constants
/** Full chart initialization including all data needed for rendering and all
 * operations. */
#define PI_FULL_INIT 0
/** Initialize only the chart header/metadata for chart database operations
 * (faster, used for chart database building). */
#define PI_HEADER_ONLY 1
/** Initialize only what's needed to generate a thumbnail image. */
#define PI_THUMB_ONLY 2

/**
 * Base class for implementing custom chart types in OpenCPN plugins.
 *
 * The PlugInChartBase class provides the interface between custom chart types
 * implemented by plugins and OpenCPN's chart display and management system.
 * Plugin developers can create derived classes to support custom chart formats,
 * enabling the display of proprietary or specialized charts within OpenCPN.
 *
 * When implementing a chart plugin, you need to:
 * 1. Create a class derived from PlugInChartBase
 * 2. Implement all required virtual methods
 * 3. Set the INSTALLS_PLUGIN_CHART capability flag in your plugin
 */
class DECL_EXP PlugInChartBase : public wxObject {
public:
  //    These methods Must be overriden in any derived class
  PlugInChartBase();
  virtual ~PlugInChartBase();

  /**
   * Returns file pattern(s) for chart files this plugin can handle.
   *
   * This method is called by OpenCPN during chart directory scanning to
   * identify which files should be examined by this chart type. The returned
   * pattern is used with wxDir::GetAllFiles() to locate potential chart files.
   *
   * @return wxString containing file pattern wildcards (e.g., "*.os63" or
   * "*.abc;*.xyz")
   *
   * @note For plugins supporting multiple file extensions, separate them with
   * semicolons without spaces between patterns (e.g., "*.000;*.S57").
   * @note On non-Windows platforms, OpenCPN automatically checks for both
   * uppercase and lowercase variants of the pattern.
   * @note The search performance directly impacts chart loading times, so
   * specific file extensions are preferred over complex patterns.
   * @note For plugins that don't use file-based charts, return an empty string.
   *
   * @see ChartDatabase::SearchDirAndAddCharts() for usage details
   */
  virtual wxString GetFileSearchMask(void);

  /**
   * Initializes a chart instance from a file.
   *
   * This method is called when OpenCPN needs to load a chart that matches the
   * file search mask. It should parse the chart file's headers or content based
   * on the initialization level requested and populate the chart object's
   * properties.
   *
   * @param full_path Full path to the chart file to be loaded
   * @param init_flags Level of initialization to perform:
   *        - PI_FULL_INIT (0): Load complete chart for display
   *        - PI_HEADER_ONLY (1): Load only chart metadata (for database
   * building)
   *        - PI_THUMB_ONLY (2): Load minimal data needed for thumbnail
   * generation
   *
   * @return An integer status:
   *         - INIT_OK (0) if initialization was successful
   *         - INIT_FAIL_RETRY if initialization failed but should be retried
   *         - INIT_FAIL_REMOVE if initialization failed and the chart should be
   * removed
   *         - INIT_FAIL_NOERROR if initialization failed but no error message
   * is needed
   */
  virtual int Init(const wxString &full_path, int init_flags);

  /**
   * Sets the color scheme for chart display.
   *
   * This method is called when the user changes OpenCPN's color scheme
   * (day/dusk/night). Chart plugins should update their rendering colors and
   * styles to match the specified scheme.
   *
   * @param cs Color scheme to use:
   *        - PI_GLOBAL_COLOR_SCHEME_RGB (0): RGB color scheme
   *        - PI_GLOBAL_COLOR_SCHEME_DAY (1): Day color scheme
   *        - PI_GLOBAL_COLOR_SCHEME_DUSK (2): Dusk/twilight color scheme
   *        - PI_GLOBAL_COLOR_SCHEME_NIGHT (3): Night/dark color scheme
   * @param bApplyImmediate True to immediately refresh display, False to defer
   *
   * @note If bApplyImmediate is true, any cached rendering should be
   * invalidated
   * @note For optimal night viewing, reduce brightness of whites and increase
   * contrast
   */
  virtual void SetColorScheme(int cs, bool bApplyImmediate);

  /**
   * Returns the minimum recommended scale for this chart.
   *
   * This method defines how far a user can zoom in before the chart is
   * considered "over-zoomed". The value is expressed as a fraction of the
   * chart's native scale. OpenCPN uses this to determine when to display
   * over-zoom indicators and to manage chart selection in quilt mode.
   *
   * @param canvas_scale_factor Current display scale factor
   * @param b_allow_overzoom Whether the user has enabled chart over-zoom in
   * settings
   * @return Minimum recommended scale value (usually a fraction of native
   * scale)
   *
   * @note Smaller values allow deeper zoom (e.g., 0.125 allows zooming to 8x
   * native scale)
   * @note Consider visual quality, data density, and accuracy when choosing
   * this value
   * @note Many charts use 1/4 or 1/8 of native scale as the minimum
   */
  virtual double GetNormalScaleMin(double canvas_scale_factor,
                                   bool b_allow_overzoom);
  /**
   * Returns the maximum recommended scale for this chart.
   *
   * This method defines how far a user can zoom out before the chart is
   * considered "under-zoomed" or too small to be useful. The value is expressed
   * as a multiple of the chart's native scale. OpenCPN uses this for chart
   * selection in quilt mode.
   *
   * @param canvas_scale_factor Current display scale factor
   * @param canvas_width Width of the display canvas in pixels
   * @return Maximum recommended scale value (usually a multiple of native
   * scale)
   *
   * @note Larger values allow viewing the chart from further out
   * @note Many charts use 2x to 4x native scale as the maximum
   * @note Consider legibility of text and symbols when choosing this value
   */
  virtual double GetNormalScaleMax(double canvas_scale_factor,
                                   int canvas_width);
  /**
   * Returns the nearest preferred scale value for this chart.
   *
   * This method allows charts to define preferred or standard scale points that
   * OpenCPN should snap to when zooming. If your chart type doesn't have
   * specific preferred scales, simply return the input value.
   *
   * @param target_scale_ppm Target scale in pixels per meter
   * @return Nearest preferred scale value in pixels per meter
   *
   * @note For many charts, returning target_scale_ppm is appropriate
   * @note Some chart types (like S-57) have standard display scales defined
   * @note PPM (pixels per meter) = 2540 / (chart_scale * inches_per_meter)
   */
  virtual double GetNearestPreferredScalePPM(double target_scale_ppm);

  /**
   * Gets the geographic boundaries of the chart.
   *
   * This method provides the latitude and longitude limits of the chart,
   * which OpenCPN uses to determine chart coverage, selection, and quilting.
   * The extent should encompass all valid data in the chart.
   *
   * @param pext Pointer to an ExtentPI structure to be filled with boundary
   * values:
   *        - NLAT: Northern latitude bound (max latitude)
   *        - SLAT: Southern latitude bound (min latitude)
   *        - ELON: Eastern longitude bound (max longitude)
   *        - WLON: Western longitude bound (min longitude)
   * @return True if successful, False if the extent cannot be determined
   *
   * @note All longitudes should be normalized to -180 to +180 range
   */
  virtual bool GetChartExtent(ExtentPI *pext);

  /**
   * Renders a region of the chart for display.
   *
   * This is the primary rendering method called when the chart needs to be
   * drawn. It should create and return a bitmap containing the rendered chart
   * image for the specified view parameters and region.
   *
   * @param VPoint Current viewport parameters:
   *        - clat/clon: Center latitude/longitude
   *        - view_scale_ppm: Current scale in pixels per meter
   *        - skew: Chart rotation skew
   *        - rotation: Display rotation
   *        - pix_width/pix_height: Viewport size in pixels
   *        - rv_rect: Rectangle region in screen coordinates
   *        - b_quilt: Whether chart quilting is active
   * @param Region wxRegion defining the area to be rendered (in screen
   * coordinates)
   * @return wxBitmap containing the rendered chart image
   *
   * @note The implementation should be optimized as this is called frequently
   * during navigation
   * @note For large charts, consider caching rendered results for improved
   * performance
   * @note The bitmap should match the viewport dimensions
   * (VPoint.pix_width/height)
   */
  virtual wxBitmap &RenderRegionView(const PlugIn_ViewPort &VPoint,
                                     const wxRegion &Region);

  /**
   * Adjusts viewport parameters for chart-specific requirements.
   *
   * This method allows charts to modify proposed viewport changes before they
   * are applied. It can be used to enforce chart-specific constraints or
   * optimize display for particular chart types.
   *
   * @param vp_last Previous viewport parameters
   * @param vp_proposed Proposed new viewport parameters (may be modified)
   * @return True if the viewport was modified, False if no changes were made
   *
   * @note Commonly used to handle special projections or boundary constraints
   * @note This is called during user navigation (pan/zoom) operations
   */
  virtual bool AdjustVP(PlugIn_ViewPort &vp_last, PlugIn_ViewPort &vp_proposed);

  /**
   * Determines the valid display area for this chart.
   *
   * This method defines the region of the viewport where the chart has valid
   * data. OpenCPN uses this information for chart quilting to determine where
   * this chart should be displayed versus other charts.
   *
   * @param VPoint Current viewport parameters
   * @param pValidRegion Pointer to wxRegion to be filled with the valid area
   *
   * @note For charts with irregular boundaries, this should reflect the actual
   * coverage
   * @note For rectangular charts, this is typically the entire viewport
   * @note The region should be specified in screen pixel coordinates
   */
  virtual void GetValidCanvasRegion(const PlugIn_ViewPort &VPoint,
                                    wxRegion *pValidRegion);

  /**
   * Returns the number of coverage table entries for this chart.
   *
   * This method reports how many distinct coverage regions the chart contains.
   * Charts may have multiple valid regions within the lat/lon box described
   * by the chart extent. For example:
   * - Raster charts (BSB/KAP) typically have only one coverage area
   * - Vector charts (ENC/S-57) often have multiple coverage areas
   *
   * @return Number of coverage table entries (0 if chart has no coverage data)
   *
   * @note For simple charts with rectangular coverage, this typically returns 1
   * @note For more complex charts with irregular boundaries, this may return
   * multiple entries
   * @note Charts with no explicit coverage information should return 0
   * @see GetCOVRTablePoints, GetCOVRTableHead
   */
  virtual int GetCOVREntries() { return 0; }

  /**
   * Returns the number of points in a specific coverage table entry.
   *
   * This method reports how many coordinate pairs (lat/lon points) define
   * the boundary polygon of a particular coverage area.
   *
   * @param iTable Index of the coverage table entry (0 to GetCOVREntries()-1)
   * @return Number of points in the specified coverage table entry
   *
   * @note Each point in the coverage table consists of a lat/lon pair (2
   * floats)
   * @note The points define a closed polygon boundary
   * @note Returns 0 if the index is out of range or no coverage data exists
   * @see GetCOVREntries, GetCOVRTableHead
   */
  virtual int GetCOVRTablePoints(int iTable) { return 0; }

  /**
   * Alternative method to get the number of points in a coverage table entry.
   *
   * This method is functionally identical to GetCOVRTablePoints() and exists
   * for historical compatibility reasons.
   *
   * @param iTable Index of the coverage table entry (0 to GetCOVREntries()-1)
   * @return Number of points in the specified coverage table entry
   *
   * @note Implementations should typically delegate to GetCOVRTablePoints()
   * @note This method exists for backward compatibility
   * @see GetCOVRTablePoints
   */
  virtual int GetCOVRTablenPoints(int iTable) { return 0; }

  /**
   * Returns a pointer to the coverage table data for a specific entry.
   *
   * This method provides access to the actual coordinate data defining the
   * boundary polygon of a coverage area. The data is stored as an array of
   * float values, organized as lat/lon pairs.
   *
   * @param iTable Index of the coverage table entry (0 to GetCOVREntries()-1)
   * @return Pointer to an array of float values containing lat/lon points:
   *         [lat0, lon0, lat1, lon1, ..., latN, lonN]
   *         Returns NULL if the index is invalid or no coverage data exists
   *
   * @note The data is arranged as consecutive (latitude, longitude) pairs
   * @note The total number of floats in the array is 2 *
   * GetCOVRTablePoints(iTable)
   * @note The coordinate system uses decimal degrees
   * @note The boundary polygon is implicitly closed (last point connects to
   * first)
   * @see GetCOVREntries, GetCOVRTablePoints
   */
  virtual float *GetCOVRTableHead(int iTable) { return (float *)NULL; }

  /**
   * Generates a thumbnail image of the chart.
   *
   * This method creates a small preview image for the chart manager and chart
   * selection interfaces. It should produce a representative, simplified view
   * of the chart that helps users identify it.
   *
   * @param tnx Desired thumbnail width in pixels
   * @param tny Desired thumbnail height in pixels
   * @param cs Color scheme to use for rendering:
   *           - PI_GLOBAL_COLOR_SCHEME_RGB (0): RGB color scheme
   *           - PI_GLOBAL_COLOR_SCHEME_DAY (1): Day color scheme
   *           - PI_GLOBAL_COLOR_SCHEME_DUSK (2): Dusk/twilight color scheme
   *           - PI_GLOBAL_COLOR_SCHEME_NIGHT (3): Night/dark color scheme
   * @return Pointer to a newly created bitmap containing the thumbnail
   *        (caller is responsible for deleting it)
   *
   * @note The thumbnail should be a reduced-detail representation of the chart
   * @note For best results, maintain the aspect ratio of the chart's geographic
   * extent
   * @note If unable to generate a thumbnail, return NULL
   */
  virtual wxBitmap *GetThumbnail(int tnx, int tny, int cs);

  //    Accessors, need not be overridden in derived class if the member
  //    variables are maintained

  /**
   * Returns the full file path of the chart.
   *
   * This method provides access to the complete file path of the chart as it
   * was loaded. OpenCPN uses this for chart identification, database
   * management, and subsequent loading operations.
   *
   * @return wxString containing the full path to the chart file
   *
   * @note This should return the same path passed to Init()
   * @note For non-file-based charts, return a unique identifier string
   * @note The path should use platform-appropriate path separators
   */
  virtual wxString GetFullPath() const { return m_FullPath; }

  /**
   * Returns the chart type identifier.
   *
   * This method identifies the general category of the chart, such as raster,
   * vector, or other specialized types. OpenCPN uses this for chart management,
   * display settings, and feature availability.
   *
   * @return ChartTypeEnumPI value indicating the chart type:
   *         - CHART_TYPE_UNKNOWN: Chart type not determined
   *         - CHART_TYPE_DUMMY: Placeholder chart
   *         - CHART_TYPE_DONTCARE: Chart type not relevant
   *         - CHART_TYPE_KAP: BSB/KAP raster chart
   *         - CHART_TYPE_GEO: GEO raster chart
   *         - CHART_TYPE_S57: S-57 vector chart
   *         - CHART_TYPE_CM93: CM93 vector chart
   *         - CHART_TYPE_CM93COMP: CM93 composite vector chart
   *         - CHART_TYPE_PLUGIN: Plugin-defined chart type
   *
   * @note Plugin charts typically return CHART_TYPE_PLUGIN
   * @note This value affects how OpenCPN treats the chart in various operations
   */
  virtual ChartTypeEnumPI GetChartType() { return m_ChartType; }

  /**
   * Returns the chart family classification.
   *
   * This method identifies the broader family category of the chart, grouping
   * similar chart types. OpenCPN uses this for feature availability and
   * rendering decisions.
   *
   * @return ChartFamilyEnumPI value indicating the chart family:
   *         - CHART_FAMILY_UNKNOWN: Chart family not determined
   *         - CHART_FAMILY_RASTER: Raster chart type (e.g., KAP, GEO)
   *         - CHART_FAMILY_VECTOR: Vector chart type (e.g., S-57, CM93)
   *         - CHART_FAMILY_DONTCARE: Chart family not relevant
   */
  virtual ChartFamilyEnumPI GetChartFamily() { return m_ChartFamily; }

  /**
   * Returns the projection type used by the chart.
   *
   * This method identifies the cartographic projection used in the chart.
   * Different projections have different characteristics for representing
   * the curved Earth on a flat surface.
   *
   * @return OcpnProjTypePI value indicating the projection:
   *         - PI_PROJECTION_UNKNOWN: Projection not determined
   *         - PI_PROJECTION_MERCATOR: Mercator projection
   *         - PI_PROJECTION_TRANSVERSE_MERCATOR: Transverse Mercator projection
   *         - PI_PROJECTION_POLYCONIC: Polyconic projection
   *         - PI_PROJECTION_ORTHOGRAPHIC: Orthographic projection
   *         - PI_PROJECTION_POLAR: Polar projection
   *         - PI_PROJECTION_STEREOGRAPHIC: Stereographic projection
   *         - PI_PROJECTION_GNOMONIC: Gnomonic projection
   *         - PI_PROJECTION_EQUIRECTANGULAR: Equirectangular/Plate Carrée
   * projection
   */
  virtual OcpnProjTypePI GetChartProjection() { return m_projection; }

  /**
   * Returns the chart's name or title.
   *
   * This method provides the human-readable name of the chart as displayed
   * in chart information panels, selection dialogs, and tooltips.
   *
   * @return wxString containing the chart name/title
   *
   * @note This should be a concise, descriptive name
   * @note For standardized charts, use the official chart name
   * @note The name should not normally include the file extension
   */
  virtual wxString GetName() { return m_Name; }

  /**
   * Returns a descriptive text about the chart.
   *
   * This method provides additional descriptive information about the chart,
   * which OpenCPN displays in chart information panels and tooltips.
   *
   * @return wxString containing the chart description
   *
   * @note Can include information like area coverage, source, etc.
   */
  virtual wxString GetDescription() { return m_Description; }

  /**
   * Returns a unique identifier for the chart.
   *
   * This method provides a unique string identifier for the chart that
   * can be used for database references and identification.
   *
   * @return wxString containing the chart ID
   *
   * @note This should be a unique identifier across all charts
   * @note For standard chart types, use the official chart number/ID
   */
  virtual wxString GetID() { return m_ID; }

  /**
   * Returns the chart's source edition identifier.
   *
   * This method provides information about the source edition of the chart,
   * which can be used to identify its origin and version.
   *
   * @return wxString containing the source edition identifier
   *
   * @note For official charts, this should contain the edition or update
   * information
   * @note Format varies depending on chart type and source
   * @note For derived charts, this can indicate the source data
   */
  virtual wxString GetSE() { return m_SE; }

  /**
   * Returns the depth units used in the chart.
   *
   * This method identifies the units used for depth measurements in the chart,
   * which is important for display and conversion purposes.
   *
   * @return wxString containing the depth units, typically:
   *         - "Meters" for metric charts
   *         - "Fathoms" for imperial/traditional charts
   *         - "Feet" for some inland or special purpose charts
   *
   * @note This affects how depth values are interpreted and displayed
   * @note OpenCPN can convert between units as needed
   * @note Should be properly capitalized for display purposes
   */
  virtual wxString GetDepthUnits() { return m_DepthUnits; }

  /**
   * Returns the vertical datum used for soundings in the chart.
   *
   * This method identifies the reference level used for depth measurements
   * in the chart, which is important for navigational safety.
   *
   * @return wxString containing the sounding datum identifier, such as:
   *         - "MLLW" (Mean Lower Low Water)
   *         - "MLW" (Mean Low Water)
   *         - "LAT" (Lowest Astronomical Tide)
   *         - "MSL" (Mean Sea Level)
   *
   * @note This affects how depth values relate to actual water depth
   * @note Different regions use different standard datums
   * @note Critical for tide calculations and safe navigation
   */
  virtual wxString GetSoundingsDatum() { return m_SoundingsDatum; }

  /**
   * Returns the horizontal geodetic datum of the chart.
   *
   * This method identifies the reference coordinate system used for horizontal
   * positioning in the chart, which affects coordinate transformations.
   *
   * @return wxString containing the datum identifier, such as:
   *         - "WGS84" (World Geodetic System 1984)
   *         - "NAD83" (North American Datum 1983)
   *         - "OSGB36" (Ordnance Survey Great Britain 1936)
   *
   * @note This affects coordinate conversions and positioning accuracy
   * @note Most modern charts use WGS84
   * @note OpenCPN applies datum shifts if necessary
   */
  virtual wxString GetDatumString() { return m_datum_str; }

  /**
   * Returns additional information about the chart.
   *
   * This method provides any extra information about the chart that doesn't
   * fit into the other specific accessor methods. OpenCPN displays this
   * in chart information panels.
   *
   * @return wxString containing extra chart information
   *
   * @note Can include any supplementary details about the chart
   * @note Often includes copyright, disclaimer, or usage information
   * @note Can include information about updates or modifications
   */
  virtual wxString GetExtraInfo() { return m_ExtraInfo; }

  /**
   * Returns the publication date/year of the chart.
   *
   * This method provides the original publication date of the chart,
   * which can be different from the edition date or file modification date.
   *
   * @return wxString containing the publication date/year
   *
   * @note Format varies depending on chart type and source
   * @note Often just contains the year of publication
   * @note Used for chart information display, not for update checking
   */
  virtual wxString GetPubDate() { return m_PubYear; }

  /**
   * Returns the error factor for the chart.
   *
   * This method provides a scaling factor that represents the estimated
   * positional error in the chart. It can be used to calculate accuracy
   * at different scales.
   *
   * @return Double value representing the chart error factor
   *
   * @note Higher values indicate greater potential position error
   * @note Used for display of position accuracy indicators
   * @note Typically based on source data quality and survey methods
   */
  virtual double GetChartErrorFactor() { return m_Chart_Error_Factor; }

  /**
   * Returns the depth unit type identifier.
   *
   * This method provides an enumerated identifier for the depth units used
   * in the chart, which can be used for programmatic unit conversions.
   *
   * @return ChartDepthUnitTypePI enumeration value:
   *         - PI_DEPTH_UNIT_UNKNOWN: Unknown depth units
   *         - PI_DEPTH_UNIT_FEET: Depths in feet
   *         - PI_DEPTH_UNIT_METERS: Depths in meters
   *         - PI_DEPTH_UNIT_FATHOMS: Depths in fathoms
   *
   * @note This complements GetDepthUnits() by providing an enumerated value
   * @note Used for internal unit conversion and display settings
   */
  virtual ChartDepthUnitTypePI GetDepthUnitId() { return m_depth_unit_id; }

  /**
   * Indicates whether the chart is ready for rendering.
   *
   * This method tells OpenCPN whether the chart has loaded sufficient data
   * to be rendered. It can be used to defer rendering until asynchronous
   * loading operations are complete.
   *
   * @return Boolean value: True if the chart is ready to render, False
   * otherwise
   *
   * @note Should return True once all required rendering data is available
   * @note Can be used to implement background loading for large charts
   * @note OpenCPN may delay display until this returns True
   */
  virtual bool IsReadyToRender() { return m_bReadyToRender; }

  /**
   * Returns the native scale of the chart.
   *
   * This method provides the natural or intended display scale of the chart,
   * typically expressed as the scale denominator (e.g., 50000 for 1:50,000).
   *
   * @return Integer value representing the chart's native scale
   *
   * @note This is the scale at which the chart was designed to be viewed
   * @note Used for scale-based chart selection and display indicators
   * @note Lower values (larger fractions) represent more detailed charts
   */
  virtual int GetNativeScale() { return m_Chart_Scale; };

  /**
   * Returns the skew/rotation angle of the chart.
   *
   * This method provides the rotation angle of the chart with respect to true
   * north. It's used for proper alignment of the chart in the display.
   *
   * @return Double value representing the chart skew in degrees
   *
   * @note Positive values indicate clockwise rotation
   * @note Zero indicates the chart is aligned with true north
   * @note Affects all coordinate transformations and display orientation
   */
  virtual double GetChartSkew() { return m_Chart_Skew; }

  /**
   * Returns the edition date of the chart.
   *
   * This method provides the date of the current edition or update of the
   * chart, which can be used to identify the most recent version.
   *
   * @return wxDateTime object containing the edition date
   *
   * @note Different from publication date and file modification date
   * @note Used for chart update management and information display
   * @note Should reflect the date of the latest corrections included
   */
  virtual wxDateTime GetEditionDate(void) { return m_EdDate; }

  //    Methods pertaining to CHART_FAMILY_RASTER type PlugIn charts only

  /**
   * Computes the source rectangle for the chart based on a given viewport.
   *
   * This method calculates the rectangle within the chart's source image that
   * corresponds to the visible area in the given viewport. It is used primarily
   * by raster chart plugins to determine which portion of the chart needs to be
   * rendered for the current view.
   *
   * @param vp The viewport parameters defining the current view
   * @param pSourceRect Pointer to a wxRect that will be filled with the
   * calculated source rectangle
   *
   * @note This method is essential for efficient rendering of raster charts,
   * allowing the plugin to extract only the needed portion of potentially very
   * large charts.
   * @note Coordinates in the resulting rectangle are in the chart's native
   * pixel space.
   */
  virtual void ComputeSourceRectangle(const PlugIn_ViewPort &vp,
                                      wxRect *pSourceRect);

  /**
   * Returns the scale factor for raster chart rendering.
   *
   * This method provides the scale factor needed to convert between the chart's
   * native pixel resolution and the display resolution. It is used by OpenCPN
   * to properly scale raster chart images during rendering.
   *
   * @return Scale factor as a double value:
   *         - Values > 1.0 indicate chart needs to be scaled up
   *         - Values < 1.0 indicate chart needs to be scaled down
   *         - Value of 1.0 indicates no scaling needed
   *
   * @note Used primarily by raster chart (CHART_FAMILY_RASTER) plugins
   */
  virtual double GetRasterScaleFactor();

  /**
   * Gets pixel data for a portion of a raster chart.
   *
   * This method retrieves bitmap data from the source chart image for a
   * specified rectangular region. The data can be subsampled to support
   * efficient chart display at different zoom levels.
   *
   * @param source Rectangle defining the region to extract in chart native
   * coordinates
   * @param pPix Pointer to a pre-allocated buffer to receive the pixel data
   * @param sub_samp Subsampling factor:
   *        - 1 = every pixel
   *        - 2 = every second pixel
   *        - 4 = every fourth pixel, etc
   * @return True if data was successfully retrieved, False if error
   *
   * @note The pPix buffer must be large enough to hold the subsampled data
   * @note Used primarily by raster chart (CHART_FAMILY_RASTER) plugins
   * @note Buffer format depends on the chart's native pixel format
   */
  virtual bool GetChartBits(wxRect &source, unsigned char *pPix, int sub_samp);

  /**
   * Gets the width of the chart in pixels.
   *
   * This method returns the full width of the source chart image in pixels.
   * This is used for coordinate transforms and display calculations.
   *
   * @return Width of chart in pixels
   *
   * @note For raster charts, this is the actual image pixel width
   * @note For vector charts, this may be a nominal value
   */
  virtual int GetSize_X();

  /**
   * Gets the height of the chart in pixels.
   *
   * This method returns the full height of the source chart image in pixels.
   * This is used for coordinate transforms and display calculations.
   *
   * @return Height of chart in pixels
   *
   * @note For raster charts, this is the actual image pixel height
   * @note For vector charts, this may be a nominal value
   */
  virtual int GetSize_Y();

  /**
   * Converts geographic coordinates to chart pixel coordinates.
   *
   * This method transforms a latitude/longitude position into the chart's
   * native pixel coordinate system.
   *
   * @param lat Latitude in decimal degrees
   * @param lon Longitude in decimal degrees
   * @param pixx Will receive X pixel coordinate
   * @param pixy Will receive Y pixel coordinate
   *
   * @note For raster charts, the origin (0,0) is typically top-left
   * @note Transforms depend on chart projection and calibration
   */
  virtual void latlong_to_chartpix(double lat, double lon, double &pixx,
                                   double &pixy);

  /**
   * Converts chart pixel coordinates to geographic coordinates.
   *
   * This method transforms a position in the chart's native pixel coordinate
   * system into latitude and longitude.
   *
   * @param pixx X pixel coordinate
   * @param pixy Y pixel coordinate
   * @param plat Will receive latitude in decimal degrees
   * @param plon Will receive longitude in decimal degrees
   *
   * @note For raster charts, origin (0,0) is typically top-left
   * @note Transforms depend on chart projection and calibration
   */
  virtual void chartpix_to_latlong(double pixx, double pixy, double *plat,
                                   double *plon);

protected:
  ChartTypeEnumPI m_ChartType;
  ChartFamilyEnumPI m_ChartFamily;

  wxString m_FullPath;
  OcpnProjTypePI m_projection;
  int m_Chart_Scale;
  double m_Chart_Skew;

  wxDateTime m_EdDate;
  bool m_bReadyToRender;

  wxString m_Name;
  wxString m_Description;
  wxString m_ID;
  wxString m_SE;
  wxString m_SoundingsDatum;
  wxString m_datum_str;
  wxString m_PubYear;
  wxString m_DepthUnits;
  wxString m_ExtraInfo;

  ChartDepthUnitTypePI m_depth_unit_id;

  double m_Chart_Error_Factor;
};

//    Declare an array of PlugIn_AIS_Targets
WX_DEFINE_ARRAY_PTR(PlugIn_AIS_Target *, ArrayOfPlugIn_AIS_Targets);

/**
 * Base class for OpenCPN plugins.
 *
 * All plugins must inherit from this class and implement the required virtual
 * methods. The class provides the interface between plugins and the OpenCPN
 * core application.
 *
 * Two types of methods exist:
 * - Required methods that must be implemented by all plugins
 * - Optional methods that may be implemented based on plugin capabilities
 *
 * PlugIns must implement optional method overrides consistent with their
 * declared capabilities flag as returned by Init().
 */
class DECL_EXP opencpn_plugin {
public:
  opencpn_plugin(void *pmgr) {}
  virtual ~opencpn_plugin();

  //    Public API to the PlugIn class

  //    This group of methods is required, and will be called by the opencpn
  //    host opencpn PlugIns must implement this group

  /**
   * Initialize the plugin and declare its capabilities.
   *
   * This required method is called by OpenCPN during plugin loading. It should:
   * - Set up any required plugin resources
   * - Register capabilities by returning a bitwise OR of WANTS_ and INSTALLS_
   * flags
   *
   * @return Integer capability flags indicating plugin features:
   *   - WANTS_OVERLAY_CALLBACK
   *   - WANTS_CURSOR_LATLON
   *   - WANTS_TOOLBAR_CALLBACK
   *   - etc. (see enum definitions)
   */
  virtual int Init(void);

  /**
   * Clean up plugin resources.
   *
   * Called by OpenCPN when plugin is disabled.
   * Shall release any resources allocated by the plugin.
   * Good place to persist plugin configuration.
   *
   * @return True if cleanup successful, False if error
   */
  virtual bool DeInit(void);

  /**
   * Returns the major version number of the plugin API that this plugin
   * supports.
   *
   * This method must be implemented by all plugins to declare compatibility
   * with a specific major version of the OpenCPN plugin API.
   *
   * @return Integer value representing the major API version (e.g., 1)
   *
   * @note Major version changes indicate incompatible API changes
   * @note Current API major version is defined by API_VERSION_MAJOR
   */
  virtual int GetAPIVersionMajor();

  /**
   * Returns the minor version number of the plugin API that this plugin
   * supports.
   *
   * This method must be implemented by all plugins to declare compatibility
   * with a specific minor version of the OpenCPN plugin API.
   *
   * @return Integer value representing the minor API version (e.g., 19)
   *
   * @note Minor version changes indicate backward-compatible API additions
   * @note Current API minor version is defined by API_VERSION_MINOR
   */
  virtual int GetAPIVersionMinor();

  /**
   * Returns the major version number of the plugin itself.
   *
   * This method should return the plugin's own major version number, which is
   * used by OpenCPN for plugin version management and display.
   *
   * @return Integer value representing the plugin's major version
   *
   * @note Part of semantic version (major.minor.patch)
   * @note Major version changes indicate incompatible API changes
   */
  virtual int GetPlugInVersionMajor();

  /**
   * Returns the minor version number of the plugin itself.
   *
   * This method should return the plugin's own minor version number, which is
   * used by OpenCPN for plugin version management and display.
   *
   * @return Integer value representing the plugin's minor version
   *
   * @note Part of semantic version (major.minor.patch)
   * @note Minor version changes indicate backward-compatible feature additions
   */
  virtual int GetPlugInVersionMinor();

  /**
   * Get the plugin's icon bitmap.
   *
   * This method should return a wxBitmap containing the plugin's icon for
   * display in the plugin manager and other UI elements.
   *
   * @return Pointer to wxBitmap containing icon, NULL for default icon
   *
   * @note The bitmap should be square, typically 32x32 pixels
   * @note Ownership of the bitmap remains with the plugin
   * @note Icon should be visible on both light and dark backgrounds
   */
  virtual wxBitmap *GetPlugInBitmap();

  //    These three methods should produce valid, meaningful strings always
  //    ---EVEN IF--- the PlugIn has not (yet) been initialized.
  //    They are used by the PlugInManager GUI

  /**
   * Get the plugin's common (short) name.
   *
   * This required method should return a short name used to identify the plugin
   * in lists and menus.
   *
   * @return wxString containing plugin name (e.g., "Dashboard")
   *
   * @note Should be valid and meaningful even before Init() is called
   * @note Used by the plugin manager GUI
   */
  virtual wxString GetCommonName();

  /**
   * Get a brief description of the plugin.
   *
   * This required method should return a short description (1-2 sentences)
   * explaining the plugin's basic functionality.
   *
   * @return wxString containing brief description
   *
   * @note Should be valid before Init() is called
   * @note Used in plugin manager list view
   */
  virtual wxString GetShortDescription();

  /**
   * Get detailed plugin information.
   *
   * This required method should return a longer description including:
   * - Detailed feature list
   * - Version information
   * - Author/copyright details
   *
   * @return wxString containing detailed plugin information
   *
   * @note Should be valid before Init() is called
   * @note Used in plugin manager details view
   */
  virtual wxString GetLongDescription();

  //    This group is optional.
  //    PlugIns may override any of these methods as required

  /**
   * Sets plugin default options.
   *
   * This method is called when a plugin is enabled via the user preferences
   * dialog. It provides an opportunity to set up any default options and
   * behaviors.
   *
   * @note Optional override - implement if plugin has configurable options
   * @note Called before plugin is activated in GUI
   * @note Good place to load saved settings from config file
   */
  virtual void SetDefaults(void);

  /**
   * Returns the number of toolbar tools this plugin provides.
   *
   * This method should return how many toolbar buttons/tools the plugin will
   * add. Must be implemented if plugin sets INSTALLS_TOOLBAR_TOOL capability
   * flag.
   *
   * @return Number of toolbar items to be added
   *
   * @note Return 0 if no toolbar items
   * @note Each tool needs unique ID for callbacks
   * @note Called during plugin initialization
   */
  virtual int GetToolbarToolCount(void);

  /**
   * Returns the number of preference pages this plugin provides.
   *
   * This method should return how many pages the plugin will add to the toolbox
   * (preferences) dialog. Must be implemented if plugin sets
   * INSTALLS_TOOLBOX_PAGE flag.
   *
   * @return Number of preference pages to add
   *
   * @note Return 0 if no preference pages
   * @note Called during preferences dialog creation
   */
  virtual int GetToolboxPanelCount(void);

  /**
   * Creates a plugin preferences page.
   *
   * This method is called to create each preferences page the plugin provides.
   * Must be implemented if plugin sets INSTALLS_TOOLBOX_PAGE flag.
   *
   * @param page_sel Index of page to create (0 to GetToolboxPanelCount()-1)
   * @param pnotebook Parent notebook to add page to
   *
   * @Deprecated  Does not invoke any action, will be removed.
   * @note Called once for each page index
   * @note Use standard wxWidgets controls for consistent look
   * @note Parent notebook uses wxAUI manager
   */
  virtual void SetupToolboxPanel(int page_sel, wxNotebook *pnotebook);

  /**
   * Handles preference page closure.
   *
   * This method is called when plugin preference pages are closed. Use it to
   * save settings changes or cleanup resources.
   *
   * @param page_sel Index of page being closed
   * @param ok_apply_cancel Result:
   *        - 0 = OK (apply changes)
   *        - 1 = APPLY (apply but don't close)
   *        - 2 = CANCEL (discard changes)
   * @Deprecated  Does not invoke any action, will be removed.
   * @note Only called if plugin implements SetupToolboxPanel()
   * @note Good place to save settings to config
   */
  virtual void OnCloseToolboxPanel(int page_sel, int ok_apply_cancel);

  /**
   * Shows the plugin preferences dialog.
   *
   * This method is called when the user requests the plugin preferences through
   * the OpenCPN options dialog. Must be implemented if plugin sets
   * WANTS_PREFERENCES flag.
   *
   * @param parent Parent window to use for dialog
   *
   * @note Different from toolbox panel preferences
   * @note Use for simple plugin-specific settings
   * @note Modal dialog recommended
   */
  virtual void ShowPreferencesDialog(wxWindow *parent);

  /**
   * Render plugin overlay graphics using standard device context.
   *
   * Called by OpenCPN during chart redraw to allow plugins to render custom
   * visualization overlays on top of charts. Drawing is done through a device
   * context abstraction (piDC) that handles both standard and OpenGL contexts.
   *
   * Examples include:
   * - Weather routing course lines and markers
   * - GRIB weather data visualization
   * - Custom chart layers
   * - Route tracking and navigation data
   *
   * Rendering order is determined by plugin load order in array for same
   * priority level.
   *
   * @param dc Reference to wxDC device context for drawing
   * @param vp Pointer to current viewport settings (scale, position, rotation)
   * @return True if overlay was rendered, false if no overlay
   *
   * @note Only called if plugin declares WANTS_OVERLAY_CALLBACK capability
   * @note Drawing should respect current viewport parameters
   * @note Performance critical - keep drawing efficient
   * @note Companion to RenderGLOverlay() for OpenGL-specific rendering
   */
  virtual bool RenderOverlay(wxMemoryDC *pmdc, PlugIn_ViewPort *vp);

  /**
   * Receives cursor lat/lon position updates.
   *
   * This method is called when the cursor moves over the chart. Must be
   * implemented if plugin sets WANTS_CURSOR_LATLON flag.
   *
   * @param lat Cursor latitude in decimal degrees
   * @param lon Cursor longitude in decimal degrees
   *
   * @note High frequency updates - keep processing quick
   * @note Only called when cursor over chart window
   */

  virtual void SetCursorLatLon(double lat, double lon);
  /**
   * Notifies plugin of viewport changes.
   *
   * This method is called whenever the chart viewport changes on any canvas due
   * to:
   * - User pan/zoom operations
   * - Periodic canvas updates
   * - Course/heading changes affecting chart orientation
   *
   * For multi-canvas configurations (e.g. split screen), this is called
   * separately for each canvas's viewport changes, regardless of which canvas
   * has focus or mouse interaction.
   *
   * @param vp New viewport parameters
   *
   * @note Called frequently during chart navigation
   * @note Use for maintaining overlay alignment
   */
  virtual void SetCurrentViewPort(PlugIn_ViewPort &vp);

  /**
   * Updates plugin with current position fix data at regular intervals.
   *
   * Called by OpenCPN approximately once per second (1 Hz), regardless of how
   * frequently new position data is actually received from GPS/NMEA sources.
   * This provides plugins with a steady, predictable update rate for navigation
   * calculations and display updates.
   *
   * Plugins can use this to track vessel position, course and speed with
   * consistent timing for smooth navigation displays and calculations.
   *
   * @param pfix Position fix data containing:
   *   - Lat: Latitude in decimal degrees (-90.0 to +90.0, NaN if invalid)
   *   - Lon: Longitude in decimal degrees (-180.0 to +180.0, NaN if invalid)
   *   - Cog: Course over ground in degrees (0-360° true bearing, NaN if
   * invalid)
   *   - Sog: Speed over ground in knots (≥0.0, NaN if invalid)
   *   - Var: Magnetic variation in degrees (NaN if unavailable)
   *   - FixTime: UTC timestamp of fix
   *   - nSats: Number of satellites used in fix (0 if unavailable)
   *
   * @note Only called if plugin declares WANTS_NMEA_EVENTS capability
   * @note For extended data including heading, use SetPositionFixEx()
   */

  virtual void SetPositionFix(PlugIn_Position_Fix &pfix);
  /**
   * Receive all NMEA 0183 sentences from OpenCPN.
   *
   * Plugins can implement this method to receive all NMEA 0183 sentences.
   * They must set the WANTS_NMEA_SENTENCES capability flag to receive updates.
   *
   * @param sentence The NMEA 0183 sentence
   *
   * @note For handling NMEA/SignalK messages, a newer recommended message API
   * is available: \htmlonly <a
   * href="https://opencpn-manuals.github.io/main/opencpn-dev/plugin-messaging.html">Plugin
   * Message API Documentation</a> \endhtmlonly
   */

  virtual void SetNMEASentence(wxString &sentence);
  /**
   * Receive all AIS sentences from OpenCPN.
   *
   * Plugins can implement this method to receive all AIS sentences.
   * They must set the WANTS_AIS_SENTENCES capability flag to receive updates.
   *
   * @param sentence The AIS sentence in standard NMEA 0183 VDM/VDO format
   *                 (e.g., "!AIVDM,1,1,,B,15MwkRUOidG?GElEa<iQk1JV06Jd,0*1D")
   *                 These sentences contain binary encoded AIS messages that
   *                 follow the ITU-R M.1371 standard.
   *
   * @note For handling NMEA/SignalK messages, a newer recommended message API
   * is available: \htmlonly <a
   * href="https://opencpn-manuals.github.io/main/opencpn-dev/plugin-messaging.html">Plugin
   * Message API Documentation</a> \endhtmlonly
   */
  virtual void SetAISSentence(wxString &sentence);

  /**
   * Handles parent window resize events.
   *
   * Called when OpenCPN's main window is resized to allow plugins to
   * adjust their UI elements accordingly.
   *
   * @param x New window width in pixels
   * @param y New window height in pixels
   */
  virtual void ProcessParentResize(int x, int y);

  /**
   * Updates plugin color scheme.
   *
   * Called when OpenCPN's color scheme changes between day, dusk and night
   * modes. Plugins should update their UI colors to match the new scheme.
   *
   * @param cs New color scheme to use:
   *   - PI_GLOBAL_COLOR_SCHEME_DAY
   *   - PI_GLOBAL_COLOR_SCHEME_DUSK
   *   - PI_GLOBAL_COLOR_SCHEME_NIGHT
   */
  virtual void SetColorScheme(PI_ColorScheme cs);

  /**
   * Handles toolbar tool clicks.
   *
   * Called when the user clicks a plugin's toolbar button. Must be implemented
   * if plugin declares WANTS_TOOLBAR_CALLBACK capability.
   *
   * @param id The tool ID assigned when tool was added via InsertPlugInTool()
   */
  virtual void OnToolbarToolCallback(int id);

  /**
   * Handles context menu item selection.
   *
   * Called when user selects a plugin's context menu item. Must be implemented
   * if plugin adds items via AddCanvasContextMenuItem().
   *
   * @param id The menu item ID assigned when item was added
   */
  virtual void OnContextMenuItemCallback(int id);

  /**
   * Updates AUI manager status.
   *
   * Called to notify plugins using wxAUI interface that they should update
   * their AUI managed windows and panes. Must be implemented if plugin declares
   * USES_AUI_MANAGER capability.
   */
  virtual void UpdateAuiStatus(void);

  /**
   * Returns array of dynamically loaded chart class names.
   *
   * Called by OpenCPN to get list of chart classes provided by this plugin.
   * Must be implemented if plugin provides custom chart types.
   *
   * @return Array of strings containing chart class names
   *
   * @note For plugins that implement INSTALLS_PLUGIN_CHART capability
   * @note Custom chart types must also implement PlugInChartBase interface
   */
  virtual wxArrayString GetDynamicChartClassNameArray(void);
};

// the types of the class factories used to create PlugIn instances
typedef opencpn_plugin *create_t(void *);
typedef void destroy_t(opencpn_plugin *);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

class DECL_EXP opencpn_plugin_16 : public opencpn_plugin {
public:
  opencpn_plugin_16(void *pmgr);
  virtual ~opencpn_plugin_16();

  using opencpn_plugin::RenderOverlay;

  /**
   * Renders plugin overlay graphics in standard (non-OpenGL) mode.
   *
   * Called by OpenCPN during chart redraw to allow plugins to render custom
   * visualizations on top of charts. Drawing occurs through piDC device
   * context that abstracts both standard and OpenGL contexts.
   *
   * Plugins render in their load order within the plugin array.
   *
   * @param dc Device context for drawing
   * @param vp Current viewport parameters
   * @return True if overlay was rendered, false if no overlay
   *
   * @note Only called if plugin declares WANTS_OVERLAY_CALLBACK capability
   * @note Drawing should respect current viewport parameters
   * @note For API v1.18+, use RenderOverlayMultiCanvas() to specify priority
   */
  virtual bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);

  /**
   * Receives plugin-to-plugin messages.
   *
   * Called when a plugin sends a message via SendPluginMessage(). Allows
   * plugins to communicate with each other by passing string messages.
   *
   * @param message_id String identifier for the message type
   * @param message_body Message content/payload
   *
   * @note Only called if plugin declares WANTS_PLUGIN_MESSAGING capability
   * @note From API v1.19: Also receives messages from REST interface
   * @note Consider using newer message API via GetListener() for structured
   * data
   */
  virtual void SetPluginMessage(wxString &message_id, wxString &message_body);
};

class DECL_EXP opencpn_plugin_17 : public opencpn_plugin {
public:
  opencpn_plugin_17(void *pmgr);
  virtual ~opencpn_plugin_17();

  using opencpn_plugin::RenderOverlay;

  virtual bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
  virtual bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);

  virtual void SetPluginMessage(wxString &message_id, wxString &message_body);
};

class DECL_EXP opencpn_plugin_18 : public opencpn_plugin {
public:
  opencpn_plugin_18(void *pmgr);
  virtual ~opencpn_plugin_18();

  using opencpn_plugin::RenderOverlay;

  virtual bool RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp);
  /**
   * Renders plugin overlay graphics in OpenGL mode.
   *
   * Called by OpenCPN during chart redraw to allow plugins to render custom
   * visualizations in OpenGL mode. Both RenderOverlay() and RenderGLOverlay()
   * use the same piDC device context abstraction for drawing, but
   * RenderGLOverlay() provides direct access to the OpenGL context.
   *
   * Plugins render in their load order within plugin array
   *
   * @param pcontext OpenGL context for direct GL drawing commands
   * @param vp Current viewport settings (scale, position, rotation)
   * @return True if overlay was rendered, false if no overlay
   *
   * @note Only called if plugin declares WANTS_OPENGL_OVERLAY_CALLBACK
   * capability
   * @note Drawing should respect current viewport parameters
   * @note For OpenGL-specific rendering optimizations
   * @note Modern plugins should use RenderGLOverlayMultiCanvas() for priority
   * control
   */
  virtual bool RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp);
  virtual void SetPluginMessage(wxString &message_id, wxString &message_body);

  /**
   * Updates plugin with extended position fix data at regular intervals.
   *
   * Called by OpenCPN approximately once per second (1 Hz), regardless of how
   * frequently new position data is actually received from GPS/NMEA sources.
   * This provides plugins with a steady, predictable update rate for navigation
   * calculations and display updates.
   *
   * Extends SetPositionFix() by adding magnetic and true heading information
   * when available from HDM/HDT NMEA sentences.
   *
   * @param pfix Extended position fix data containing:
   *   - All basic position fix fields from SetPositionFix()
   *   - Hdm: Magnetic heading in degrees (0-360°, NaN if unavailable)
   *   - Hdt: True heading in degrees (0-360°, NaN if unavailable)
   *
   * @note Only called if plugin declares WANTS_NMEA_EVENTS capability
   *
   * @see SetPositionFix() for basic position data without heading
   */
  virtual void SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix);
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

class DECL_EXP opencpn_plugin_19 : public opencpn_plugin_18 {
public:
  opencpn_plugin_19(void *pmgr);
  virtual ~opencpn_plugin_19();

  /**
   * Allows plugin to add pages to global Options dialog.
   *
   * Called during plugin loading step, after Init().
   * Plugin can add its own pages to the Options dialog.
   *
   */
  virtual void OnSetupOptions(void);
};

class DECL_EXP opencpn_plugin_110 : public opencpn_plugin_19 {
public:
  opencpn_plugin_110(void *pmgr);
  virtual ~opencpn_plugin_110();

  virtual void LateInit(void);  // If WANTS_LATE_INIT is returned by Init()
};

class DECL_EXP opencpn_plugin_111 : public opencpn_plugin_110 {
public:
  opencpn_plugin_111(void *pmgr);
  virtual ~opencpn_plugin_111();
};

class DECL_EXP opencpn_plugin_112 : public opencpn_plugin_111 {
public:
  opencpn_plugin_112(void *pmgr);
  virtual ~opencpn_plugin_112();

  /**
   * Handles mouse events from chart window.
   *
   * Called for mouse events in the chart display area.
   * Must be implemented if plugin declares WANTS_MOUSE_EVENTS capability.
   *
   * @param event The mouse event
   * @return True if event was handled, false to pass to OpenCPN
   */
  virtual bool MouseEventHook(wxMouseEvent &event);

  /**
   * Receives vector chart object information.
   *
   * Called when user clicks vector chart features. Provides details about
   * the selected chart object.
   *
   * @param chart Chart identifier
   * @param feature Feature type
   * @param objname Object name/identifier
   * @param lat Object latitude
   * @param lon Object longitude
   * @param scale Display scale when clicked
   * @param nativescale Chart's native scale
   */
  virtual void SendVectorChartObjectInfo(wxString &chart, wxString &feature,
                                         wxString &objname, double lat,
                                         double lon, double scale,
                                         int nativescale);
};

class DECL_EXP opencpn_plugin_113 : public opencpn_plugin_112 {
public:
  opencpn_plugin_113(void *pmgr);
  virtual ~opencpn_plugin_113();

  /**
   * Handles keyboard events from main window.
   *
   * Called for keyboard events in OpenCPN's main window.
   * Must be implemented if plugin declares WANTS_KEYBOARD_EVENTS capability.
   *
   * @param event The keyboard event
   * @return True if event was handled, false to pass to OpenCPN
   */
  virtual bool KeyboardEventHook(wxKeyEvent &event);

  /**
   * Handles toolbar button press.
   *
   * Called when user presses (mouse down) plugin's toolbar button.
   *
   * @param id The tool ID assigned when tool was added
   */
  virtual void OnToolbarToolDownCallback(int id);

  /**
   * Handles toolbar button release.
   *
   * Called when user releases (mouse up) plugin's toolbar button.
   *
   * @param id The tool ID assigned when tool was added
   */
  virtual void OnToolbarToolUpCallback(int id);
};

class DECL_EXP opencpn_plugin_114 : public opencpn_plugin_113 {
public:
  opencpn_plugin_114(void *pmgr);
  virtual ~opencpn_plugin_114();
};

class DECL_EXP opencpn_plugin_115 : public opencpn_plugin_114 {
public:
  opencpn_plugin_115(void *pmgr);
  virtual ~opencpn_plugin_115();
};

class DECL_EXP opencpn_plugin_116 : public opencpn_plugin_115 {
public:
  opencpn_plugin_116(void *pmgr);
  virtual ~opencpn_plugin_116();
  /**
   * Renders plugin overlay graphics in OpenGL mode with canvas selection.
   *
   * Extended version of RenderGLOverlay() that supports multiple chart canvases
   * and overlay priorities.
   *
   * @param pcontext OpenGL context for drawing
   * @param vp Current viewport parameters
   * @param canvasIndex Index of target canvas (0-based)
   * @param priority Drawing priority level (OVERLAY_LEGACY etc)
   * @return True if overlay was rendered
   */
  virtual bool RenderGLOverlayMultiCanvas(wxGLContext *pcontext,
                                          PlugIn_ViewPort *vp, int canvasIndex);

  /**
   * Renders plugin overlay graphics with canvas selection.
   *
   * Extended version of RenderOverlay() that supports multiple chart canvases
   * and overlay priorities.
   *
   * @param dc Device context for drawing
   * @param vp Current viewport parameters
   * @param canvasIndex Index of target canvas (0-based)
   * @param priority Drawing priority level (OVERLAY_LEGACY etc)
   * @return True if overlay was rendered
   */
  virtual bool RenderOverlayMultiCanvas(wxDC &dc, PlugIn_ViewPort *vp,
                                        int canvasIndex);
  /**
   * Prepares plugin context menu items.
   *
   * Called before showing chart context menu to allow plugin to update its
   * menu items for specific canvas.
   *
   * @param canvasIndex Index of canvas showing menu
   */
  virtual void PrepareContextMenu(int canvasIndex);
};

class DECL_EXP opencpn_plugin_117 : public opencpn_plugin_116 {
public:
  opencpn_plugin_117(void *pmgr);
  /**
   * Forms a semantic version together with GetPlugInVersionMajor() and
   * GetPlugInVersionMinor().
   *
   * @return Patch version number
   */
  virtual int GetPlugInVersionPatch();

  /** Post-release version part, extends the semver spec. */
  virtual int GetPlugInVersionPost();

  /** Pre-release tag version part, see GetPlugInVersionPatch() */
  virtual const char *GetPlugInVersionPre();

  /** Build version part  see GetPlugInVersionPatch(). */
  virtual const char *GetPlugInVersionBuild();

  /*Provide active leg data to plugins*/
  virtual void SetActiveLegInfo(Plugin_Active_Leg_Info &leg_info);
};

class DECL_EXP opencpn_plugin_118 : public opencpn_plugin_117 {
public:
  opencpn_plugin_118(void *pmgr);

  using opencpn_plugin_116::RenderGLOverlayMultiCanvas;
  using opencpn_plugin_116::RenderOverlayMultiCanvas;

  /// Render plugin overlay over chart canvas in OpenGL mode
  ///
  /// \param pcontext Pointer to the OpenGL context
  /// \param vp Pointer to the Viewport
  /// \param canvasIndex Index of the chart canvas, 0 for the first canvas
  /// \param priority Priority, plugins only upgrading from older API versions
  ///        should draw only when priority is OVERLAY_LEGACY (0)
  /// \return true if overlay was rendered, false otherwise
#ifdef _MSC_VER
  virtual bool RenderGLOverlayMultiCanvas(wxGLContext *pcontext,
                                          PlugIn_ViewPort *vp, int canvasIndex,
                                          int priority = -1);
#else
  virtual bool RenderGLOverlayMultiCanvas(wxGLContext *pcontext,
                                          PlugIn_ViewPort *vp, int canvasIndex,
                                          int priority);

  bool RenderGLOverlayMultiCanvas(wxGLContext *pcontext, PlugIn_ViewPort *vp,
                                  int canvas_ix) override {
    return RenderGLOverlayMultiCanvas(pcontext, vp, canvas_ix, -1);
  }
#endif

  /// Render plugin overlay over chart canvas in non-OpenGL mode
  ///
  /// \param dc Reference to the "device context"
  /// \param vp Pointer to the Viewport
  /// \param canvasIndex Index of the chart canvas, 0 for the first canvas
  /// \param priority Priority, plugins only upgrading from older API versions
  ///        should draw only when priority is OVERLAY_LEGACY (0)
  /// \return true if overlay was rendered, false otherwise
#ifdef _MSC_VER
  virtual bool RenderOverlayMultiCanvas(wxDC &dc, PlugIn_ViewPort *vp,
                                        int canvasIndex, int priority = -1);
#else
  virtual bool RenderOverlayMultiCanvas(wxDC &dc, PlugIn_ViewPort *vp,
                                        int canvas_ix, int priority);
  bool RenderOverlayMultiCanvas(wxDC &dc, PlugIn_ViewPort *vp,
                                int canvas_ix) override {
    return RenderOverlayMultiCanvas(dc, vp, canvas_ix, -1);
  }
#endif
};

class DECL_EXP opencpn_plugin_119 : public opencpn_plugin_118 {
public:
  opencpn_plugin_119(void *pmgr);

  /**
   * Called just before OpenCPN exits.
   *
   * Provides an opportunity to perform cleanup and save state before
   * OpenCPN shutdown completes. Called early in the shutdown process before
   * DeInit() and before core subsystems are terminated. Rarely needed.
   *
   * Typical uses include:
   * - Closing network connections
   * - Releasing system resources
   * - Stopping background threads
   * - Terminating external processes
   *
   * @note Only called if plugin declares WANTS_PRESHUTDOWN_HOOK capability
   * @note Plugin should handle this quickly to avoid delaying shutdown
   * @note Core subsystems are still available during this call
   */
  virtual void PreShutdownHook();
};

class DECL_EXP opencpn_plugin_120 : public opencpn_plugin_119 {
public:
  opencpn_plugin_120(void *pmgr);

  virtual void OnContextMenuItemCallbackExt(int id, std::string obj_ident,
                                            std::string obj_type, double lat,
                                            double lon);
};

//------------------------------------------------------------------
//      Route and Waypoint PlugIn support
//
//------------------------------------------------------------------

/**
 * Class representing a hyperlink associated with a waypoint or route.
 *
 * Hyperlinks allow attaching URLs and descriptive text to navigation objects.
 * Common uses include linking to:
 * - Websites with location information
 * - Local files with details/photos
 * - Charts or cruising guides
 * - Weather forecasts
 * - Marina/port information
 */
class DECL_EXP Plugin_Hyperlink {
public:
  wxString DescrText;  //!< User-visible description of the link
  wxString Link;       //!< The actual URL or file path
  wxString Type;  //!< Link type identifier (e.g., "Website", "Chart", "Image")
};

WX_DECLARE_LIST(Plugin_Hyperlink, Plugin_HyperlinkList);

/**
 * Plugin waypoint class providing core waypoint/mark functionality.
 *
 * This class represents both standalone waypoints (marks) and route waypoints.
 * Standalone waypoints appear independently on charts while route waypoints
 * are connected as part of routes.
 *
 * A waypoint may be:
 * - A standalone mark created by the user
 * - A point within a route
 * - An imported waypoint from GPX
 * - A plugin-generated navigation point
 * - A point within a track
 */
class DECL_EXP PlugIn_Waypoint {
public:
  /**
   * Default constructor - creates waypoint at 0,0
   */
  PlugIn_Waypoint();

  /**
   * Creates a waypoint at specified position with icon and name.
   *
   * @param lat Latitude in decimal degrees
   * @param lon Longitude in decimal degrees
   * @param icon_ident Icon name/identifier
   * @param wp_name Waypoint name
   * @param GUID Optional globally unique identifier (empty for auto-generated)
   */
  PlugIn_Waypoint(double lat, double lon, const wxString &icon_ident,
                  const wxString &wp_name, const wxString &GUID = _T(""));
  ~PlugIn_Waypoint();

  double m_lat;                //!< Latitude in decimal degrees
  double m_lon;                //!< Longitude in decimal degrees
  wxString m_GUID;             //!< Globally unique identifier
  wxString m_MarkName;         //!< Display name
  wxString m_MarkDescription;  //!< Optional description
  wxDateTime m_CreateTime;     //!< Creation timestamp in UTC.
  bool m_IsVisible;            //!< Visibility state
  wxString m_IconName;         //!< Icon identifier

  Plugin_HyperlinkList *m_HyperlinkList;  //!< List of associated hyperlinks
};

WX_DECLARE_LIST(PlugIn_Waypoint, Plugin_WaypointList);

/**
 * Plugin route class for managing ordered waypoint collections.
 *
 * This class represents a route consisting of an ordered list of waypoints.
 * Routes can be used for navigation planning and guidance.
 */
class DECL_EXP PlugIn_Route {
public:
  PlugIn_Route(void);
  ~PlugIn_Route(void);

  wxString m_NameString;   //!< Route name
  wxString m_StartString;  //!< Name/description of starting point
  wxString m_EndString;    //!< Name/description of ending point
  wxString m_GUID;         //!< Globally unique identifier

  /**
   * List of waypoints making up this route in order.
   * First point is start, last point is end.
   */
  Plugin_WaypointList *pWaypointList;
};

/**
 * Plugin track class for managing recorded track data.
 *
 * This class represents a track (vessel's recorded path) consisting of an
 * ordered list of track points. Tracks differ from routes in that they
 * represent historical positions rather than planned paths.
 *
 * Tracks are used to:
 * - Record vessel movement history
 * - Display past voyages
 * - Analyze cruise statistics
 * - Export track data for external use
 */
class DECL_EXP PlugIn_Track {
public:
  PlugIn_Track(void);
  ~PlugIn_Track(void);

  wxString m_NameString;   //!< Display name of the track
  wxString m_StartString;  //!< Description of track start point/time
  wxString m_EndString;    //!< Description of track end point/time
  wxString m_GUID;         //!< Globally unique identifier

  /**
   * List of waypoints making up this track in chronological order.
   * First point is start time, last point is end time.
   * Each point typically includes position, time, and optional data
   * like depth or temperature.
   */
  Plugin_WaypointList *pWaypointList;
};

//----------------------------------------------------------------------------------------------------------
//    The PlugIn CallBack API Definition
//
//    The API back up to the PlugIn Manager
//    PlugIns may call these static functions as necessary for system services
//
//----------------------------------------------------------------------------------------------------------

/**
 * Adds a tool to OpenCPN's toolbar.
 *
 * This method creates and adds a new tool button to OpenCPN's main toolbar.
 *
 * @param label Tool identifier and internal name
 * @param bitmap Normal state bitmap
 * @param bmpRollover Bitmap shown on mouseover (optional)
 * @param kind Tool type (normal, toggle, radio button etc)
 * @param shortHelp Tooltip text
 * @param longHelp Detailed help text
 * @param clientData Optional user data pointer
 * @param position Position in toolbar (-1 for end)
 * @param tool_sel Tool selection mode
 * @param pplugin Pointer to plugin instance
 * @return Tool ID for later reference, -1 if failed
 *
 * @note Tool IDs are managed internally and should be stored for later use
 * @note Position parameter allows inserting tools at specific locations
 */
extern "C" DECL_EXP int InsertPlugInTool(wxString label, wxBitmap *bitmap,
                                         wxBitmap *bmpRollover, wxItemKind kind,
                                         wxString shortHelp, wxString longHelp,
                                         wxObject *clientData, int position,
                                         int tool_sel, opencpn_plugin *pplugin);
/**
 * Removes a tool from OpenCPN's toolbar.
 *
 * This method removes a previously added plugin tool from the toolbar.
 *
 * @param tool_id Tool ID returned by InsertPlugInTool()
 */
extern "C" DECL_EXP void RemovePlugInTool(int tool_id);

/**
 * Temporarily changes toolbar tool visibility.
 *
 * Shows or hides a toolbar tool without removing it.
 *
 * @param item Tool ID to modify
 * @param viz True to show tool, false to hide
 */
extern "C" DECL_EXP void SetToolbarToolViz(int item, bool viz);

/**
 * Sets toolbar item toggle state.
 *
 * Sets the pressed/unpressed state for toggle type tools.
 *
 * @param item Tool ID to modify
 * @param toggle True for pressed state, false for unpressed
 */
extern "C" DECL_EXP void SetToolbarItemState(int item, bool toggle);

/**
 * Updates toolbar tool bitmaps.
 *
 * Changes the normal and rollover bitmaps for an existing tool.
 *
 * @param item Tool ID to modify
 * @param bitmap New normal state bitmap
 * @param bmpRollover New rollover bitmap
 */
extern "C" DECL_EXP void SetToolbarToolBitmaps(int item, wxBitmap *bitmap,
                                               wxBitmap *bmpRollover);
/**
 * Adds a tool using SVG graphics.
 *
 * Like InsertPlugInTool but using SVG files for scalable graphics.
 *
 * @param label Tool identifier and internal name
 * @param SVGfile Normal state SVG file path
 * @param SVGfileRollover Rollover state SVG file path
 * @param SVGfileToggled Toggled state SVG file path
 * @param kind Tool type (normal, toggle etc)
 * @param shortHelp Tooltip text
 * @param longHelp Detailed help text
 * @param clientData Optional user data
 * @param position Position in toolbar (-1 for end)
 * @param tool_sel Tool selection mode
 * @param pplugin Pointer to plugin instance
 * @return Tool ID or -1 if failed
 */
extern "C" DECL_EXP int InsertPlugInToolSVG(
    wxString label, wxString SVGfile, wxString SVGfileRollover,
    wxString SVGfileToggled, wxItemKind kind, wxString shortHelp,
    wxString longHelp, wxObject *clientData, int position, int tool_sel,
    opencpn_plugin *pplugin);

/**
 * Updates SVG graphics for toolbar tool.
 *
 * Changes the SVG files used for a tool's different states.
 *
 * @param item Tool ID to modify
 * @param SVGfile Normal state SVG file
 * @param SVGfileRollover Rollover state SVG file
 * @param SVGfileToggled Toggled state SVG file
 */
extern "C" DECL_EXP void SetToolbarToolBitmapsSVG(int item, wxString SVGfile,
                                                  wxString SVGfileRollover,
                                                  wxString SVGfileToggled);
/**
 * Adds item to chart canvas context menu.
 *
 * This method adds a menu item to the right-click context menu.
 *
 * @param pitem Menu item to add
 * @param pplugin Pointer to plugin instance
 * @return Item ID for later reference
 */
extern "C" DECL_EXP int AddCanvasContextMenuItem(wxMenuItem *pitem,
                                                 opencpn_plugin *pplugin);
/**
 * Removes a context menu item completely.
 *
 * Unlike SetCanvasContextMenuItemViz(), this permanently removes the menu item.
 *
 * @param item Menu item ID returned from AddCanvasContextMenuItem()
 */
extern "C" DECL_EXP void RemoveCanvasContextMenuItem(int item);

/**
 * Temporarily changes context menu item visibility.
 *
 * Hides or shows a context menu item without removing it.
 *
 * @param item Menu item ID to modify
 * @param viz True to show item, false to hide
 */
extern "C" DECL_EXP void SetCanvasContextMenuItemViz(int item, bool viz);

/**
 * Sets menu item enabled/disabled state.
 *
 * Greys out (disables) or enables a context menu item.
 *
 * @param item Menu item ID to modify
 * @param grey True to disable (grey out), false to enable
 */
extern "C" DECL_EXP void SetCanvasContextMenuItemGrey(int item, bool grey);

/**
 * Gets OpenCPN's configuration object.
 *
 * Provides access to OpenCPN's wxFileConfig object for reading/writing
 * settings.
 *
 * @return Pointer to global config object
 * @note Do not delete the returned pointer
 */
extern "C" DECL_EXP wxFileConfig *GetOCPNConfigObject(void);

/**
 * Requests window refresh.
 *
 * Triggers a redraw of the specified window.
 *
 * @param window Window to refresh
 */
extern "C" DECL_EXP void RequestRefresh(wxWindow *);

/**
 * Gets a global color value.
 *
 * Retrieves color values from OpenCPN's color scheme system.
 *
 * @param colorName Name of the color to retrieve
 * @param pcolour Pointer to wxColour to receive the color value
 * @return True if color was found, false if not
 */
extern "C" DECL_EXP bool GetGlobalColor(wxString colorName, wxColour *pcolour);

/**
 * Converts lat/lon to canvas physical pixel coordinates.
 *
 * Transforms geographic coordinates to screen physical pixels for the given
 * viewport.
 *
 * @param vp Current viewport
 * @param pp Will receive physical pixel coordinates
 * @param lat Latitude in decimal degrees
 * @param lon Longitude in decimal degrees
 */
extern "C" DECL_EXP void GetCanvasPixLL(PlugIn_ViewPort *vp, wxPoint *pp,
                                        double lat, double lon);
/**
 * Converts canvas physical pixel coordinates to lat/lon.
 *
 * Transforms screen physical pixels to geographic coordinates for the given
 * viewport.
 *
 * @param vp Current viewport
 * @param p Physical pixel coordinates
 * @param plat Will receive latitude in decimal degrees
 * @param plon Will receive longitude in decimal degrees
 */
extern "C" DECL_EXP void GetCanvasLLPix(PlugIn_ViewPort *vp, wxPoint p,
                                        double *plat, double *plon);
/**
 * Gets OpenCPN's main canvas window.
 *
 * Provides access to the main chart display window.
 *
 * @return Pointer to main canvas window
 */
extern "C" DECL_EXP wxWindow *GetOCPNCanvasWindow();

/**
 * Gets a font for UI elements.
 *
 * Plugins can use this to access OpenCPN's font management system which
 * supports locale-dependent fonts and colors. Font configurations are cached
 * and shared to minimize memory usage.
 *
 * @param TextElement UI element identifier. Supported values:
 *   - "AISTargetAlert": AIS alert messages
 *   - "AISTargetQuery": AIS information dialogs
 *   - "StatusBar": Main status bar text
 *   - "AIS Target Name": Labels for AIS targets
 *   - "ObjectQuery": Chart object information text
 *   - "RouteLegInfoRollover": Route information hover windows
 *   - "ExtendedTideIcon": Text on extended tide icons
 *   - "CurrentValue": Current measurement values
 *   - "Console Legend": Console text labels (e.g. "XTE", "BRG")
 *   - "Console Value": Console numeric values
 *   - "AISRollover": AIS target rollover text
 *   - "TideCurrentGraphRollover": Tide and current graph hover text
 *   - "Marks": Waypoint label text
 *   - "ChartTexts": Text rendered directly on charts
 *   - "ToolTips": Tooltip text
 *   - "Dialog": Dialog box and control panel text
 *   - "Menu": Menu item text
 *   - "GridText": Grid annotation text
 *
 * @param default_size Font size in points, 0 to use system default size
 * @return Pointer to configured wxFont, do not delete it
 *
 * @note Each UI element can have different fonts per locale to support
 * language-specific fonts. Color is also managed - use OCPNGetFontColor()
 * to get the configured color.
 * @note The "console" in OpenCPN displays key navigation data such as Cross
 * Track Error (XTE), Bearing (BRG), Velocity Made Good (VMG), Range (RNG), and
 * Time to Go (TTG). By default, the text is large and green, optimized for
 * visibility.
 */
extern "C" DECL_EXP wxFont *OCPNGetFont(wxString TextElement,
                                        int default_size = 0);

/**
 * Gets shared application data location.
 *
 * Returns path to OpenCPN's shared data directory containing resources like:
 * - Chart catalog files
 * - S57 overlay files
 * - World map data
 *
 * @return Pointer to wxString containing shared data path
 * @note Do not delete the returned pointer
 * @note Path includes trailing separator
 */
extern "C" DECL_EXP wxString *GetpSharedDataLocation();

/**
 * Gets array of AIS targets.
 *
 * Returns array of all currently tracked AIS targets.
 *
 * @return Pointer to array of PlugIn_AIS_Target pointers
 * @note Array contents owned by core - do not delete targets
 */
extern "C" DECL_EXP ArrayOfPlugIn_AIS_Targets *GetAISTargetArray(void);

/**
 * Gets main frame AUI manager.
 *
 * Provides access to wxAuiManager controlling OpenCPN's main frame layout.
 * Used by plugins that need to add/manage dockable windows.
 *
 * @return Pointer to main frame wxAuiManager
 * @note Only available if plugin declares USES_AUI_MANAGER capability
 */
extern "C" DECL_EXP wxAuiManager *GetFrameAuiManager(void);

/**
 * Adds a locale catalog for translations.
 *
 * Adds a plugin's translation catalog to OpenCPN's locale system.
 *
 * @param catalog Path to translation catalog file
 * @return True if catalog was loaded successfully
 */
extern "C" DECL_EXP bool AddLocaleCatalog(wxString catalog);

/**
 * Pushes NMEA sentence to the system.
 *
 * Adds an NMEA sentence to OpenCPN's data stream as if received from device.
 *
 * @param str Complete NMEA sentence including checksum
 */
extern "C" DECL_EXP void PushNMEABuffer(wxString str);

/**
 * Gets chart database entry as XML.
 *
 * Retrieves XML representation of chart database entry.
 *
 * @param dbIndex Index of chart in database
 * @param b_getGeom True to include chart geometry info
 * @return wxXmlDocument containing chart data
 */
extern DECL_EXP wxXmlDocument GetChartDatabaseEntryXML(int dbIndex,
                                                       bool b_getGeom);
/**
 * Updates chart database in place.
 *
 * Updates chart database with new chart directories.
 *
 * @param dir_array Array of directory paths to scan
 * @param b_force_update Force full update even if no changes detected
 * @param b_ProgressDialog Show progress dialog during update
 * @return True if database was updated
 */
extern DECL_EXP bool UpdateChartDBInplace(wxArrayString dir_array,
                                          bool b_force_update,
                                          bool b_ProgressDialog);
/**
 * Gets chart database directory list.
 *
 * Returns array of chart directories in database.
 *
 * @return Array of directory paths as strings
 */
extern DECL_EXP wxArrayString GetChartDBDirArrayString();

/**
 * Sends message to other plugins.
 *
 * Broadcasts a message that other plugins can receive.
 *
 * @param message_id Message identifier
 * @param message_body Message content
 */
extern "C" DECL_EXP void SendPluginMessage(wxString message_id,
                                           wxString message_body);
/**
 * Applies system color scheme to window.
 *
 * Adjusts window colors to match OpenCPN's color scheme.
 *
 * @param window Window to adjust colors for
 */
extern "C" DECL_EXP void DimeWindow(wxWindow *);

/**
 * Centers chart display on specified position at given scale.
 *
 * @param lat Latitude in decimal degrees
 * @param lon Longitude in decimal degrees
 * @param scale Display scale factor
 */
extern "C" DECL_EXP void JumpToPosition(double lat, double lon, double scale);

/* API 1.9  adds some common cartographic functions to avoid unnecessary code
 * duplication */
/* Study the original OpenCPN source (georef.c) for functional definitions  */

/**
 * Calculates destination point given starting point, bearing and distance.
 *
 * Compute the destination coordinates when traveling from a
 * starting position along a specified bearing for a given distance. Despite
 * the name suggesting Mercator calculations, this function actually uses
 * great circle (geodesic) calculations via the WGS84 ellipsoid model for
 * maximum accuracy.
 *
 * @param lat Starting point latitude in decimal degrees (-90.0 to +90.0)
 * @param lon Starting point longitude in decimal degrees (-180.0 to +180.0)
 * @param brg Initial bearing in degrees true (0.0-359.9°, 0°=North, 90°=East)
 * @param dist Distance to travel in nautical miles (≥0.0)
 * @param dlat Pointer to store destination latitude in decimal degrees
 *             Cannot be NULL
 * @param dlon Pointer to store destination longitude in decimal degrees
 *             in [-180,+180] range. Cannot be NULL
 *
 * @note Uses WGS84 ellipsoid great circle calculations, not rhumb line
 * @note If dist=0.0, returns dlat=lat and dlon=lon (same position)
 * @note Output longitude is automatically normalized to [-180,+180] range
 * @note Handles all bearing values, including those ≥360° (automatically
 * normalized)
 */
extern "C" DECL_EXP void PositionBearingDistanceMercator_Plugin(
    double lat, double lon, double brg, double dist, double *dlat,
    double *dlon);
/**
 * Calculates bearing and distance between two points using Mercator projection.
 *
 * Compute the rhumb line (constant bearing) distance and initial
 * bearing between two geographic positions using Mercator projection.
 *
 * The algorithm automatically handles dateline crossings by selecting the
 * shortest route, adjusting longitude differences that exceed ±180°.
 *
 * @param lat0 Starting point latitude in decimal degrees (-90.0 to +90.0)
 * @param lon0 Starting point longitude in decimal degrees (-180.0 to +180.0)
 * @param lat1 Ending point latitude in decimal degrees (-90.0 to +90.0)
 * @param lon1 Ending point longitude in decimal degrees (-180.0 to +180.0)
 * @param brg Pointer to store initial bearing in degrees true [0.0-360.0°)
 *            Can be NULL if bearing is not needed
 * @param dist Pointer to store distance in nautical miles (≥0.0)
 *             Can be NULL if distance is not needed
 *
 * @note If lat0/lon0 equals lat1/lon1, returns bearing=180.0° and distance=0.0
 * NM
 * @see DistGreatCircle_Plugin() for great circle distance calculations
 */
extern "C" DECL_EXP void DistanceBearingMercator_Plugin(
    double lat0, double lon0, double lat1, double lon1, double *brg,
    double *dist);

/**
 * Calculates great circle distance between two points.
 *
 * @param slat Start latitude in decimal degrees
 * @param slon Start longitude in decimal degrees
 * @param dlat End latitude in decimal degrees
 * @param dlon End longitude in decimal degrees
 * @return Distance in nautical miles
 */
extern "C" DECL_EXP double DistGreatCircle_Plugin(double slat, double slon,
                                                  double dlat, double dlon);
/**
 * Converts geographic coordinates to Transverse Mercator projection.
 *
 * @param lat Latitude in decimal degrees
 * @param lon Longitude in decimal degrees
 * @param lat0 Reference latitude
 * @param lon0 Central meridian
 * @param x Easting coordinate (output)
 * @param y Northing coordinate (output)
 */
extern "C" DECL_EXP void toTM_Plugin(float lat, float lon, float lat0,
                                     float lon0, double *x, double *y);

/**
 * Converts Transverse Mercator coordinates to geographic.
 *
 * @param x Easting coordinate
 * @param y Northing coordinate
 * @param lat0 Reference latitude
 * @param lon0 Central meridian
 * @param lat Latitude in decimal degrees (output)
 * @param lon Longitude in decimal degrees (output)
 */
extern "C" DECL_EXP void fromTM_Plugin(double x, double y, double lat0,
                                       double lon0, double *lat, double *lon);

/**
 * Converts geographic coordinates to Simple Mercator projection.
 *
 * @param lat Latitude in decimal degrees
 * @param lon Longitude in decimal degrees
 * @param lat0 Reference latitude
 * @param lon0 Central meridian
 * @param x Easting coordinate (output)
 * @param y Northing coordinate (output)
 */
extern "C" DECL_EXP void toSM_Plugin(double lat, double lon, double lat0,
                                     double lon0, double *x, double *y);

/**
 * Converts Simple Mercator coordinates to geographic.
 *
 * @param x Easting coordinate
 * @param y Northing coordinate
 * @param lat0 Reference latitude
 * @param lon0 Central meridian
 * @param lat Latitude in decimal degrees (output)
 * @param lon Longitude in decimal degrees (output)
 */
extern "C" DECL_EXP void fromSM_Plugin(double x, double y, double lat0,
                                       double lon0, double *lat, double *lon);

/**
 * Converts geographic coordinates to Elliptical Simple Mercator projection.
 *
 * @param lat Latitude in decimal degrees
 * @param lon Longitude in decimal degrees
 * @param lat0 Reference latitude
 * @param lon0 Central meridian
 * @param x Easting coordinate (output)
 * @param y Northing coordinate (output)
 */
extern "C" DECL_EXP void toSM_ECC_Plugin(double lat, double lon, double lat0,
                                         double lon0, double *x, double *y);

/**
 * Converts Elliptical Simple Mercator coordinates to geographic.
 *
 * @param x Easting coordinate
 * @param y Northing coordinate
 * @param lat0 Reference latitude
 * @param lon0 Central meridian
 * @param lat Latitude in decimal degrees (output)
 * @param lon Longitude in decimal degrees (output)
 */
extern "C" DECL_EXP void fromSM_ECC_Plugin(double x, double y, double lat0,
                                           double lon0, double *lat,
                                           double *lon);

/**
 * Decodes a single VDO (Own Ship AIS) message.
 *
 * Parses a VDO sentence and extracts position and accuracy information.
 *
 * @param str The raw VDO sentence
 * @param pos Will receive decoded position data
 * @param acc Will receive accuracy information
 * @return True if successfully decoded
 */
extern "C" DECL_EXP bool DecodeSingleVDOMessage(const wxString &str,
                                                PlugIn_Position_Fix_Ex *pos,
                                                wxString *acc);

/**
 * Gets height of chart bar in pixels.
 *
 * @return Height of chart bar widget
 */

extern "C" DECL_EXP int GetChartbarHeight(void);
/**
 * Gets GPX representation of active route waypoint.
 *
 * Retrieves an XML/GPX format description of the currently active route
 * waypoint. The buffer will contain a GPX formatted waypoint like:
 *
 * <?xml version="1.0"?>
 * <wpt lat="50.123456" lon="-4.123456">
 *   <name>Waypoint Name</name>
 *   <sym>triangle</sym>
 *   <type>WPT</type>
 *   <time>2024-03-12T10:30:00Z</time>
 *   <extensions>
 *     <opencpn:guid>12345678-1234-1234-1234-123456789012</opencpn:guid>
 *     <opencpn:viz>1</opencpn:viz>
 *     <opencpn:viz_name>0</opencpn:viz_name>
 *   </extensions>
 * </wpt>
 *
 * @param buffer Pre-allocated char buffer to receive GPX data
 * @param buffer_length Size of provided buffer in bytes
 * @return True if waypoint data was written to buffer, false if
 *         - No active waypoint exists
 *         - Buffer is too small
 *         - Other error occurred
 *
 * @note Buffer must be pre-allocated and large enough for GPX data
 * @note Recommended minimum buffer size is 4096 bytes
 * @note XML is formatted with newlines and indentation for readability
 */
extern "C" DECL_EXP bool GetActiveRoutepointGPX(char *buffer,
                                                unsigned int buffer_length);

/* API 1.9 */

/**
 * Enum defining parent pages in OpenCPN options dialog where plugins can add
 * panels.
 *
 * When adding plugin preference panels using AddOptionsPage(), the plugin must
 * specify which main section should contain the new panel.
 */
typedef enum OptionsParentPI {
  /** Display settings section. For options affecting chart display, colors,
     etc. */
  PI_OPTIONS_PARENT_DISPLAY,

  /** Connections section. For options affecting data connections, ports, etc.
   */
  PI_OPTIONS_PARENT_CONNECTIONS,

  /** Charts section. For options affecting chart loading, display, etc. */
  PI_OPTIONS_PARENT_CHARTS,

  /** Ships section. For options affecting vessel display, AIS, etc. */
  PI_OPTIONS_PARENT_SHIPS,

  /** User Interface section. For options affecting toolbars, interface, etc. */
  PI_OPTIONS_PARENT_UI,

  /** Plugins section. For plugin-specific settings and configuration. */
  PI_OPTIONS_PARENT_PLUGINS
} _OptionsParentPI;

/**
 * Adds a new preferences page to OpenCPN options dialog.
 *
 * Creates a new panel in the specified section of the options dialog.
 * Plugin should populate the returned wxScrolledWindow with preference
 * controls.
 *
 * @param parent Parent section to add page to (see OptionsParentPI enum)
 * @param title Title shown for the new page
 * @return Pointer to wxScrolledWindow to populate with controls, NULL if error
 *
 * @note Plugin must handle saving/loading settings when page closes
 * @note Use standard wxWidgets controls for consistent look
 */
extern DECL_EXP wxScrolledWindow *AddOptionsPage(OptionsParentPI parent,
                                                 wxString title);

/**
 * Remove a previously added options page. Should be called from  plugin's
 * DeInit() method.
 *
 * @param page Pointer to page previously returned by AddOptionsPage()
 * @return True if page was successfully removed
 */
extern DECL_EXP bool DeleteOptionsPage(wxScrolledWindow *page);

/* API 1.10  */

/* API 1.10  adds some common functions to avoid unnecessary code duplication */
/* Study the original OpenCPN source for functional definitions  */
/**
 * Converts nautical miles to user's preferred distance unit.
 *
 * @param nm_distance Distance value in nautical miles
 * @param unit Override unit choice (-1 for user preference):
 *             0=nm, 1=km, 2=mi, 3=fathoms
 * @return Distance in user's preferred unit
 *
 * @note Default unit is determined by OpenCPN settings if unit=-1
 */
extern "C" DECL_EXP double toUsrDistance_Plugin(double nm_distance,
                                                int unit = -1);

/**
 * Converts from user's preferred distance unit to nautical miles.
 *
 * @param usr_distance Distance value in user's unit
 * @param unit Override unit choice (-1 for user preference):
 *             0=nm, 1=km, 2=mi, 3=fathoms
 * @return Distance in nautical miles
 */
extern "C" DECL_EXP double fromUsrDistance_Plugin(double usr_distance,
                                                  int unit = -1);

/**
 * Converts knots to user's preferred speed unit.
 *
 * @param kts_speed Speed value in knots
 * @param unit Override unit choice (-1 for user preference):
 *             0=knots, 1=km/h, 2=mph, 3=m/s
 * @return Speed in user's preferred unit
 */
extern "C" DECL_EXP double toUsrSpeed_Plugin(double kts_speed, int unit = -1);

/**
 * Converts from user's preferred speed unit to knots.
 *
 * @param usr_speed Speed value in user's unit
 * @param unit Override unit choice (-1 for user preference):
 *             0=knots, 1=km/h, 2=mph, 3=m/s
 * @return Speed in knots
 */
extern "C" DECL_EXP double fromUsrSpeed_Plugin(double usr_speed, int unit = -1);

/**
 * Converts Celsius to user's preferred temperature unit.
 *
 * @param cel_temp Temperature in Celsius
 * @param unit Override unit choice (-1 for user preference):
 *             0=Celsius, 1=Fahrenheit
 * @return Temperature in user's preferred unit
 */
extern "C" DECL_EXP double toUsrTemp_Plugin(double cel_temp, int unit = -1);

/**
 * Converts from user's preferred temperature unit to Celsius.
 *
 * @param usr_temp Temperature in user's unit
 * @param unit Override unit choice (-1 for user preference):
 *             0=Celsius, 1=Fahrenheit
 * @return Temperature in Celsius
 */
extern "C" DECL_EXP double fromUsrTemp_Plugin(double usr_temp, int unit = -1);

/**
 * Gets display string for user's preferred distance unit.
 *
 * @param unit Override unit choice (-1 for user preference):
 *             0=nm, 1=km, 2=mi, 3=fathoms
 * @return Localized unit string (e.g. "nm", "km", "mi", "fathoms")
 */
extern DECL_EXP wxString getUsrDistanceUnit_Plugin(int unit = -1);

/**
 * Gets display string for user's preferred speed unit.
 *
 * @param unit Override unit choice (-1 for user preference):
 *             0=knots, 1=km/h, 2=mph, 3=m/s
 * @return Localized unit string (e.g. "kts", "km/h", "mph", "m/s")
 */
extern DECL_EXP wxString getUsrSpeedUnit_Plugin(int unit = -1);

/**
 * Gets display string for user's preferred temperature unit.
 *
 * @param unit Override unit choice (-1 for user preference):
 *             0=Celsius, 1=Fahrenheit, 2=Kelvin
 * @return Localized unit string ("°C", "°F", or "K")
 */
extern DECL_EXP wxString getUsrTempUnit_Plugin(int unit = -1);

/**
 * Gets display string for user's preferred wind speed unit.
 *
 * @param unit Override unit choice (-1 for user preference):
 *             0=knots, 1=m/s, 2=mph, 3=km/h
 * @return Localized unit string (e.g. "kts", "m/s", "mph", "km/h")
 */
extern DECL_EXP wxString getUsrWindSpeedUnit_Plugin(int unit = -1);

/**
 * Converts knots to user's preferred wind speed unit.
 *
 * @param kts_wspeed Wind speed in knots
 * @param unit Override unit choice (-1 for user preference):
 *             0=knots, 1=m/s, 2=mph, 3=km/h
 * @return Wind speed in user's preferred unit
 */
extern DECL_EXP double toUsrWindSpeed_Plugin(double kts_wspeed, int unit = -1);

/**
 * Converts from user's preferred wind speed unit to knots.
 *
 * @param usr_wspeed Wind speed in user's unit
 * @param unit Override unit choice (-1 for user preference):
 *             0=knots, 1=m/s, 2=mph, 3=km/h
 * @return Wind speed in knots
 */
extern DECL_EXP double fromUsrWindSpeed_Plugin(double usr_wspeed,
                                               int unit = -1);

/**
 * Gets display string for user's preferred depth unit.
 *
 * @param unit Override unit choice (-1 for user preference):
 *             0=feet, 1=meters, 2=fathoms
 * @return Localized unit string (e.g. "ft", "m", "fa")
 */
extern DECL_EXP wxString getUsrDepthUnit_Plugin(int unit = -1);

/**
 * Converts meters to user's preferred depth unit.
 *
 * @param m_depth Depth in meters
 * @param unit Override unit choice (-1 for user preference):
 *             0=feet, 1=meters, 2=fathoms
 * @return Depth in user's preferred unit
 */
extern DECL_EXP double toUsrDepth_Plugin(double m_depth, int unit = -1);

/**
 * Converts from user's preferred depth unit to meters.
 *
 * @param usr_depth Depth in user's unit
 * @param unit Override unit choice (-1 for user preference):
 *             0=feet, 1=meters, 2=fathoms
 * @return Depth in meters
 */
extern DECL_EXP double fromUsrDepth_Plugin(double usr_depth, int unit = -1);

/**
 * Parse a formatted coordinate string to get decimal degrees.
 *
 * Attempt to parse a wide variety of formatted coordinate
 * strings and convert them to decimal degrees. It handles formats like:
 * - 37°54.204' N
 * - N37 54 12
 * - 37°54'12"
 * - 37.9034
 * - 122°18.621' W
 * - 122w 18 37
 * - -122.31035
 *
 * @param sdms The formatted coordinate string to parse
 * @return The decimal degrees value, or NaN if parsing failed
 */
extern DECL_EXP double fromDMM_PlugIn(wxString sdms);

/**
 * Configuration options for date and time formatting.
 *
 * Formatting options that determine how dates and times
 * are displayed throughout the application. It allows configuring format
 * strings, timezone settings, and geographic reference for local time
 * calculations.
 *
 * The format settings use standard date/time format specifiers (like strftime)
 * for creating custom date/time representations based on user preferences.
 * Timezone settings allow displaying times in UTC, system local time, or a
 * custom zone based on the vessel's current position.
 */
struct DECL_EXP DateTimeFormatOptions {
  DateTimeFormatOptions();
  virtual ~DateTimeFormatOptions();

  /**
   * The format string for date/time.
   *
   * The following predefined format strings are supported:
   * - "$long_date": Thursday December 31, 2021.
   * - "$short_date": 12/31/2021.
   * - "$weekday_short_date": Thu 12/31/2021.
   * - "$hour_minutes_seconds": 15:34:56 zero-padded.
   * - "$hour_minutes": 15:34 zero-padded.
   * - "$long_date_time": Thursday December 31, 2021 12:34:56.
   * - "$short_date_time": 12/31/2021 15:34:56.
   * - "$weekday_short_date_time": Thu 12/31/2021 15:34:56.
   *
   * The default is $weekday_short_date_time.
   *
   * The descriptors are resolved to localized date/time string representations.
   * For example, $short_date is resolved to "12/31/2021" in the US locale and
   * "31/12/2021" in the UK locale.
   */
  wxString format_string;

  /**
   * The timezone to use when formatting the date/time. Supported options are:
   * - Empty string (default): the date/time is formatted according to
   *   the OpenCPN global settings. This should be used to ensure consistency
   *   of the date/time representation across the entire application.
   * - "UTC": the date/time is formatted in UTC, regardless of the OpenCPN
   *   global settings.
   * - "Local Time": the date/time is formatted in the local time, regardless of
   *   the OpenCPN global settings.
   *
   * @note In the future, additional timezone options may be supported:
   * - "LMT": the date/time is formatted in local mean time. In this
   *   case, longitude is required.
   * - Valid timezone name: the date/time is formatted in that timezone.
   */
  wxString time_zone;

  /**
   * Whether to show timezone information in the formatted date/time string.
   * When true, the timezone abbreviation (e.g., "EDT", "UTC", "LMT") is
   * appended to the date/time string. When false, no timezone information
   * is included.
   */
  bool show_timezone;

  /**
   * The longitude to use when formatting the date/time in Local Mean Time
   * (LMT). The longitude is required when the time_zone is set to "LMT".
   */
  double longitude;

  int version = 1;  // For future compatibility checks

  /**
   * Sets the date/time format pattern string.
   *
   * @param format String containing date/time format specifiers.
   *
   * This method configures the format pattern used when displaying dates and
   * times. The following predefined format strings are supported:
   * - "$long_date": Thursday December 31, 2021.
   * - "$short_date": 12/31/2021.
   * - "$weekday_short_date": Thu 12/31/2021.
   * - "$hour_minutes_seconds": 15:34:56 zero-padded.
   * - "$hour_minutes": 15:34 zero-padded.
   * - "$long_date_time": Thursday December 31, 2021 12:34:56.
   * - "$short_date_time": 12/31/2021 15:34:56.
   * - "$weekday_short_date_time": Thu 12/31/2021 15:34:56.
   *
   * The default is $weekday_short_date_time.
   */
  DateTimeFormatOptions &SetFormatString(const wxString &fmt) {
    format_string = fmt;
    return *this;
  }

  /**
   * Sets the timezone mode for date/time display
   *
   * @param timezone Specifies the timezone mode:
   * - Empty string (default): the date/time is formatted according to
   *   the OpenCPN global settings. This should be used to ensure consistency
   *   of the date/time representation across the entire application.
   * - "UTC": the date/time is formatted in UTC, regardless of the OpenCPN
   *   global settings.
   * - "Local Time": the date/time is formatted in the local time, regardless of
   *   the OpenCPN global settings.
   *
   * This method configures how time values are adjusted for timezone display.
   * - When set to empty or UTC, all times are shown in universal time without
   *   adjustment.
   * - When set to "Local Time", times are adjusted to match the
   *   timezone settings of the computer running OpenCPN.
   */
  DateTimeFormatOptions &SetTimezone(const wxString &tz) {
    time_zone = tz;
    return *this;
  }

  /**
   * Sets whether to show timezone information in the formatted output.
   *
   * @param show Whether to show timezone information
   * @return Reference to this object for method chaining
   */
  DateTimeFormatOptions &SetShowTimezone(bool show) {
    show_timezone = show;
    return *this;
  }

  /**
   * Sets the reference longitude for Local Mean Time (LMT) calculations.
   *
   * @param lon Longitude in decimal degrees (-180 to +180)
   *
   * When timezone mode is set to local time at vessel position,
   * this method provides the longitude used to calculate the Local Mean Time
   * (LMT).
   *
   * LMT is calculated based on the sun's position relative to the local
   * meridian, with solar noon occurring when the sun crosses the meridian. Each
   * 15 degrees of longitude represents approximately 1 hour of time difference.
   *
   * Unlike standard timezone offsets which use fixed boundaries, LMT provides a
   * continuous time representation based precisely on the vessel's longitude,
   * useful for celestial navigation and traditional maritime timekeeping.
   */
  DateTimeFormatOptions &SetLongitude(double lon) {
    longitude = lon;
    return *this;
  }
};

/**
 * Format a date/time to a localized string representation, conforming to the
 * global date/time format and timezone settings.
 *
 * The function expects date_time to be in local time and formats it according
 * to the timezone configuration either in:
 * - UTC: Coordinated Universal Time (default)
 * - Local Time: System's configured timezone with proper DST handling
 * - LMT: Local Mean Time calculated based on the longitude specified in options
 *
 * @note This function should be used instead of wxDateTime.Format() to ensure
 * consistent date/time formatting across the entire application.
 *
 * @param date_time The date/time to format, must be in local time.
 * @param options The date/time format options including target timezone and
 * formatting preferences.
 * @return wxString The formatted date/time string with appropriate timezone
 * indicator.
 */
extern DECL_EXP wxString toUsrDateTimeFormat_Plugin(
    const wxDateTime date_time,
    const DateTimeFormatOptions &options = DateTimeFormatOptions());

/**
 * Generates a new globally unique identifier (GUID).
 *
 * Creates a unique string identifier suitable for waypoints, routes, etc.
 * Format is RFC 4122 compliant UUID string.
 *
 * @return wxString containing new GUID
 *
 * @note Returns format like "6ba7b810-9dad-11d1-80b4-00c04fd430c8"
 * @note Thread-safe and collision resistant
 */
extern DECL_EXP wxString GetNewGUID();

/**
 * Checks if a great circle route crosses land.
 *
 * Tests if a direct path between two points intersects with land using
 * GSHHS (Global Self-consistent Hierarchical High-resolution Shorelines) data.
 *
 * @param lat1 Start latitude in decimal degrees
 * @param lon1 Start longitude in decimal degrees
 * @param lat2 End latitude in decimal degrees
 * @param lon2 End longitude in decimal degrees
 * @return True if route crosses land, false if water only
 */
extern "C" DECL_EXP bool PlugIn_GSHHS_CrossesLand(double lat1, double lon1,
                                                  double lat2, double lon2);
/**
 * Plays a sound file asynchronously.
 *
 * @param sound_file Path to audio file to play
 * @note Supported formats depend on sound backend
 */
extern DECL_EXP void PlugInPlaySound(wxString &sound_file);

// API 1.10 Route and Waypoint Support

/**
 * Finds built-in system waypoint icon.
 *
 * Retrieves standard waypoint icon bitmap by name from OpenCPN's icon set.
 *
 * @param icon_name Name/identifier of icon to find
 * @return Pointer to wxBitmap of icon, NULL if not found
 */
extern DECL_EXP wxBitmap *FindSystemWaypointIcon(wxString &icon_name);

/**
 * Adds a custom waypoint icon.
 *
 * Registers a new custom waypoint icon for use in OpenCPN.
 *
 * @param pimage Bitmap image for the icon
 * @param key Unique identifier for the icon
 * @param description User-friendly description
 * @return True if successfully added
 */
extern DECL_EXP bool AddCustomWaypointIcon(wxBitmap *pimage, wxString key,
                                           wxString description);

/**
 * Adds a single waypoint.
 *
 * Creates a new waypoint and adds it to OpenCPN's waypoint management system.
 *
 * @param pwaypoint Waypoint to add
 * @param b_permanent True to save persistently, false for temporary
 * @return True if successfully added
 */
extern DECL_EXP bool AddSingleWaypoint(PlugIn_Waypoint *pwaypoint,
                                       bool b_permanent = true);

/**
 * Deletes a single waypoint.
 *
 * Removes waypoint identified by GUID from OpenCPN.
 *
 * @param GUID Unique identifier of waypoint to delete
 * @return True if successfully deleted
 */
extern DECL_EXP bool DeleteSingleWaypoint(wxString &GUID);

/**
 * Updates a single waypoint.
 *
 * Modifies properties of an existing waypoint.
 *
 * @param pwaypoint Waypoint with updated properties (GUID must match existing)
 * @return True if successfully updated
 */
extern DECL_EXP bool UpdateSingleWaypoint(PlugIn_Waypoint *pwaypoint);

/**
 * Adds a new route.
 *
 * Creates a new route and adds it to OpenCPN's route management system.
 *
 * @param proute Route to add
 * @param b_permanent True to save persistently, false for temporary
 * @return True if successfully added
 */
extern DECL_EXP bool AddPlugInRoute(PlugIn_Route *proute,
                                    bool b_permanent = true);

/**
 * Deletes a route.
 *
 * Removes route identified by GUID from OpenCPN.
 *
 * @param GUID Unique identifier of route to delete
 * @return True if successfully deleted
 */
extern DECL_EXP bool DeletePlugInRoute(wxString &GUID);

/**
 * Updates an existing route.
 *
 * Updates properties and waypoints of a route already in OpenCPN's route
 * manager.
 *
 * @param proute Route with updated properties (GUID must match existing route)
 * @return True if route was successfully updated
 */
extern DECL_EXP bool UpdatePlugInRoute(PlugIn_Route *proute);

/**
 * Adds a new track.
 *
 * Creates a new track and adds it to OpenCPN's track manager.
 *
 * @param ptrack Track to add
 * @param b_permanent True to save persistently, false for temporary
 * @return True if successfully added
 */
extern DECL_EXP bool AddPlugInTrack(PlugIn_Track *ptrack,
                                    bool b_permanent = true);

/**
 * Deletes a track.
 *
 * Removes track identified by GUID from OpenCPN.
 *
 * @param GUID Unique identifier of track to delete
 * @return True if successfully deleted
 */
extern DECL_EXP bool DeletePlugInTrack(wxString &GUID);

/**
 * Updates an existing track.
 *
 * Updates properties and waypoints of a track already in OpenCPN's track
 * manager.
 *
 * @param ptrack Track with updated properties (GUID must match existing track)
 * @return True if track was successfully updated
 */
extern DECL_EXP bool UpdatePlugInTrack(PlugIn_Track *ptrack);

/* API 1.11  */

/* API 1.11  adds some more common functions to avoid unnecessary code
 * duplication */

/**
 * Gets a global system color.
 *
 * Retrieves a base color value from OpenCPN's color management system.
 *
 * @param colorName Name of the color to retrieve
 * @return wxColour containing the color value
 */
wxColour DECL_EXP GetBaseGlobalColor(wxString colorName);

/**
 * Shows a message box dialog.
 *
 * Displays a platform-native message dialog with specified text and buttons.
 *
 * @param parent Parent window
 * @param message Message text to display
 * @param caption Dialog title text
 * @param style Button/icon style flags (wxOK, wxCANCEL, etc)
 * @param x Optional X position (-1 for default)
 * @param y Optional Y position (-1 for default)
 * @return ID of button pressed
 */
int DECL_EXP OCPNMessageBox_PlugIn(wxWindow *parent, const wxString &message,
                                   const wxString &caption = _T("Message"),
                                   int style = wxOK, int x = -1, int y = -1);

/**
 * Convert decimal degrees to a formatted string.
 *
 * Converts a decimal degrees value to a string formatted in the currently
 * specified format. For example, -123.456 can be converted to "123° 27.36' W".
 *
 * @param NEflag North/East flags: 1 = N/S, 2 = E/W
 * @param a Degrees decimal in the range -180.0 to 180.0
 * @param hi_precision If true, format with 4 decimal places instead of 1
 * @return Formatted string in one of these formats depending on preferences:
 *         - DD° MM.mmm'
 *         - DD.ddddd°
 *         - DD° MM' SS.sss"
 */
extern DECL_EXP wxString toSDMM_PlugIn(int NEflag, double a,
                                       bool hi_precision = true);

/**
 * Gets private application data directory.
 *
 * Returns path to OpenCPN's private data directory for storing settings.
 *
 * @return Pointer to wxString containing data path
 * @note Do not delete the returned pointer
 * @note Path includes trailing separator
 */
extern "C" DECL_EXP wxString *GetpPrivateApplicationDataLocation();

/**
 * Gets OpenCPN executable path.
 *
 * Returns full path to OpenCPN executable file.
 *
 * @return wxString containing executable path
 */
extern DECL_EXP wxString GetOCPN_ExePath(void);

/**
 * Gets plugins directory location.
 *
 * Returns path to directory containing installed plugins.
 *
 * @return Pointer to wxString containing plugins path
 * @note Do not delete the returned pointer
 */
extern "C" DECL_EXP wxString *GetpPlugInLocation();

/**
 * Gets the installation path for a specific plugin.
 *
 * Returns the full directory path where the specified plugin is installed.
 *
 * @param pplugin Pointer to plugin instance
 * @return wxString containing plugin's installation path
 */
extern DECL_EXP wxString GetPlugInPath(opencpn_plugin *pplugin);

/**
 * Adds a chart to the database without full rebuild.
 *
 * Adds a single chart to OpenCPN's database and optionally refreshes display.
 *
 * @param full_path Full path to chart file
 * @param b_RefreshCanvas True to refresh display after adding
 * @return Database index of added chart, -1 if failed
 */
extern "C" DECL_EXP int AddChartToDBInPlace(wxString &full_path,
                                            bool b_RefreshCanvas);
/**
 * Removes a chart from database without full rebuild.
 *
 * @param full_path Full path to chart file to remove
 * @return 0 if successful, -1 if failed
 */
extern "C" DECL_EXP int RemoveChartFromDBInPlace(wxString &full_path);

/**
 * Gets system locale canonical name.
 *
 * Returns the canonical name of current system locale (e.g. "en_US").
 * Used for localization and translations.
 *
 * @return wxString containing locale canonical name
 */
extern DECL_EXP wxString GetLocaleCanonicalName();

/**
 * API 1.11 adds access to S52 Presentation library Types.
 *
 * OpenGL and vector rendering capability flags for S52 PLIB.
 *
 * These flags define various capabilities and optimizations available for
 * rendering vector charts and S52 features. They are used by plugins to
 * indicate what rendering features they support.
 */

/** Support for OpenGL Vertex Buffer Objects (VBO)
 * Enables hardware-accelerated rendering of vector lines using modern OpenGL
 * VBOs. More efficient than immediate mode rendering. */

#define PLIB_CAPS_LINE_VBO 1
/** Support for line vertex buffers
 * Enables batched line rendering using vertex buffers.
 * Optimizes performance by reducing draw calls. */
#define PLIB_CAPS_LINE_BUFFER 1 << 1

/** Support for single geometry buffers
 * Allows combining multiple geometries into a single buffer.
 * Reduces memory usage and draw calls. */
#define PLIB_CAPS_SINGLEGEO_BUFFER 1 << 2

/** Support for object segment lists
 * Enables breaking complex geometries into optimized segments.
 * Improves culling and rendering of large objects. */
#define PLIB_CAPS_OBJSEGLIST 1 << 3

/** Support for object category mutation
 * Allows dynamic changes to object display categories.
 * Required for runtime S52 display category changes. */
#define PLIB_CAPS_OBJCATMUTATE 1 << 4

class PI_S57Obj;

WX_DECLARE_LIST(PI_S57Obj, ListOfPI_S57Obj);

// ----------------------------------------------------------------------------
// PlugInChartBaseGL
//  Derived from PlugInChartBase, add OpenGL Vector chart support
// ----------------------------------------------------------------------------

/**
 * OpenGL-optimized chart base class for plugin chart types.
 *
 * Derived from PlugInChartBase, this class adds OpenGL-specific rendering
 * methods for vector charts. It's designed for charts that can benefit from
 * hardware acceleration and OpenGL features.
 *
 * Key capabilities:
 * - Direct OpenGL rendering for better performance
 * - Vector object management and queries
 * - Support for S57 chart features
 */
class DECL_EXP PlugInChartBaseGL : public PlugInChartBase {
public:
  PlugInChartBaseGL();
  virtual ~PlugInChartBaseGL();

  /**
   * Renders chart content using OpenGL.
   *
   * Primary rendering method for OpenGL-enabled chart display. Allows efficient
   * hardware-accelerated rendering of vector charts.
   *
   * @param glc OpenGL context for rendering
   * @param VPoint Current viewport parameters
   * @param Region Region to render in screen coordinates
   * @param b_use_stencil True to enable stencil buffer for clipping
   * @return Status/error code:
   *         0 = no error
   *         Other values indicate rendering errors
   */
  virtual int RenderRegionViewOnGL(const wxGLContext &glc,
                                   const PlugIn_ViewPort &VPoint,
                                   const wxRegion &Region, bool b_use_stencil);

  /**
   * Gets chart objects near specified position.
   *
   * Returns list of S57 objects within selection radius of given position.
   * Used for object queries and pick operations.
   *
   * @param lat Position latitude
   * @param lon Position longitude
   * @param select_radius Search radius in pixels
   * @param VPoint Current viewport
   * @return List of S57 objects found, NULL if none
   */
  virtual ListOfPI_S57Obj *GetObjRuleListAtLatLon(float lat, float lon,
                                                  float select_radius,
                                                  PlugIn_ViewPort *VPoint);

  /**
   * Creates description text for chart objects.
   *
   * Generates human-readable descriptions of S57 objects for tooltips
   * and info displays.
   *
   * @param obj_list List of objects to describe
   * @return Formatted description text
   */
  virtual wxString CreateObjDescriptions(ListOfPI_S57Obj *obj_list);

  /**
   * Gets number of no-coverage areas in chart.
   *
   * @return Number of no-coverage table entries (0 if none)
   */
  virtual int GetNoCOVREntries();

  /**
   * Gets number of points in no-coverage area boundary.
   *
   * @param iTable Index of no-coverage area
   * @return Number of points in boundary polygon
   */
  virtual int GetNoCOVRTablePoints(int iTable);

  /**
   * Alternative to GetNoCOVRTablePoints().
   *
   * @param iTable Index of no-coverage area
   * @return Number of points in boundary polygon
   */
  virtual int GetNoCOVRTablenPoints(int iTable);

  /**
   * Gets coordinate data for no-coverage area boundary.
   *
   * @param iTable Index of no-coverage area
   * @return Pointer to array of lat/lon points, NULL if invalid index
   */
  virtual float *GetNoCOVRTableHead(int iTable);
};

// ----------------------------------------------------------------------------
// PlugInChartBaseGLPlus2
//  Derived from PlugInChartBaseGL, add additional chart management methods
// ----------------------------------------------------------------------------

/**
 * Extended OpenGL-optimized chart base class with additional querying
 * capabilities.
 *
 * Derived from PlugInChartBaseGL, this class adds methods for querying specific
 * chart objects like lights. It's designed for vector charts that need
 * fine-grained access to chart features.
 */
class DECL_EXP PlugInChartBaseGLPlus2 : public PlugInChartBaseGL {
public:
  PlugInChartBaseGLPlus2();
  virtual ~PlugInChartBaseGLPlus2();

  /**
   * Gets list of visible light objects at specified position.
   *
   * Returns list of S57 light objects that would be visible at the given
   * geographic position. This method provides more specific querying than
   * the general GetObjRuleListAtLatLon() method.
   *
   * @param lat Latitude in decimal degrees
   * @param lon Longitude in decimal degrees
   * @param VPoint Current viewport settings
   * @return List of S57 light objects, NULL if none found
   *
   * @note Useful for showing detailed light information
   * @note Takes into account light sectors and ranges
   * @note Only returns objects visible at current display scale
   */
  virtual ListOfPI_S57Obj *GetLightsObjRuleListVisibleAtLatLon(
      float lat, float lon, PlugIn_ViewPort *VPoint);
};

// ----------------------------------------------------------------------------
// PlugInChartBaseExtended
//  Derived from PlugInChartBase, add extended chart support methods
// ----------------------------------------------------------------------------

/**
 * Extended chart base class with separated text rendering.
 *
 * Derived from PlugInChartBase, this class adds methods to separate text and
 * graphic rendering for both standard DC and OpenGL contexts. This separation
 * allows for more efficient rendering and better control over text placement.
 */
class DECL_EXP PlugInChartBaseExtended : public PlugInChartBase {
public:
  PlugInChartBaseExtended();
  virtual ~PlugInChartBaseExtended();

  /**
   * OpenGL rendering with combined text and graphics.
   *
   * Legacy full rendering method that draws both chart graphics and text
   * in a single pass using OpenGL.
   *
   * @param glc OpenGL context for rendering
   * @param VPoint Current viewport settings
   * @param Region Region to render
   * @param b_use_stencil True to enable stencil buffer
   * @return Status code (0 = success)
   */
  virtual int RenderRegionViewOnGL(const wxGLContext &glc,
                                   const PlugIn_ViewPort &VPoint,
                                   const wxRegion &Region, bool b_use_stencil);

  /**
   * Standard DC rendering without text.
   *
   * Renders only the graphical elements of the chart (no text)
   * using standard device context.
   *
   * @param VPoint Current viewport
   * @param Region Region to render
   * @return Bitmap containing rendered graphics
   */
  virtual wxBitmap &RenderRegionViewOnDCNoText(const PlugIn_ViewPort &VPoint,
                                               const wxRegion &Region);

  /**
   * Standard DC text-only rendering.
   *
   * Renders only text elements on top of previously rendered graphics.
   *
   * @param dc Memory DC to draw text into
   * @param VPoint Current viewport
   * @param Region Region to render
   * @return True if text was rendered
   */
  virtual bool RenderRegionViewOnDCTextOnly(wxMemoryDC &dc,
                                            const PlugIn_ViewPort &VPoint,
                                            const wxRegion &Region);

  /**
   * OpenGL rendering without text.
   *
   * Renders only graphical elements using OpenGL.
   *
   * @param glc OpenGL context
   * @param VPoint Current viewport
   * @param Region Region to render
   * @param b_use_stencil True to enable stencil buffer
   * @return Status code (0 = success)
   */
  virtual int RenderRegionViewOnGLNoText(const wxGLContext &glc,
                                         const PlugIn_ViewPort &VPoint,
                                         const wxRegion &Region,
                                         bool b_use_stencil);

  /**
   * OpenGL text-only rendering.
   *
   * Renders only text elements using OpenGL on top of graphics.
   *
   * @param glc OpenGL context
   * @param VPoint Current viewport
   * @param Region Region to render
   * @param b_use_stencil True to enable stencil buffer
   * @return Status code (0 = success)
   */
  virtual int RenderRegionViewOnGLTextOnly(const wxGLContext &glc,
                                           const PlugIn_ViewPort &VPoint,
                                           const wxRegion &Region,
                                           bool b_use_stencil);

  virtual ListOfPI_S57Obj *GetObjRuleListAtLatLon(float lat, float lon,
                                                  float select_radius,
                                                  PlugIn_ViewPort *VPoint);
  virtual wxString CreateObjDescriptions(ListOfPI_S57Obj *obj_list);

  virtual int GetNoCOVREntries();
  virtual int GetNoCOVRTablePoints(int iTable);
  virtual int GetNoCOVRTablenPoints(int iTable);
  virtual float *GetNoCOVRTableHead(int iTable);

  /**
   * Clears any cached text elements.
   *
   * Removes all cached text elements, forcing them to be regenerated
   * on next render.
   */
  virtual void ClearPLIBTextList();
};

// ----------------------------------------------------------------------------
// PlugInChartBaseExtendedPlus2
//  Derived from PlugInChartBaseExtended, add additional extended chart support
//  methods
// ----------------------------------------------------------------------------

class DECL_EXP PlugInChartBaseExtendedPlus2 : public PlugInChartBaseExtended {
public:
  PlugInChartBaseExtendedPlus2();
  virtual ~PlugInChartBaseExtendedPlus2();

  virtual ListOfPI_S57Obj *GetLightsObjRuleListVisibleAtLatLon(
      float lat, float lon, PlugIn_ViewPort *VPoint);
};

class wxArrayOfS57attVal;

/**
 * Name identifiers for S57 lookup table sets.
 *
 * These values represent the fifth letter in S52 lookup table names, defining
 * which drawing rules to use for different chart feature types:
 * - Points (simplified or paper chart symbols)
 * - Lines
 * - Areas (plain or symbolized boundaries)
 */
typedef enum _PI_LUPname {
  PI_SIMPLIFIED = 'L',  //!< Point objects using simplified symbols
  PI_PAPER_CHART =
      'R',         //!< Point objects using traditional paper chart symbols
  PI_LINES = 'S',  //!< Line objects (e.g., depth contours, coastlines)
  PI_PLAIN_BOUNDARIES = 'N',       //!< Area objects with simple boundaries
  PI_SYMBOLIZED_BOUNDARIES = 'O',  //!< Area objects with symbolized boundaries
  PI_LUPNAME_NUM = 5               //!< Total number of lookup table types
} PI_LUPname;

/**
 * Display categories for S52 chart features.
 *
 * These categories control the visibility of chart features according to
 * IHO S-52 presentation library specifications. Features can be assigned
 * to different categories to allow selective display based on importance
 * and use case.
 *
 * The categories form a hierarchy:
 * - Display Base: Critical navigation features that must always be visible
 * - Standard: Additional features needed for basic safe navigation
 * - Other: Extra features for detailed navigation and reference
 * - Mariner's Standard: User-selected standard features
 * - Mariner's Other: Additional user-selected features
 */
typedef enum _PI_DisCat {
  PI_DISPLAYBASE = 'D',  //!< Display Base - features that must always be shown
  PI_STANDARD = 'S',     //!< Standard - default features for safe navigation
  PI_OTHER = 'O',  //!< Other - additional features for detailed navigation
  PI_MARINERS_STANDARD = 'M',  //!< Mariner specified standard features
  PI_MARINERS_OTHER,           //!< Additional mariner specified features
  PI_DISP_CAT_NUM,             //!< Number of categories (for array sizing)
} PI_DisCat;

/**
 * Display priority levels for S52 chart objects.
 *
 * These values define the drawing order for different types of chart features
 * according to IHO S-52 presentation library specifications. Higher priority
 * objects are drawn on top of lower priority objects.
 *
 * The priorities form a hierarchy from base fill patterns through to user
 * navigation objects:
 * - Basic area fills (priority 0)
 * - Land/sea areas (priorities 1-3)
 * - Fixed/floating aids (priorities 4-6)
 * - Routes and navigation features (priorities 7-8)
 * - User data and vessel symbols (priority 9)
 */
typedef enum _PI_DisPrio {
  PI_PRIO_NODATA = '0',      //!< No data fill area pattern (lowest priority)
  PI_PRIO_GROUP1 = '1',      //!< S57 group 1 filled areas (land, etc)
  PI_PRIO_AREA_1 = '2',      //!< Superimposed areas
  PI_PRIO_AREA_2 = '3',      //!< Superimposed areas including water features
  PI_PRIO_SYMB_POINT = '4',  //!< Point symbols including land features
  PI_PRIO_SYMB_LINE = '5',   //!< Line symbols including restricted areas
  PI_PRIO_SYMB_AREA = '6',   //!< Area symbols including traffic areas
  PI_PRIO_ROUTEING = '7',    //!< Routeing lines (TSS, recommended tracks)
  PI_PRIO_HAZARDS = '8',     //!< Hazards, buoys, beacons
  PI_PRIO_MARINERS = '9',    //!< VRM, EBL, own ship (highest priority)
  PI_PRIO_NUM = 10           //!< Number of priority levels
} PI_DisPrio;

/**
 * Return values for plugin initialization.
 *
 * These values are returned by opencpn_plugin::Init() to indicate the result
 * of plugin initialization and guide OpenCPN's handling of initialization
 * failures.
 */
typedef enum PI_InitReturn {
  PI_INIT_OK = 0,       //!< Initialization successful
  PI_INIT_FAIL_RETRY,   //!< Init failed, retry suggested
  PI_INIT_FAIL_REMOVE,  //!< Init failed, suggest remove from further use
  PI_INIT_FAIL_NOERROR  //!< Init failed, request no explicit error message
} _PI_InitReturn;

/**
 * Represents a line segment element for efficient vector chart rendering.
 *
 * This class stores geometry and rendering information for line segments in
 * vector charts. It's designed for optimal OpenGL rendering using vertex buffer
 * objects and spatial indexing.
 */
class PI_line_segment_element {
public:
  size_t vbo_offset;  //!< Offset into vertex buffer object for this segment
  size_t n_points;    //!< Number of points in this line segment
  int priority;       //!< Drawing priority (higher draws on top)
  float lat_max;      //!< Maximum latitude of segment's bounding box
  float lat_min;      //!< Minimum latitude of segment's bounding box
  float lon_max;      //!< Maximum longitude of segment's bounding box
  float lon_min;      //!< Minimum longitude of segment's bounding box
  int type;           //!< Line type identifier
  void *private0;     //!< Private data pointer for implementation use

  /**
   * Pointer to next segment in linked list.
   * Used for maintaining collections of segments.
   */
  PI_line_segment_element *next;
};

/**
 * Class representing an S-57 chart object.
 *
 * This class encapsulates a single feature from an S-57 electronic chart,
 * storing its geometry, attributes, and rendering properties. Used for both
 * simple objects like buoys and complex ones like depth areas.
 */
class DECL_EXP PI_S57Obj {
public:
  //  Public Methods
  PI_S57Obj();

public:
  // Instance Data
  char FeatureName[8];  //!< S-57 feature type code (e.g., "DEPARE")
  int Primitive_type;   //!< Geometry type (point, line, area)

  char *att_array;             //!< Array of attribute types
  wxArrayOfS57attVal *attVal;  //!< Array of attribute values
  int n_attr;                  //!< Number of attributes

  int iOBJL;  //!< Object lookup index
  int Index;  //!< Runtime object index

  double x;            //!< X coordinate for point features
  double y;            //!< Y coordinate for point features
  double z;            //!< Z coordinate (usually depth)
  int npt;             //!< Number of points in geometry
  void *geoPt;         //!< Geometry data for lines/areas
  double *geoPtz;      //!< Z values for multipoint features
  double *geoPtMulti;  //!< Lat/lon pairs for decomposed points

  void *pPolyTessGeo;  //!< Tesselated polygon geometry

  double m_lat;  //!< Reference latitude
  double m_lon;  //!< Reference longitude

  double chart_ref_lat;  //!< Chart reference latitude
  double chart_ref_lon;  //!< Chart reference longitude

  double lat_min;  //!< Bounding box minimum latitude
  double lat_max;  //!< Bounding box maximum latitude
  double lon_min;  //!< Bounding box minimum longitude
  double lon_max;  //!< Bounding box maximum longitude

  int Scamin;  //!< Minimum display scale

  bool bIsClone;  //!< True if object is a clone
  int nRef;       //!< Reference count for memory management

  bool bIsAton;        //!< True if object is an aid to navigation
  bool bIsAssociable;  //!< True if object can have relationships

  int m_n_lsindex;          //!< Number of line segments
  int *m_lsindex_array;     //!< Array of line segment indices
  int m_n_edge_max_points;  //!< Maximum points per edge
  void *m_chart_context;    //!< Parent chart context

  PI_DisCat m_DisplayCat;  //!< Display category (base, standard, other)

  void *S52_Context;  //!< S52 presentation data
  PI_S57Obj *child;   //!< Child object list (for multipoint soundings)
  PI_S57Obj *next;    //!< Linked list pointer

  // Coordinate transform coefficients
  double x_rate;    //!< X scale factor
  double y_rate;    //!< Y scale factor
  double x_origin;  //!< X origin offset
  double y_origin;  //!< Y origin offset

  // OpenGL rendering parameters
  int auxParm0;  //!< Auxiliary parameter 0
  int auxParm1;  //!< Auxiliary parameter 1
  int auxParm2;  //!< Auxiliary parameter 2
  int auxParm3;  //!< Auxiliary parameter 3

  PI_line_segment_element *m_ls_list;  //!< Line segment list
  bool m_bcategory_mutable;            //!< True if category can change
  int m_DPRI;                          //!< Display priority
};

/**
 * Gets current color scheme used by S52 PLIB.
 *
 * @return Color scheme name ("DAY", "DUSK", or "NIGHT")
 */
wxString DECL_EXP PI_GetPLIBColorScheme();
/**
 * Gets configured depth unit for S52 display.
 *
 * @return Integer depth unit code:
 *         0 = Meters
 *         1 = Feet
 *         2 = Fathoms
 */
int DECL_EXP PI_GetPLIBDepthUnitInt();

/**
 * Gets configured S52 symbol style.
 *
 * @return Integer symbol style:
 *         0 = Simplified symbols
 *         1 = Traditional paper chart symbols
 */
int DECL_EXP PI_GetPLIBSymbolStyle();

/**
 * Gets configured S52 boundary style.
 *
 * @return Integer boundary style:
 *         0 = Plain boundaries
 *         1 = Symbolized boundaries
 */
int DECL_EXP PI_GetPLIBBoundaryStyle();

/**
 * Gets hash value representing current PLIB state.
 * Used to detect configuration changes.
 *
 * @return Integer hash of PLIB settings
 */
int DECL_EXP PI_GetPLIBStateHash();

/**
 * Gets configured safety contour depth.
 *
 * @return Safety contour depth in configured units
 */
double DECL_EXP PI_GetPLIBMarinerSafetyContour();

/**
 * Gets geographic bounding box of S57 object.
 *
 * @param pObj S57 object to get bounds for
 * @param lat_min Min latitude output
 * @param lat_max Max latitude output
 * @param lon_min Min longitude output
 * @param lon_max Max longitude output
 * @return True if bounds were calculated
 */
bool DECL_EXP PI_GetObjectRenderBox(PI_S57Obj *pObj, double *lat_min,
                                    double *lat_max, double *lon_min,
                                    double *lon_max);

/**
 * Updates rendering context for S57 object.
 *
 * @param pObj Object to update context for
 */
void DECL_EXP PI_UpdateContext(PI_S57Obj *pObj);

/**
 * Checks if object should be rendered.
 *
 * @param pObj Object to check
 * @param vp Current viewport
 * @return True if object should be rendered
 */
bool DECL_EXP PI_PLIBObjectRenderCheck(PI_S57Obj *pObj, PlugIn_ViewPort *vp);

/**
 * Gets Look-Up Table (LUP) name for object.
 *
 * The LUP in the S-52 standard defines how chart features should be symbolized
 * and displayed.
 *
 * @see PI_LUPname
 *
 * @param pObj Object to get LUP name for
 * @return LUP name enum value
 */
PI_LUPname DECL_EXP PI_GetObjectLUPName(PI_S57Obj *pObj);

/**
 * Gets display priority for object.
 *
 * @param pObj Object to get priority for
 * @return Display priority enum value
 */
PI_DisPrio DECL_EXP PI_GetObjectDisplayPriority(PI_S57Obj *pObj);

/**
 * Gets display category for object.
 *
 * @param pObj Object to get category for
 * @return Display category enum value
 */
PI_DisCat DECL_EXP PI_GetObjectDisplayCategory(PI_S57Obj *pObj);

/**
 * Sets rendering priority for line feature.
 *
 * @param pObj Line object to set priority for
 * @param prio New priority value
 */
void DECL_EXP PI_PLIBSetLineFeaturePriority(PI_S57Obj *pObj, int prio);

/**
 * Prepares PLIB for new rendering pass.
 * Clears internal caches and states.
 */
void DECL_EXP PI_PLIBPrepareForNewRender(void);

/**
 * Frees S52 PLIB context.
 *
 * @param pContext Context pointer to free
 */
void DECL_EXP PI_PLIBFreeContext(void *pContext);

/**
 * Sets rendering capability flags.
 *
 * @param flags Bitwise combination of PLIB_CAPS flags:
 *        PLIB_CAPS_LINE_VBO
 *        PLIB_CAPS_LINE_BUFFER
 *        PLIB_CAPS_SINGLEGEO_BUFFER
 *        etc.
 */
void DECL_EXP PI_PLIBSetRenderCaps(unsigned int flags);

/**
 * Sets S52 PLIB rendering context for an object.
 *
 * Prepares the S52 presentation library context for rendering a chart object.
 * Must be called before rendering to ensure proper symbolization.
 *
 * @param pObj S57 object to set context for
 * @return True if context was set successfully
 */
bool DECL_EXP PI_PLIBSetContext(PI_S57Obj *pObj);

/**
 * Renders an S57 object using standard device context.
 *
 * Draws a chart object using S52 presentation library rules through a
 * standard wxDC device context.
 *
 * @param pdc Device context to draw into
 * @param pObj S57 object to render
 * @param vp Current viewport parameters
 * @return Status code (0 = success)
 *
 * @note For non-OpenGL rendering
 * @note Context must be set with PLIBSetContext() first
 */
int DECL_EXP PI_PLIBRenderObjectToDC(wxDC *pdc, PI_S57Obj *pObj,
                                     PlugIn_ViewPort *vp);

/**
 * Renders an S57 area object using standard device context.
 *
 * Specialized rendering for area features like depth areas, land areas etc.
 * Supports pixel buffer output for optimized drawing.
 *
 * @param pdc Device context to draw into
 * @param pObj S57 area object to render
 * @param vp Current viewport parameters
 * @param rect Rectangle defining render area
 * @param pixbuf Optional pixel buffer for cached rendering
 * @return Status code (0 = success)
 *
 * @note For area patterns and fills
 * @note Context must be set with PLIBSetContext() first
 */
int DECL_EXP PI_PLIBRenderAreaToDC(wxDC *pdc, PI_S57Obj *pObj,
                                   PlugIn_ViewPort *vp, wxRect rect,
                                   unsigned char *pixbuf);

/**
 * Renders an S57 area object using OpenGL.
 *
 * OpenGL-optimized rendering for area features with hardware acceleration.
 *
 * @param glcc OpenGL context for rendering
 * @param pObj S57 area object to render
 * @param vp Current viewport parameters
 * @param render_rect Rectangle defining render bounds
 * @return Status code (0 = success)
 *
 * @note For hardware-accelerated area rendering
 * @note Context must be set with PLIBSetContext() first
 */
int DECL_EXP PI_PLIBRenderAreaToGL(const wxGLContext &glcc, PI_S57Obj *pObj,
                                   PlugIn_ViewPort *vp, wxRect &render_rect);

/**
 * Renders any S57 object using OpenGL.
 *
 * Generic OpenGL rendering for any chart object type with hardware
 * acceleration.
 *
 * @param glcc OpenGL context for rendering
 * @param pObj S57 object to render
 * @param vp Current viewport parameters
 * @param render_rect Rectangle defining render bounds
 * @return Status code (0 = success)
 *
 * @note For hardware-accelerated rendering
 * @note Context must be set with PLIBSetContext() first
 * @note Handles points, lines, areas and text
 */
int DECL_EXP PI_PLIBRenderObjectToGL(const wxGLContext &glcc, PI_S57Obj *pObj,
                                     PlugIn_ViewPort *vp, wxRect &render_rect);

/* API 1.11 OpenGL Display List and vertex buffer object routines

   Effectively these two routines cancel each other so all
   of the translation, scaling and rotation can be done by opengl.

   Display lists need only be built infrequently, but used in each frame
   greatly accelerates the speed of rendering.  This avoids costly calculations,
   and also allows the vertexes to be stored in graphics memory.

   static int dl = 0;
   glPushMatrix();
   PlugInMultMatrixViewport(current_viewport);
   if(dl)
      glCallList(dl);
   else {
      dl = glGenLists(1);
      PlugInViewPort norm_viewport = current_viewport;
      NormalizeViewPort(norm_viewport);
      glNewList(dl, GL_COMPILE_AND_EXECUTE);
      ... // use norm_viewport with GetCanvasLLPix here
      glEndList();
   }
   glPopMatrix();
   ... // use current_viewport with GetCanvasLLPix again
*/

/**
 * Checks if viewport has been normalized.
 *
 * Tests whether the viewport parameters have been adjusted to a normalized form
 * suitable for OpenGL rendering.
 *
 * @param vp Viewport to check
 * @return True if viewport is normalized, false if not
 */
extern DECL_EXP bool PlugInHasNormalizedViewPort(PlugIn_ViewPort *vp);

/**
 * Applies viewport transformation matrix.
 *
 * Applies rotation, scaling and translation to viewport based on reference
 * point. Used with OpenGL display lists for efficient rendering.
 *
 * @param vp Viewport to transform
 * @param lat Optional reference latitude (default 0)
 * @param lon Optional reference longitude (default 0)
 *
 * @note Pairs with PlugInNormalizeViewport()
 * @note Used for OpenGL display list optimization
 * @note Reference point affects translation component
 */
extern DECL_EXP void PlugInMultMatrixViewport(PlugIn_ViewPort *vp,
                                              float lat = 0, float lon = 0);

/**
 * Normalizes viewport parameters.
 *
 * Adjusts viewport parameters to a normalized form for consistent OpenGL
 * rendering. The normalized form simplifies transformations and coordinates.
 *
 * @param vp Viewport to normalize
 * @param lat Optional reference latitude (default 0)
 * @param lon Optional reference longitude (default 0)
 *
 * @note Pairs with PlugInMultMatrixViewport()
 * @note Reference point affects normalization origin
 * @note Required for proper OpenGL display list usage
 */
extern DECL_EXP void PlugInNormalizeViewport(PlugIn_ViewPort *vp, float lat = 0,
                                             float lon = 0);

class wxPoint2DDouble;

/**
 * Converts lat/lon to canvas pixels with double precision.
 *
 * High-precision version of GetCanvasPixLL() for accurate positioning.
 * Transforms geographic coordinates to canvas pixel coordinates.
 *
 * @param vp Current viewport settings
 * @param pp Will receive pixel coordinates as doubles
 * @param lat Latitude in decimal degrees
 * @param lon Longitude in decimal degrees
 */
extern "C" DECL_EXP void GetDoubleCanvasPixLL(PlugIn_ViewPort *vp,
                                              wxPoint2DDouble *pp, double lat,
                                              double lon);

/* API 1.13  */
/* API 1.13  adds some more common functions to avoid unnecessary code
 * duplication */

/**
 * Converts degrees/decimal minutes string to decimal degrees.
 *
 * Parses position strings in DDMMss format like "4930.000N".
 *
 * @param sdms Position string in degrees/decimal minutes format
 * @return Position in decimal degrees, or 0.0 if invalid format
 */
extern DECL_EXP double fromDMM_Plugin(wxString sdms);

/**
 * Sets chart display rotation angle.
 *
 * Rotates the chart display by the specified angle.
 *
 * @param rotation Rotation angle in degrees [0-360)
 *                 0 = North up
 *                 90 = East up
 *                 etc.
 */
extern DECL_EXP void SetCanvasRotation(double rotation);

/**
 * Sets chart projection type.
 *
 * Changes the projection used for chart display.
 *
 * @param projection Projection type from PI_ProjectionType enum:
 *                   - PI_PROJECTION_MERCATOR
 *                   - PI_PROJECTION_TRANSVERSE_MERCATOR
 *                   - PI_PROJECTION_POLYCONIC
 *                   etc.
 */
extern DECL_EXP void SetCanvasProjection(int projection);

/**
 * Gets waypoint data by GUID.
 *
 * Retrieves details for a specific waypoint.
 *
 * @param GUID Unique identifier of waypoint to get
 * @param pwaypoint Pointer to receive waypoint data
 * @return True if waypoint was found and data copied
 */
extern DECL_EXP bool GetSingleWaypoint(wxString GUID,
                                       PlugIn_Waypoint *pwaypoint);

/**
 * Checks if chart should pan when cursor near edge.
 *
 * Tests if automatic chart panning should occur based on cursor position.
 *
 * @param x Cursor X coordinate
 * @param y Cursor Y coordinate
 * @param dragging True if mouse is being dragged
 * @param margin Edge margin width in pixels
 * @param delta Pan amount in pixels
 * @return True if panning should occur
 */
extern DECL_EXP bool CheckEdgePan_PlugIn(int x, int y, bool dragging,
                                         int margin, int delta);

/**
 * Gets icon bitmap by name.
 *
 * Retrieves standard OpenCPN toolbar icon bitmap by name.
 *
 * @param name Name/identifier of icon to get
 * @return Bitmap containing the icon
 */
extern DECL_EXP wxBitmap GetIcon_PlugIn(const wxString &name);

/**
 * Sets mouse cursor.
 *
 * Changes the mouse cursor displayed over chart window.
 *
 * @param pPlugin_Cursor Pointer to cursor to use, NULL for default
 */
extern DECL_EXP void SetCursor_PlugIn(wxCursor *pPlugin_Cursor = NULL);

/**
 * Retrieves a platform-normalized font scaled for consistent physical size.
 *
 * Provides a font that maintains perceptually consistent size across different
 * platforms, screen densities, and display characteristics. The scaling ensures
 * that a specified font size appears similar in physical dimensions regardless
 * of:
 * - Screen DPI
 * - Operating system
 * - Display resolution
 * - Physical screen size
 *
 * @param TextElement Identifies the UI context (e.g., "AISTargetAlert",
 * "StatusBar")
 * @param default_size Base font size in points. When 0, uses system default.
 *                     When non-zero (e.g., 12), applies cross-platform scaling
 *                     to maintain consistent physical font size.
 *
 * @return Pointer to a wxFont with platform-normalized scaling
 *
 * @note Scaling mechanism:
 *       - Adjusts point size based on system DPI
 *       - Applies platform-specific scaling factors
 *       - Ensures readable text across diverse display environments
 *
 * @note Returned font is managed by OpenCPN's font cache
 * @note Pointer should not be deleted by the caller
 *
 * @example
 * // A 12-point font will look similar on:
 * // - Windows laptop
 * // - MacBook Retina display
 * // - Android tablet
 * wxFont* font = GetOCPNScaledFont_PlugIn("StatusBar", 12);
 */
extern DECL_EXP wxFont *GetOCPNScaledFont_PlugIn(wxString TextElement,
                                                 int default_size = 0);

/**
 * Gets a uniquely scaled font copy for responsive UI elements.
 *
 * Like GetOCPNScaledFont_PlugIn() but scales font size more aggressively based
 * on OpenCPN's responsive/touchscreen mode settings. Used by GUI tools and
 * windows that need larger fonts for touch usability. Always ensures minimum
 * 3mm physical size regardless of configured point size.
 *
 * @param item UI element name to get font for
 * @return Scaled wxFont object
 * @see OCPNGetFont() for supported TextElement values
 * @see GetOCPNScaledFont_PlugIn()
 */
extern DECL_EXP wxFont GetOCPNGUIScaledFont_PlugIn(wxString item);

/**
 * Gets GUI scaling factor for a specific scaling level.
 *
 * Returns the display scaling factor to use for GUI elements at a given
 * scaling level. Used to scale toolbar icons and other UI elements.
 *
 * @param GUIScaledFactor Base scaling factor level (typically 0-5)
 * @return Scaling multiplier to apply to base sizes:
 *         - 1.0 = Normal/100% scaling
 *         - 2.0 = Double size/200% scaling
 *         - etc.
 *
 * @note Used for consistent UI scaling across platforms
 * @note Takes system DPI into account
 */
extern DECL_EXP double GetOCPNGUIToolScaleFactor_PlugIn(int GUIScaledFactor);

/**
 * Gets current global GUI scaling factor.
 *
 * Returns the display scaling factor currently in use for the OpenCPN GUI.
 * Used to scale plugin UI elements to match core application.
 *
 * @return Current GUI scaling multiplier:
 *         - 1.0 = Normal/100% scaling
 *         - 2.0 = Double size/200% scaling
 *         - etc.
 *
 * @note Reflects user's selected GUI scale setting
 * @note Takes system DPI into account
 */
extern DECL_EXP double GetOCPNGUIToolScaleFactor_PlugIn();

/**
 * Gets chart rendering scale factor.
 *
 * Returns the scaling factor used for chart display. This may differ from
 * the GUI scaling factor to optimize chart rendering and readability.
 *
 * @return Chart display scaling multiplier:
 *         - 1.0 = Native chart resolution
 *         - 2.0 = Double resolution
 *         - etc.
 *
 * @note Used for high DPI displays
 * @note Affects chart text, symbols and features
 */
extern DECL_EXP float GetOCPNChartScaleFactor_Plugin();

/**
 * Gets color configured for a UI text element.
 *
 * @param TextElement UI element ID like "AISTargetAlert"
 * @return Color configured for element, defaults to system window text color
 * @see OCPNGetFont() for supported TextElement values
 */
extern DECL_EXP wxColour GetFontColour_PlugIn(wxString TextElement);

/**
 * Gets current canvas tilt angle.
 *
 * Returns the tilt angle of the chart display canvas in degrees.
 * Tilt provides a 3D perspective view of the chart.
 *
 * @return Tilt angle in degrees (0-90)
 *         0 = Normal top-down view
 *         90 = Horizontal view
 */
extern DECL_EXP double GetCanvasTilt();

/**
 * Gets current canvas tilt angle.
 *
 * Returns the tilt angle of the chart display canvas in degrees.
 * Tilt provides a 3D perspective view of the chart.
 *
 * @return Tilt angle in degrees (0-90)
 *         0 = Normal top-down view
 *         90 = Horizontal view
 */
extern DECL_EXP void SetCanvasTilt(double tilt);

/**
 * Start playing a sound file asynchronously. Supported formats depends
 * on sound backend. The deviceIx is only used on platforms using the
 * portaudio sound backend where -1 indicates the default device.
 */
extern DECL_EXP bool PlugInPlaySoundEx(wxString &sound_file,
                                       int deviceIndex = -1);

/**
 * Adds a chart directory to OpenCPN's chart database.
 *
 * Registers a new directory containing charts for OpenCPN to scan
 * and include in its database.
 *
 * @param path Full path to directory containing charts
 *
 * @note Directory is not scanned immediately
 * @note Call ForceChartDBUpdate() to scan new directory
 * @note Path should use platform-appropriate separators
 */
extern DECL_EXP void AddChartDirectory(wxString &path);

/**
 * Forces an update of the chart database.
 *
 * Triggers OpenCPN to scan configured chart directories for new or
 * removed charts and update the database accordingly.
 *
 * @note More efficient than full rebuild
 * @note Only detects changes since last update
 * @note May take several seconds for large chart sets
 */
extern DECL_EXP void ForceChartDBUpdate();

/**
 * Forces complete rebuild of chart database.
 *
 * Triggers OpenCPN to do a full rebuild of its chart database by
 * rescanning all configured chart directories from scratch.
 *
 * @note More thorough but slower than ForceChartDBUpdate()
 * @note Removes all existing chart entries first
 * @note Required after major chart organization changes
 * @note May take several minutes for large chart sets
 */
extern DECL_EXP void ForceChartDBRebuild();

/**
 * Returns the platform-specific default documents directory.
 *
 * @return Path to writeable documents directory
 */
extern DECL_EXP wxString GetWritableDocumentsDir(void);

/**
 * Gets pointer to active options dialog.
 *
 * @return Pointer to active wxDialog, NULL if no dialog shown
 */
extern DECL_EXP wxDialog *GetActiveOptionsDialog();

/**
 * Gets array of all waypoint/marks GUIDs.
 *
 * Can be used in conjunction with GetSingleWaypoint() or GetSingleWaypointEx to
 * retrieve waypoint/mark details by GUID.
 *
 * @return Array of waypoint/mark unique identifiers
 * @see GetSingleWaypoint()
 * @see GetSingleWaypointEx()
 */
extern DECL_EXP wxArrayString GetWaypointGUIDArray(void);

/**
 * Gets array of available waypoint icons.
 *
 * @return Array of icon names that can be used for waypoints
 */
extern DECL_EXP wxArrayString GetIconNameArray(void);

/**
 * Registers a new font configuration element.
 *
 * Allows plugins to define custom UI elements needing font configuration,
 * beyond the standard elements defined in OCPNGetFont().
 *
 * @param TextElement New UI element identifier to register
 * @return True if element was registered, false if already exists
 * @see OCPNGetFont()
 */
extern DECL_EXP bool AddPersistentFontKey(wxString TextElement);

/**
 * Gets name of currently active style sheet.
 *
 * Returns the name of the currently selected S52 presentation library style.
 * Style sheets control the overall appearance of vector chart features.
 *
 * @return Name of active style sheet
 *
 * @note Style sheets are user-selectable
 * @note Affects S52 vector chart display only
 */
extern DECL_EXP wxString GetActiveStyleName();

/**
 * Creates bitmap from SVG file.
 *
 * Renders an SVG file to a bitmap at the specified dimensions. Useful for
 * creating resolution-independent toolbar icons and other graphics.
 *
 * @param filename Full path to SVG file
 * @param width Desired bitmap width in pixels
 * @param height Desired bitmap height in pixels
 * @return wxBitmap containing rendered SVG, invalid bitmap if error
 *
 * @note SVG is scaled to fit requested dimensions
 * @note Aspect ratio is preserved unless width/height force different ratio
 * @note Returns invalid bitmap if file not found or invalid SVG
 */
extern DECL_EXP wxBitmap GetBitmapFromSVGFile(wxString filename,
                                              unsigned int width,
                                              unsigned int height);

/**
 * Checks if touch interface mode is enabled.
 *
 * Returns whether OpenCPN is currently operating in touch-optimized mode.
 * Touch mode affects UI element sizes and interaction behaviors.
 *
 * @return True if touch mode enabled, false if standard mouse/keyboard mode
 *
 * @note Touch mode can be toggled by user preference
 * @note Affects toolbar, dialog and control sizes
 * @note May affect gesture recognition
 */
extern DECL_EXP bool IsTouchInterface_PlugIn(void);

/*  Platform optimized File/Dir selector dialogs */
/**
 * Shows platform-optimized directory selector dialog.
 *
 * Displays a platform-native directory selection dialog that is optimized for
 * each operating system's conventions and capabilities.
 *
 * @param parent Parent window for the dialog
 * @param file_spec Will receive selected directory path
 * @param Title Dialog title text
 * @param initDir Initial directory to show
 * @return Dialog result:
 *         - wxID_OK if directory selected
 *         - wxID_CANCEL if dialog cancelled
 *
 * @note Uses native file pickers on Windows/Mac
 * @note Falls back to wxWidgets dialog on other platforms
 * @note Selected path uses platform-appropriate separators
 */
extern DECL_EXP int PlatformDirSelectorDialog(wxWindow *parent,
                                              wxString *file_spec,
                                              wxString Title, wxString initDir);

/**
 * Shows platform-optimized file selector dialog.
 *
 * Displays a platform-native file selection dialog that is optimized for
 * each operating system's conventions and capabilities.
 *
 * @param parent Parent window for the dialog
 * @param file_spec Will receive selected file path
 * @param Title Dialog title text
 * @param initDir Initial directory to show
 * @param suggestedName Suggested filename
 * @param wildcard File type filter (e.g. "*.txt")
 * @return Dialog result:
 *         - wxID_OK if file selected
 *         - wxID_CANCEL if dialog cancelled
 *
 * @note Uses native file pickers on Windows/Mac
 * @note Falls back to wxWidgets dialog on other platforms
 * @note Selected path uses platform-appropriate separators
 * @note Wildcard format varies by platform
 */
extern DECL_EXP int PlatformFileSelectorDialog(wxWindow *parent,
                                               wxString *file_spec,
                                               wxString Title, wxString initDir,
                                               wxString suggestedName,
                                               wxString wildcard);

/*  OpenCPN HTTP File Download PlugIn Interface   */

/*   Various method Return Codes, etc          */
/**
 * Status codes for HTTP file download operations.
 *
 * These values indicate the final result or current state of file download
 * operations initiated through the plugin download API.
 */
typedef enum _OCPN_DLStatus {
  OCPN_DL_UNKNOWN = -1,      //!< Unknown or uninitialized status
  OCPN_DL_NO_ERROR = 0,      //!< Download completed successfully
  OCPN_DL_FAILED = 1,        //!< Download failed (general error)
  OCPN_DL_ABORTED = 2,       //!< Download was cancelled by user
  OCPN_DL_USER_TIMEOUT = 4,  //!< Download timed out waiting for user action
  OCPN_DL_STARTED = 8        //!< Download has begun but not yet complete
} OCPN_DLStatus;

/**
 * Event types for HTTP file download operations.
 *
 * These values identify different stages of the download process when receiving
 * download progress events.
 */
typedef enum _OCPN_DLCondition {
  OCPN_DL_EVENT_TYPE_UNKNOWN = -1,   //!< Unknown event type
  OCPN_DL_EVENT_TYPE_START = 80,     //!< Download is starting
  OCPN_DL_EVENT_TYPE_PROGRESS = 81,  //!< Download progress update
  OCPN_DL_EVENT_TYPE_END = 82        //!< Download has completed
} OCPN_DLCondition;

//      Style definitions for Synchronous file download modal dialogs, if
//      desired. Abstracted from wxCURL package
enum OCPN_DLDialogStyle {
  OCPN_DLDS_ELAPSED_TIME = 0x0001,  //!< The dialog shows the elapsed time.
  OCPN_DLDS_ESTIMATED_TIME =
      0x0002,  //!< The dialog shows the estimated total time.
  OCPN_DLDS_REMAINING_TIME = 0x0004,  //!< The dialog shows the remaining time.
  OCPN_DLDS_SPEED = 0x0008,           //!< The dialog shows the transfer speed.
  OCPN_DLDS_SIZE = 0x0010,  //!< The dialog shows the size of the resource to
                            //!< download/upload.
  OCPN_DLDS_URL =
      0x0020,  //!< The dialog shows the URL involved in the transfer.

  // styles related to the use of wxCurlConnectionSettingsDialog:

  OCPN_DLDS_CONN_SETTINGS_AUTH =
      0x0040,  //!< The dialog allows the user to change the authentication
               //!< settings.
  OCPN_DLDS_CONN_SETTINGS_PORT = 0x0080,  //!< The dialog allows the user to
                                          //!< change the port for the transfer.
  OCPN_DLDS_CONN_SETTINGS_PROXY =
      0x0100,  //!< The dialog allows the user to change the proxy settings.

  OCPN_DLDS_CONN_SETTINGS_ALL = OCPN_DLDS_CONN_SETTINGS_AUTH |
                                OCPN_DLDS_CONN_SETTINGS_PORT |
                                OCPN_DLDS_CONN_SETTINGS_PROXY,

  OCPN_DLDS_SHOW_ALL = OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_ESTIMATED_TIME |
                       OCPN_DLDS_REMAINING_TIME | OCPN_DLDS_SPEED |
                       OCPN_DLDS_SIZE | OCPN_DLDS_URL |
                       OCPN_DLDS_CONN_SETTINGS_ALL,

  OCPN_DLDS_CAN_ABORT = 0x0200,  //!< The transfer can be aborted by the user.
  OCPN_DLDS_CAN_START = 0x0400,  //!< The transfer won't start automatically.
                                 //!< The user needs to start it.
  OCPN_DLDS_CAN_PAUSE = 0x0800,  //!< The transfer can be paused.

  OCPN_DLDS_AUTO_CLOSE =
      0x1000,  //!< The dialog auto closes when transfer is complete.

  // by default all available features are enabled:
  OCPN_DLDS_DEFAULT_STYLE = OCPN_DLDS_CAN_START | OCPN_DLDS_CAN_PAUSE |
                            OCPN_DLDS_CAN_ABORT | OCPN_DLDS_SHOW_ALL |
                            OCPN_DLDS_AUTO_CLOSE
};

#define ONLINE_CHECK_RETRY \
  30  // Recheck the Internet connection availability every ONLINE_CHECK_RETRY s

/**
 * Synchronously download a file with progress dialog.
 *
 * Download a file from a URL while showing a progress dialog.
 * Blocks until the download completes or fails.
 *
 * @param url URL to download from
 * @param outputFile Local file path to save to
 * @param title Progress dialog title
 * @param message Progress dialog message
 * @param bitmap Optional bitmap to show in dialog
 * @param parent Parent window for dialog
 * @param style Dialog style flags (see OCPN_DLDialogStyle)
 * @param timeout_secs Timeout in seconds (0 for no timeout)
 * @return Download status:
 *         - OCPN_DL_NO_ERROR: Success
 *         - OCPN_DL_FAILED: Download failed
 *         - OCPN_DL_ABORTED: User cancelled
 *         - OCPN_DL_USER_TIMEOUT: Timeout expired
 */
extern DECL_EXP _OCPN_DLStatus OCPN_downloadFile(
    const wxString &url, const wxString &outputFile, const wxString &title,
    const wxString &message, const wxBitmap &bitmap, wxWindow *parent,
    long style, int timeout_secs);

/**
 * Asynchronously downloads a file in the background.
 *
 * Initiates a background download that does not block. Progress and completion
 * are reported via events sent to the provided handler.
 *
 * @param url URL to download from
 * @param outputFile Local file path to save to
 * @param handler Event handler to receive download events
 * @param handle Will receive handle to track download
 * @return Initial status:
 *         - OCPN_DL_STARTED: Download started
 *         - OCPN_DL_FAILED: Failed to start download
 *
 * @note Handler will receive OCPN_downloadEvent events
 * @note Use handle with OCPN_cancelDownloadFileBackground()
 */
extern DECL_EXP _OCPN_DLStatus
OCPN_downloadFileBackground(const wxString &url, const wxString &outputFile,
                            wxEvtHandler *handler, long *handle);

/**
 * Cancels a background download.
 *
 * @param handle Download handle from OCPN_downloadFileBackground()
 */
extern DECL_EXP void OCPN_cancelDownloadFileBackground(long handle);

/**
 * Performs HTTP POST request.
 *
 * Makes a blocking HTTP POST request and returns the response.
 *
 * @param url URL to post to
 * @param parameters POST parameters/data
 * @param result Will receive response body
 * @param timeout_secs Timeout in seconds
 * @return Status code (see OCPN_DLStatus)
 */
extern DECL_EXP _OCPN_DLStatus OCPN_postDataHttp(const wxString &url,
                                                 const wxString &parameters,
                                                 wxString &result,
                                                 int timeout_secs);

/**
 * Checks internet connectivity.
 *
 * @return True if internet appears to be available
 */
extern DECL_EXP bool OCPN_isOnline();

/*  Supporting  Event for Background downloading          */
/*  OCPN_downloadEvent Definition  */

/*  PlugIn should be ready/able to handle this event after initiating a
 * background file transfer
 *
 * The event as received should be parsed primarily by the getDLEventCondition()
 * method. This will allow identification of download start, progress, and end
 * states.
 *
 * Other accessor methods contain status, byte counts, etc.
 *
 * A PlugIn may safely destroy its EvtHandler after receipt of an
 * OCPN_downloadEvent with getDLEventCondition == OCPN_DL_EVENT_TYPE_END
 */
class DECL_EXP OCPN_downloadEvent : public wxEvent {
public:
  OCPN_downloadEvent(wxEventType commandType = wxEVT_NULL, int id = 0);
  ~OCPN_downloadEvent();

  // accessors
  _OCPN_DLStatus getDLEventStatus() { return m_stat; }
  OCPN_DLCondition getDLEventCondition() { return m_condition; }

  void setDLEventStatus(_OCPN_DLStatus stat) { m_stat = stat; }
  void setDLEventCondition(OCPN_DLCondition cond) { m_condition = cond; }

  void setTotal(long bytes) { m_totalBytes = bytes; }
  void setTransferred(long bytes) { m_sofarBytes = bytes; }
  long getTotal() { return m_totalBytes; }
  long getTransferred() { return m_sofarBytes; }

  void setComplete(bool b_complete) { m_b_complete = b_complete; }
  bool getComplete() { return m_b_complete; }

  // required for sending with wxPostEvent()
  wxEvent *Clone() const;

private:
  OCPN_DLStatus m_stat;
  OCPN_DLCondition m_condition;

  long m_totalBytes;
  long m_sofarBytes;
  bool m_b_complete;
};

// extern WXDLLIMPEXP_CORE const wxEventType wxEVT_DOWNLOAD_EVENT;

#ifdef MAKING_PLUGIN
extern DECL_IMP wxEventType wxEVT_DOWNLOAD_EVENT;
#else
extern DECL_EXP wxEventType wxEVT_DOWNLOAD_EVENT;
#endif

/* API 1.14  */
/* API 1.14  adds some more common functions to avoid unnecessary code
 * duplication */

extern DECL_EXP bool LaunchDefaultBrowser_Plugin(wxString url);

// API 1.14 Extra canvas Support

/**
 * Renders AIS targets on a secondary OpenGL canvas.
 *
 * Allow plugins to render AIS targets on additional OpenGL
 * canvases beyond the main chart display. This is useful for creating auxiliary
 * navigation views that need to show vessel traffic.
 *
 * @param glcanvas The OpenGL canvas to draw on
 * @param vp Current viewport settings for the canvas
 *
 * @note Canvas must have valid OpenGL context
 * @note Uses same AIS target database as main display
 * @note Respects global AIS display settings
 * @note Draws using same symbology as main display
 */
extern DECL_EXP void PlugInAISDrawGL(wxGLCanvas *glcanvas,
                                     const PlugIn_ViewPort &vp);

/**
 * Sets text color for a UI element.
 *
 * @param TextElement UI element ID. See OCPNGetFont()
 * @param color New text color to use
 * @return True if element found and color was set, false if not found
 * @note Changes are held in memory only and not persisted to config
 * @see OCPNGetFont()
 */
extern DECL_EXP bool PlugInSetFontColor(const wxString TextElement,
                                        const wxColour color);

// API 1.15

/**
 * Gets physical display size in millimeters.
 *
 * Returns the physical display dimensions to allow proper scaling of navigation
 * elements like range rings, which need to maintain true physical size
 * regardless of screen resolution.
 *
 * @return Display size in millimeters
 *
 * @note Returns the primary display size on multi-monitor systems
 * @note Used for physical size calculations like range rings
 * @note Some systems may return nominal rather than actual size
 * @note Value is independent of screen resolution/DPI
 */
extern DECL_EXP double PlugInGetDisplaySizeMM();

/**
 * Creates or finds a font in the font cache.
 *
 * @param point_size Font size in points
 * @param family Font family (wxFONTFAMILY_SWISS etc)
 * @param style Style flags (wxFONTSTYLE_NORMAL etc)
 * @param weight Weight flags (wxFONTWEIGHT_NORMAL etc)
 * @param underline True for underlined font
 * @param facename Font face name, empty for default
 * @param encoding Font encoding, wxFONTENCODING_DEFAULT for default
 * @return Pointer to cached wxFont, do not delete
 */
extern DECL_EXP wxFont *FindOrCreateFont_PlugIn(
    int point_size, wxFontFamily family, wxFontStyle style, wxFontWeight weight,
    bool underline = false, const wxString &facename = wxEmptyString,
    wxFontEncoding encoding = wxFONTENCODING_DEFAULT);

/**
 * Gets minimum available GSHHS coastline data quality.
 *
 * Returns the lowest quality level of GSHHS (Global Self-consistent,
 * Hierarchical, High-resolution Shoreline) data installed. GSHHS data comes in
 * different resolutions/qualities for different zoom levels.
 *
 * @return Integer quality level:
 *         0 = Crude quality (lowest)
 *         1 = Low quality
 *         2 = Intermediate quality
 *         3 = High quality
 *         4 = Full quality (highest)
 *
 * @note Used to determine available coastline detail levels
 * @note Lower qualities use less memory but show less detail
 * @note Quality selection affects chart drawing performance
 */
extern DECL_EXP int PlugInGetMinAvailableGshhgQuality();

/**
 * Gets maximum available GSHHS coastline data quality.
 *
 * Returns the highest quality level of GSHHS data installed.
 *
 * @return Integer quality level (0-4, see PlugInGetMinAvailableGshhgQuality())
 *
 * @note Used to determine best available coastline detail
 * @note Higher qualities provide more detailed shorelines
 * @note Higher qualities require more storage and memory
 */
extern DECL_EXP int PlugInGetMaxAvailableGshhgQuality();

/**
 * Controls autopilot route handling.
 *
 * Enables or disables sending of active route information to autopilot.
 * When enabled, OpenCPN will send route waypoints and other navigation
 * data to connected autopilot devices.
 *
 * @param enable True to enable autopilot route updates, false to disable
 *
 * @note Requires compatible autopilot interface
 * @note Only affects routes, not tracks or individual waypoints
 * @note Route must be activated for updates to be sent
 */
extern DECL_EXP void PlugInHandleAutopilotRoute(bool enable);

// API 1.16
//
/**
 * Returns an installed plugin's data directory given a plugin name.
 *
 * Platform-specific behavior:
 * - On Linux: Searches directories from XDG_DATA_DIRS env variable for
 *   "opencpn/plugins/<plugin_name>"
 * - On other platforms: Checks GetSharedDataDir() + "/opencpn/plugins/" +
 * plugin_name
 *
 * @param plugin_name The name of the plugin to find data for (e.g.,
 * "weather_routing_pi")
 * @return Path to the plugin's data directory if found, empty string if not
 * found
 */
extern DECL_EXP wxString GetPluginDataDir(const char *plugin_name);

/**
 * Checks if OpenCPN is in shutdown process.
 *
 * @return True if OpenCPN is shutting down, false otherwise
 */
extern DECL_EXP bool ShuttingDown(void);

//  Support for MUI MultiCanvas model

/**
 * Gets the currently focused chart canvas.
 *
 * Returns the chart canvas window that currently has input focus in
 * multi-canvas configurations. A canvas gains focus when:
 *
 * - User clicks within the canvas area
 * - User uses keyboard shortcuts to switch canvas focus
 * - Canvas is explicitly given focus programmatically
 *
 * Focus determines which canvas:
 * - Receives keyboard input events
 * - Is the target for navigation commands
 * - Shows active canvas indicators
 * - Gets tool/menu actions by default
 *
 * @return Pointer to focused canvas window, NULL if none focused
 *
 * @see GetCanvasIndexUnderMouse() To find canvas under mouse cursor
 * @see GetCanvasCount() To get total number of canvases
 * @see GetCanvasByIndex() To get canvas by index number
 */
extern DECL_EXP wxWindow *PluginGetFocusCanvas();

/**
 * Gets the canvas currently designated for overlay rendering.
 *
 * Returns the chart canvas window that should receive plugin overlay
 * graphics in multi-canvas configurations.
 *
 * @return Pointer to overlay target canvas, NULL if none available
 */
extern DECL_EXP wxWindow *PluginGetOverlayRenderCanvas();

/**
 * Centers specified canvas on given position at given scale.
 *
 * @param canvas Canvas window to adjust view for
 * @param lat Target latitude in decimal degrees
 * @param lon Target longitude in decimal degrees
 * @param scale Display scale factor (larger = more zoomed in)
 */
extern "C" DECL_EXP void CanvasJumpToPosition(wxWindow *canvas, double lat,
                                              double lon, double scale);
/**
 * Adds item to canvas context menu.
 *
 * @param pitem Menu item to add
 * @param pplugin Pointer to plugin instance
 * @param name Optional canvas name for multi-canvas configs
 * @return Menu item ID for later reference
 *
 * @note IDs should be stored to later remove/modify items
 * @note Empty name targets default/primary canvas
 */
extern "C" DECL_EXP int AddCanvasMenuItem(wxMenuItem *pitem,
                                          opencpn_plugin *pplugin,
                                          const char *name = "");
/**
 * Removes a context menu item completely.
 *
 * Unlike SetCanvasMenuItemViz(), this permanently removes the menu item.
 *
 * @param item Menu item ID returned from AddCanvasMenuItem()
 * @param name Optional canvas name for multi-canvas configs (empty for default)
 */
extern "C" DECL_EXP void RemoveCanvasMenuItem(int item, const char *name = "");

/**
 * Temporarily changes context menu item visibility.
 *
 * Hides or shows a context menu item without removing it.
 *
 * @param item Menu item ID to modify
 * @param viz True to show item, false to hide
 * @param name Optional canvas name (empty for default)
 */
extern "C" DECL_EXP void SetCanvasMenuItemViz(int item, bool viz,
                                              const char *name = "");

/**
 * Sets menu item enabled/disabled state.
 *
 * Greys out (disables) or enables a context menu item.
 *
 * @param item Menu item ID to modify
 * @param grey True to disable (grey out), false to enable
 * @param name Optional canvas name (empty for default)
 */
extern "C" DECL_EXP void SetCanvasMenuItemGrey(int item, bool grey,
                                               const char *name = "");

// Extract waypoints, routes and tracks
/**
 * Gets GUID of currently selected waypoint.
 *
 * @return GUID string, empty if no waypoint selected
 */
extern DECL_EXP wxString GetSelectedWaypointGUID_Plugin();

/**
 * Gets GUID of currently selected route.
 *
 * @return GUID string, empty if no route selected
 */
extern DECL_EXP wxString GetSelectedRouteGUID_Plugin();

/**
 * Gets GUID of currently selected track.
 *
 * @return GUID string, empty if no track selected
 */
extern DECL_EXP wxString GetSelectedTrackGUID_Plugin();

/**
 * Gets waypoint details by GUID.
 *
 * @param guid GUID of waypoint to get
 * @return Unique pointer to waypoint data, NULL if not found
 */
extern DECL_EXP std::unique_ptr<PlugIn_Waypoint> GetWaypoint_Plugin(
    const wxString &);

/**
 * Gets route details by GUID.
 *
 * @param guid GUID of route to get
 * @return Unique pointer to route data, NULL if not found
 */
extern DECL_EXP std::unique_ptr<PlugIn_Route> GetRoute_Plugin(const wxString &);

/**
 * Gets track details by GUID.
 *
 * @param guid GUID of track to get
 * @return Unique pointer to track data, NULL if not found
 */
extern DECL_EXP std::unique_ptr<PlugIn_Track> GetTrack_Plugin(const wxString &);

/**
 * Gets canvas window under mouse cursor.
 *
 * @return Pointer to canvas window, NULL if mouse not over any canvas
 */
extern DECL_EXP wxWindow *GetCanvasUnderMouse();

/**
 * Gets index of chart canvas under mouse cursor.
 *
 * Returns the index of the canvas window that the mouse cursor is currently
 * positioned over in multi-canvas configurations. Note that having the mouse
 * over a canvas does not automatically give that canvas focus - it merely
 * indicates mouse position.
 *
 * @return Canvas index (0-based), -1 if mouse not over any canvas
 * @note This returns mouse position only - does not affect canvas focus
 * @see GetFocusCanvas() To determine which canvas has input focus
 * @see GetCanvasCount() To get total number of canvases
 */
extern DECL_EXP int GetCanvasIndexUnderMouse();

// extern DECL_EXP std::vector<wxWindow *> GetCanvasArray();
/**
 * Gets chart canvas window by index.
 *
 * Retrieves pointer to a specific canvas window by its index number in
 * multi-canvas configurations.
 *
 * @param canvasIndex Index of canvas to get (0-based)
 * @return Pointer to canvas window, NULL if invalid index
 *
 * @note Index 0 is always the main/primary canvas
 * @note Valid indices are 0 to GetCanvasCount()-1
 */
extern DECL_EXP wxWindow *GetCanvasByIndex(int canvasIndex);

/**
 * Gets total number of chart canvases.
 *
 * Returns the number of active chart canvas windows.
 *
 * @return Number of canvases (always >= 1)
 *
 * @note Minimum is 1 for main canvas
 * @note Maximum depends on available screen space
 */
extern DECL_EXP int GetCanvasCount();

/**
 * Checks if chart should pan when cursor near edge.
 *
 * Tests if automatic chart panning should occur based on cursor position
 * for a specific canvas.
 *
 * @param x Cursor X coordinate in canvas space
 * @param y Cursor Y coordinate in canvas space
 * @param dragging True if mouse is being dragged
 * @param margin Edge margin width in pixels
 * @param delta Pan amount in pixels
 * @param canvasIndex Index of canvas to check
 * @return True if panning should occur
 */
extern DECL_EXP bool CheckMUIEdgePan_PlugIn(int x, int y, bool dragging,
                                            int margin, int delta,
                                            int canvasIndex);

/**
 * Sets mouse cursor for specific canvas.
 *
 * Changes the mouse cursor displayed over a specific chart canvas window.
 *
 * @param pCursor Pointer to cursor to use, NULL for default
 * @param canvasIndex Index of target canvas
 */
extern DECL_EXP void SetMUICursor_PlugIn(wxCursor *pCursor, int canvasIndex);

// API 1.17
//

/**
 * Gets bounding rectangle of master toolbar.
 *
 * Returns the screen coordinates and size of OpenCPN's main toolbar.
 * Useful for positioning plugin UI elements relative to the toolbar.
 *
 * @return wxRect containing toolbar position and dimensions
 */
extern DECL_EXP wxRect GetMasterToolbarRect();

/**
 * Coordinate format options for lat/lon display.
 *
 * These values define the different formats available for displaying
 * geographic coordinates in the UI.
 */
enum SDDMFORMAT {
  /** Format like "50° 12.345' N" */
  DEGREES_DECIMAL_MINUTES = 0,

  /** Format like "50.12345° N" */
  DECIMAL_DEGREES,

  /** Format like "50° 12' 34.5" N" */
  DEGREES_MINUTES_SECONDS,

  /** Number of available formats */
  END_SDDMFORMATS
};

/**
 * Gets currently selected latitude/longitude display format.
 *
 * Returns the coordinate format currently configured in OpenCPN settings.
 *
 * @return Format enum value (see SDDMFORMAT)
 */
extern DECL_EXP int GetLatLonFormat(void);

// API 1.17

/**
 * Resets cross track error to zero.
 *
 * Sets the current cross track error (XTE) value to zero. This affects
 * navigation displays and autopilot output.
 *
 * @note Useful when starting a new course or clearing navigation errors
 * @note XTE is the perpendicular distance from current position to planned
 * route
 */
extern "C" DECL_EXP void ZeroXTE();

/**
 * Extended waypoint class with additional navigation features.
 *
 * This class extends the basic PlugIn_Waypoint with additional capabilities
 * for managing waypoint display, range rings, routes, and more complex
 * navigation features.
 *
 * Key features include:
 * - Free-standing vs route waypoint status tracking
 * - Route membership counting
 * - Scale-dependent visibility control
 * - Range ring visualization
 * - Hyperlink attachments
 * - Enhanced name/description fields
 * - Activity status tracking
 *
 * Used when more control is needed over waypoint behavior than provided
 * by the basic PlugIn_Waypoint class.
 *
 * @note Requires OpenCPN API version 1.17 or later
 * @note Provides better support for complex navigation features
 * @note Use basic PlugIn_Waypoint for simple waypoint needs
 */
class DECL_EXP PlugIn_Waypoint_Ex {
public:
  PlugIn_Waypoint_Ex();
  PlugIn_Waypoint_Ex(double lat, double lon, const wxString &icon_ident,
                     const wxString &wp_name, const wxString &GUID = "",
                     const double ScaMin = 1e9, const bool bNameVisible = false,
                     const int nRanges = 0, const double RangeDistance = 1.0,
                     const wxColor RangeColor = wxColor(255, 0, 0));
  ~PlugIn_Waypoint_Ex();
  /**
   * Initializes waypoint properties to default values.
   *
   * Sets standard default values for waypoint fields:
   * - Zero latitude/longitude
   * - Empty name and description
   * - Current timestamp
   * - Visible but inactive state
   * - Default icon and settings
   */
  void InitDefaults();

  /**
   * Gets "free-standing" status of waypoint.
   *
   * A waypoint is considered "free-standing" if it was:
   * - Created by dropping a point in the GUI
   * - Imported from a GPX file
   * - Added via AddSingleWaypoint API
   * (vs being part of a route)
   *
   * @return True if waypoint is free-standing, false if part of route
   */
  bool GetFSStatus();

  /**
   * Gets number of routes containing this waypoint.
   *
   * Returns count of how many routes include this waypoint.
   * Used to manage waypoint deletion and route integrity.
   *
   * @return Number of routes waypoint belongs to (0 if free-standing)
   */
  int GetRouteMembershipCount();

  double m_lat;                //!< Latitude in decimal degrees
  double m_lon;                //!< Longitude in decimal degrees
  wxString m_GUID;             //!< Globally unique identifier
  wxString m_MarkName;         //!< Display name of waypoint
  wxString m_MarkDescription;  //!< Optional description text
  wxDateTime m_CreateTime;     //!< Creation timestamp in UTC.
  bool IsVisible;              //!< Visibility state on chart
  bool IsActive;               //!< Active state (e.g. destination)

  double scamin;       //!< Minimum display scale (1:X) for waypoint visibility
  bool b_useScamin;    //!< True to enable scale-dependent visibility
  bool IsNameVisible;  //!< True to show waypoint name on chart

  int nrange_rings;       //!< Number of range rings to display around waypoint
  double RangeRingSpace;  //!< Distance between range rings in preferred units
  wxColour RangeRingColor;  //!< Color to draw range rings

  wxString IconName;         //!< Name of icon to use for waypoint symbol
  wxString IconDescription;  //!< User-friendly description of icon

  /**
   * List of hyperlinks associated with this waypoint.
   * Can link to web pages, local files, charts, etc.
   * Ownership is transferred to waypoint.
   */
  Plugin_HyperlinkList *m_HyperlinkList;
};

WX_DECLARE_LIST(PlugIn_Waypoint_Ex, Plugin_WaypointExList);

class DECL_EXP PlugIn_Waypoint_ExV2 {
public:
  PlugIn_Waypoint_ExV2();
  PlugIn_Waypoint_ExV2(double lat, double lon, const wxString &icon_ident,
                       const wxString &wp_name, const wxString &GUID = "",
                       const double ScaMin = 1e9, const double ScaMax = 1e6,
                       const bool bNameVisible = false,
                       const int nRangeRings = 0,
                       const double RangeDistance = 1.0,
                       const int RangeRingSpaceUnits = 0,  // 0:nm, 1:km
                       const wxColor RangeColor = wxColor(255, 0, 0),
                       const double WaypointArrivalRadius = 0.0,
                       const bool ShowWaypointRangeRings = false,
                       const double PlannedSpeed = 0.0,
                       const wxString TideStation = wxEmptyString);

  virtual ~PlugIn_Waypoint_ExV2();

  /**
   * Gets "free-standing" status of waypoint.
   *
   * A waypoint is considered "free-standing" if it was:
   * - Created by dropping a point in the GUI
   * - Imported from a GPX file
   * - Added via AddSingleWaypoint API
   * (vs being part of a route)
   *
   * @return True if waypoint is free-standing, false if part of route
   */
  bool GetFSStatus();

  /**
   * Gets number of routes containing this waypoint.
   *
   * Returns count of how many routes include this waypoint.
   * Used to manage waypoint deletion and route integrity.
   *
   * @return Number of routes waypoint belongs to (0 if free-standing)
   */
  int GetRouteMembershipCount();

  double m_lat;                //!< Latitude in decimal degrees
  double m_lon;                //!< Longitude in decimal degrees
  wxString m_GUID;             //!< Globally unique identifier
  wxString m_MarkName;         //!< Display name of waypoint
  wxString m_MarkDescription;  //!< Optional description text
  wxDateTime m_CreateTime;     //!< Creation timestamp in UTC.
  bool IsVisible;              //!< Visibility state on chart
  bool IsActive;               //!< Active state (e.g. destination)

  double scamin;       //!< Minimum display scale (1:X) for waypoint visibility
  bool b_useScamin;    //!< True to enable scale-dependent visibility
  bool IsNameVisible;  //!< True to show waypoint name on chart

  int nrange_rings;       //!< Number of range rings to display around waypoint
  double RangeRingSpace;  //!< Distance between range rings in preferred units
  int RangeRingSpaceUnits;  //!< Units for range ring spacing - 0:nm, 1:km
  wxColour RangeRingColor;  //!< Color to draw range rings

  wxString IconName;         //!< Name of icon to use for waypoint symbol
  wxString IconDescription;  //!< User-friendly description of icon

  /**
   * List of hyperlinks associated with this waypoint.
   * Can link to web pages, local files, charts, etc.
   * Ownership is transferred to waypoint.
   */
  Plugin_HyperlinkList *m_HyperlinkList;

  // New fields that are not in PlugIn_Waypoint_Ex
  double scamax;  //!< Maximum display scale (1:X) for waypoint visibility
  double m_PlannedSpeed;           //!< Planned speed for next leg (knots)
  bool m_bShowWaypointRangeRings;  //!< True to show range rings on chart
  double m_WaypointArrivalRadius;  //!< Arrival radius in nautical miles
  /** Estimated departure time in UTC, or wxInvalidDateTime if not set. */
  wxDateTime m_ETD;
  /** Tide Station Identifier. */
  wxString m_TideStation;

protected:
  /**
   * Initializes waypoint properties to default values.
   *
   * Sets standard default values for waypoint fields:
   * - Zero latitude/longitude
   * - Empty name and description
   * - Current timestamp
   * - Visible but inactive state
   * - Default icon and settings
   */
  void InitDefaults();
};

WX_DECLARE_LIST(PlugIn_Waypoint_ExV2, Plugin_WaypointExV2List);

/**
 * Extended route class for managing complex route features.
 *
 * This class extends the basic route functionality to provide additional
 * capabilities for managing routes with extended properties and waypoint
 * collections.
 *
 * Key features include:
 * - Route naming and description
 * - Start/end point labeling
 * - Visibility control
 * - Active route status
 * - Extended waypoint list management
 * - Global unique identification
 *
 * Used when advanced route management capabilities are needed beyond
 * the basic PlugIn_Route class.
 *
 * @note Requires OpenCPN API version 1.17 or later
 * @note Works with PlugIn_Waypoint_Ex for extended waypoint features
 * @note Used by navigation and routing plugins
 */
class DECL_EXP PlugIn_Route_Ex {
public:
  PlugIn_Route_Ex(void);
  ~PlugIn_Route_Ex(void);

  wxString m_NameString;   //!< User-visible name of the route
  wxString m_StartString;  //!< Description of route start point
  wxString m_EndString;    //!< Description of route end point
  wxString m_GUID;         //!< Globally unique identifier
  bool m_isActive;         //!< True if this is the active route
  bool m_isVisible;        //!< True if route should be displayed
  wxString m_Description;  //!< Optional route description/notes

  /**
   * List of waypoints making up this route in order.
   * First point is start, last point is end.
   * Uses PlugIn_Waypoint_Ex for extended waypoint features.
   */
  Plugin_WaypointExList *pWaypointList;
};

/**
 * Enhanced route class for working with PlugIn_Waypoint_ExV2 waypoints.
 *
 * This class provides functionality similar to PlugIn_Route_Ex but works
 * with the new PlugIn_Waypoint_ExV2 class.
 *
 * Key features include:
 * - Route naming and description
 * - Start/end point labeling
 * - Visibility control
 * - Active route status
 * - Extended waypoint list management
 * - Global unique identification
 */
class DECL_EXP PlugIn_Route_ExV2 {
public:
  PlugIn_Route_ExV2();
  virtual ~PlugIn_Route_ExV2();

  wxString m_NameString;   //!< User-visible name of the route
  wxString m_StartString;  //!< Description of route start point
  wxString m_EndString;    //!< Description of route end point
  wxString m_GUID;         //!< Globally unique identifier
  bool m_isActive;         //!< True if this is the active route
  bool m_isVisible;        //!< True if route should be displayed
  wxString m_Description;  //!< Optional route description/notes

  /**
   * List of waypoints making up this route in order.
   * First point is start, last point is end.
   * Uses PlugIn_Waypoint_ExV2 for enhanced features.
   */
  Plugin_WaypointExV2List *pWaypointList;
};

/**
 * Gets array of route GUIDs.
 *
 * Returns list of globally unique identifiers for all routes.
 *
 * @return Array of route GUID strings
 */
extern DECL_EXP wxArrayString GetRouteGUIDArray(void);

/**
 * Gets array of track GUIDs.
 *
 * Returns list of globally unique identifiers for all tracks.
 *
 * @return Array of track GUID strings
 */
extern DECL_EXP wxArrayString GetTrackGUIDArray(void);

/**
 * Gets extended waypoint data by GUID.
 *
 * @param GUID Unique identifier of waypoint to get
 * @param pwaypoint Pointer to receive waypoint data
 * @return True if waypoint found and data copied
 */
extern DECL_EXP bool GetSingleWaypointEx(wxString GUID,
                                         PlugIn_Waypoint_Ex *pwaypoint);

/**
 * Gets extended waypoint data by GUID.
 *
 * @param GUID Unique identifier of waypoint to get
 * @param pwaypoint Pointer to receive waypoint data
 * @return True if waypoint found and data copied
 */
extern DECL_EXP bool GetSingleWaypointExV2(wxString GUID,
                                           PlugIn_Waypoint_ExV2 *pwaypoint);

/**
 * Adds a waypoint with extended properties.
 *
 * @param pwaypoint Extended waypoint data to add
 * @param b_permanent True to save persistently, false for temporary
 * @return True if successfully added
 */
extern DECL_EXP bool AddSingleWaypointEx(PlugIn_Waypoint_Ex *pwaypoint,
                                         bool b_permanent = true);

/**
 * Adds a waypoint with extended V2 properties.
 *
 * @param pwaypoint Extended V2 waypoint data to add
 * @param b_permanent True to save persistently, false for temporary
 * @return True if successfully added
 */
extern DECL_EXP bool AddSingleWaypointExV2(PlugIn_Waypoint_ExV2 *pwaypoint,
                                           bool b_permanent = true);

/**
 * Updates an existing extended waypoint.
 *
 * @param pwaypoint Updated waypoint data (GUID must match existing)
 * @return True if successfully updated
 */
extern DECL_EXP bool UpdateSingleWaypointEx(PlugIn_Waypoint_Ex *pwaypoint);

/**
 * Updates an existing extended V2 waypoint.
 *
 * @param pwaypoint Updated V2 waypoint data (GUID must match existing)
 * @return True if successfully updated
 */
extern DECL_EXP bool UpdateSingleWaypointExV2(PlugIn_Waypoint_ExV2 *pwaypoint);

/**
 * Adds a route with extended features.
 *
 * @param proute Extended route data to add
 * @param b_permanent True to save persistently, false for temporary
 * @return True if successfully added
 */
extern DECL_EXP bool AddPlugInRouteEx(PlugIn_Route_Ex *proute,
                                      bool b_permanent = true);

/**
 * Adds a new route with V2 waypoints.
 *
 * @param proute Route to add
 * @return True if route added successfully
 */
extern DECL_EXP bool AddPlugInRouteExV2(PlugIn_Route_ExV2 *proute,
                                        bool b_permanent = true);

/**
 * Updates an existing extended route.
 *
 * @param proute Updated route data (GUID must match existing)
 * @return True if successfully updated
 */
extern DECL_EXP bool UpdatePlugInRouteEx(PlugIn_Route_Ex *proute);

/**
 * Updates existing route with V2 waypoints.
 *
 * Modifies an existing route with new V2 waypoint data.
 *
 * @param proute Updated route data (GUID must match existing)
 * @return True if route updated successfully
 */
extern DECL_EXP bool UpdatePlugInRouteExV2(PlugIn_Route_ExV2 *proute);

/**
 * Gets extended waypoint by GUID.
 *
 * Returns smart pointer to waypoint with extended properties.
 *
 * @param guid GUID of waypoint to get
 * @return Unique pointer to waypoint data, NULL if not found
 */
extern DECL_EXP std::unique_ptr<PlugIn_Waypoint_Ex> GetWaypointEx_Plugin(
    const wxString &GUID);

/**
 * Gets complete waypoint details by GUID.
 *
 * Returns a comprehensive waypoint object with all V2 properties.
 *
 * @param GUID Unique identifier of waypoint to retrieve
 * @return Unique pointer to waypoint data, NULL if not found
 */
extern DECL_EXP std::unique_ptr<PlugIn_Waypoint_ExV2> GetWaypointExV2_Plugin(
    const wxString &GUID);

/**
 * Gets extended route by GUID.
 *
 * Returns smart pointer to route with extended properties.
 *
 * @param guid GUID of route to get
 * @return Unique pointer to route data, NULL if not found
 */
extern DECL_EXP std::unique_ptr<PlugIn_Route_Ex> GetRouteEx_Plugin(
    const wxString &GUID);

/**
 * Gets route details with V2 waypoints by GUID.
 *
 * Returns comprehensive route object with all V2 waypoint properties.
 *
 * @param GUID Unique identifier of route to retrieve
 * @return Unique pointer to route data, NULL if not found
 */
extern DECL_EXP std::unique_ptr<PlugIn_Route_ExV2> GetRouteExV2_Plugin(
    const wxString &GUID);

/**
 * Gets GUID of currently active waypoint.
 *
 * @return GUID string, empty if no active waypoint
 */
extern DECL_EXP wxString GetActiveWaypointGUID(void);

/**
 * Gets GUID of currently active route.
 *
 * @return GUID string, empty if no active route
 */
extern DECL_EXP wxString GetActiveRouteGUID(void);

// API 1.18

/**
 * Gets content scaling factor for current display.
 *
 * Returns the display scaling factor used for scaling UI content on high DPI
 * displays. Common on GTK3 and Mac Retina displays.
 *
 * @return Display content scale factor:
 *         1.0 = Standard/100% scaling
 *         2.0 = 200% scaling/Retina
 *         etc.
 *
 * @note Used for proper scaling on high DPI displays
 * @note Different from WinDIPScaleFactor()
 */
extern DECL_EXP double OCPN_GetDisplayContentScaleFactor();

/**
 * Gets Windows-specific DPI scaling factor.
 *
 * Returns Windows DPI scaling factor configured in display settings.
 *
 * @return Windows DPI scale factor:
 *         1.0 = 96 DPI (100%)
 *         1.25 = 120 DPI (125%)
 *         1.5 = 144 DPI (150%)
 *         etc.
 *
 * @note Windows-specific scaling mechanism
 * @note Used in conjunction with ContentScaleFactor
 */
extern DECL_EXP double OCPN_GetWinDIPScaleFactor();

/**
 * Gets list of configured priority mapping schemes.
 *
 * Returns list of available data source priority mapping configurations.
 *
 * @return Vector of priority map names
 */
extern DECL_EXP std::vector<std::string> GetPriorityMaps();

/**
 * Sets and applies new priority mapping scheme.
 *
 */
extern DECL_EXP void UpdateAndApplyPriorityMaps(std::vector<std::string> map);

/**
 * Gets list of active priority identifiers.
 *
 * Returns list of currently active data source priority identifiers.
 *
 * @return Vector of active priority identifiers
 */
extern DECL_EXP std::vector<std::string> GetActivePriorityIdentifiers();

/**
 * Gets global watchdog timeout value.
 *
 * Returns timeout in seconds used by system watchdog monitoring.
 *
 * @return Timeout value in seconds
 */
extern DECL_EXP int GetGlobalWatchdogTimoutSeconds();

/**
 * Filter options for object queries.
 *
 * Controls whether layer objects are included when querying routes,
 * tracks and waypoints.
 */
typedef enum _OBJECT_LAYER_REQ {
  OBJECTS_ALL = 0,     //!< Return all objects including those in layers
  OBJECTS_NO_LAYERS,   //!< Exclude objects that are in layers
  OBJECTS_ONLY_LAYERS  //!< Return only objects that are in layers
} OBJECT_LAYER_REQ;

// FIXME (dave)  Implement these
/**
 * Gets array of route GUIDs with layer filtering.
 *
 * Returns list of globally unique identifiers for routes, filtered by layer
 * requirements.
 *
 * @param req Filter option:
 *        - OBJECTS_ALL: Return all routes regardless of layer membership
 *        - OBJECTS_NO_LAYERS: Only return routes that are not in any layer
 *        - OBJECTS_ONLY_LAYERS: Only return routes that are part of layers
 * @return Array of matching route GUID strings
 *
 * @note Layers are collections of navigation objects that can be toggled as a
 * unit
 * @note Routes may exist independently or as part of one or more layers
 */
extern DECL_EXP wxArrayString GetRouteGUIDArray(OBJECT_LAYER_REQ req);

/**
 * Gets array of track GUIDs with layer filtering.
 *
 * Returns list of globally unique identifiers for tracks, filtered by layer
 * requirements.
 *
 * @param req Filter option:
 *        - OBJECTS_ALL: Return all tracks regardless of layer membership
 *        - OBJECTS_NO_LAYERS: Only return tracks that are not in any layer
 *        - OBJECTS_ONLY_LAYERS: Only return tracks that are part of layers
 * @return Array of matching track GUID strings
 *
 * @note Tracks are recorded vessel paths that may be organized into layers
 * @note Layer membership affects track visibility and management
 */
extern DECL_EXP wxArrayString GetTrackGUIDArray(OBJECT_LAYER_REQ req);

/**
 * Gets array of waypoint/track GUIDs with layer filtering.
 *
 * Returns list of globally unique identifiers for waypoints, filtered by layer
 * requirements.
 *
 * @param req Filter option:
 *        - OBJECTS_ALL: Return all waypoints regardless of layer membership
 *        - OBJECTS_NO_LAYERS: Only return waypoints that are not in any layer
 *        - OBJECTS_ONLY_LAYERS: Only return waypoints that are part of layers
 * @return Array of matching waypoint GUID strings
 *
 * @note Independent waypoints and route waypoints may belong to layers
 * @note Layer filtering allows selective processing of waypoint subsets
 * @see GetRouteGUIDArray()
 * @see GetTrackGUIDArray()
 */
extern DECL_EXP wxArrayString GetWaypointGUIDArray(OBJECT_LAYER_REQ req);

/**   listen-notify interface   */

/* Listening to messages. */
class ObservableListener;

/** The event used by notify/listen. */
class ObservedEvt;

// This is a verbatim copy from observable_evt.h, don't define twice.
#ifndef OBSERVABLE_EVT_H
#define OBSERVABLE_EVT_H

wxDECLARE_EVENT(obsNOTIFY, ObservedEvt);

/**
 * Custom event class for OpenCPN's notification system.
 *
 * This class extends wxCommandEvent to add support for passing shared pointer
 * payloads between event producers and consumers. It's used by the
 * notification/listening system to deliver messages and data between plugins
 * and the core application.
 *
 * Key features:
 * - Built on standard wxWidgets event system
 * - Supports arbitrary data payloads via shared pointers
 * - Thread-safe message passing
 * - Reference-counted memory management
 */
class ObservedEvt : public wxCommandEvent {
public:
  /**
   * Constructor.
   *
   * @param commandType Event type ID, defaults to obsNOTIFY
   * @param id Window identifier, defaults to 0
   */
  ObservedEvt(wxEventType commandType = obsNOTIFY, int id = 0)
      : wxCommandEvent(commandType, id) {}
  ObservedEvt(const ObservedEvt &event) : wxCommandEvent(event) {
    this->m_shared_ptr = event.m_shared_ptr;
  }

  /**
   * Creates a cloned copy of this event.
   *
   * Required by wxWidgets event system for event propagation.
   *
   * @return Pointer to new copy of this event
   */
  wxEvent *Clone() const { return new ObservedEvt(*this); }

  /**
   * Gets the event's payload data.
   *
   * @return Shared pointer to the payload data
   */
  std::shared_ptr<const void> GetSharedPtr() const { return m_shared_ptr; }

  /**
   * Sets the event's payload data.
   *
   * @param p Shared pointer to payload data
   */
  void SetSharedPtr(std::shared_ptr<const void> p) { m_shared_ptr = p; }

private:
  std::shared_ptr<const void> m_shared_ptr;  //!< Payload data pointer
};

#endif  // OBSERVABLE_EVT_H

class ObservableListener;

/**
 * Identifier for NMEA 2000 message types.
 *
 * Wraps a PGN (Parameter Group Number) value to identify NMEA 2000 message
 * types when subscribing to messages.
 */
struct NMEA2000Id {
  const uint64_t id;  //!< The PGN value
  /**
   * Creates identifier from PGN value.
   * @param value PGN number (e.g. 127245 for Rudder)
   */
  NMEA2000Id(int value) : id(static_cast<uint64_t>(value)) {};
};

/**
 * Gets listener for NMEA 2000 messages.
 *
 * Creates a listener to receive NMEA 2000 messages of a specific type.
 *
 * @param id PGN identifier for message type to listen for
 * @param ev Event type to generate on message receipt
 * @param handler Event handler to receive messages
 * @return Shared pointer to listener object
 */
extern DECL_EXP std::shared_ptr<ObservableListener> GetListener(
    NMEA2000Id id, wxEventType ev, wxEvtHandler *handler);

/**
 * Identifier for NMEA 0183 sentence types.
 *
 * Wraps a sentence identifier (e.g. "RMC", "GLL") to identify NMEA 0183
 * messages when subscribing.
 */
struct NMEA0183Id {
  const std::string id;  //!< Sentence identifier
  /**
   * Creates identifier from sentence type.
   * @param s Sentence identifier (e.g. "RMC")
   */
  NMEA0183Id(const std::string &s) : id(s) {};
};

/**
 * Gets listener for NMEA 0183 messages.
 *
 * Creates a listener to receive NMEA 0183 messages of a specific type.
 *
 * @param id Sentence identifier to listen for
 * @param ev Event type to generate on message receipt
 * @param handler Event handler to receive messages
 * @return Shared pointer to listener object
 */
extern DECL_EXP std::shared_ptr<ObservableListener> GetListener(
    NMEA0183Id id, wxEventType ev, wxEvtHandler *handler);

/**
 * Identifier for Signal K paths.
 *
 * Wraps a Signal K path (e.g. "navigation.position") to identify data
 * to subscribe to.
 */
struct SignalkId {
  const std::string id;  //!< Signal K path
  /**
   * Creates identifier from Signal K path.
   * @param s Signal K path (e.g. "navigation.position")
   */
  SignalkId(const std::string &s) : id(s) {};
};

/**
 * Gets listener for Signal K updates.
 *
 * Creates a listener to receive Signal K updates for a specific path.
 *
 * @param id Signal K path to listen for
 * @param ev Event type to generate on updates
 * @param handler Event handler to receive updates
 * @return Shared pointer to listener object
 */
extern DECL_EXP std::shared_ptr<ObservableListener> GetListener(
    SignalkId id, wxEventType ev, wxEvtHandler *handler);

/**
   Return N2K payload for a received n2000 message of type id in ev.
   The vector returned is described in the following example

        [147,19,                     // Header bytes, unused
         3,                          // N2K priority
         16,240,1,                   // example pgn 126992 encoded little endian
         255,                        // N2K destination address
         1,                          // N2K origin address
         255,255,255,255,            // timestamp, unused
         8,                          // count of following NMEA2000 data
         13,240,207,76,208,3,94,40,  // NMEA2000 data
         85                          // CRC byte, unused,not included in count
        ];
*/
extern DECL_EXP std::vector<uint8_t> GetN2000Payload(NMEA2000Id id,
                                                     ObservedEvt ev);

/**
 *  Get SignalK status payload after receiving a message.
 *  @return pointer to a wxJSONValue map object. Typical usage:
 *
 *      auto ptr = GetSignalkPayload(ev);
 *      const auto msg = *std::static_pointer_cast<const wxJSONValue>(payload);
 *
 *  The map contains the following entries:
 *  - "Data": the parsed json message
 *  - "ErrorCount": int, the number of parsing errors
 *  - "WarningCount": int, the number of parsing warnings
 *  - "Errors": list of strings, error messages.
 *  - "Warnings": list of strings, warning messages..
 *  - "Context": string, message context
 *  - "ContextSelf": string, own ship context.
 */
extern DECL_EXP std::shared_ptr<void> GetSignalkPayload(ObservedEvt ev);

/**
 * Return source identifier (iface) of a received n2000 message of type id
 * in ev.
 */
extern DECL_EXP std::string GetN2000Source(NMEA2000Id id, ObservedEvt ev);

/** Return payload in a received n0183 message of type id in ev. */
extern DECL_EXP std::string GetN0183Payload(NMEA0183Id id, ObservedEvt ev);

/**
 * Navigation data message identifier.
 *
 * This struct acts as a facade for identifying basic navigation data message
 * types when subscribing to navigation updates.
 */
struct NavDataId {
  const int type;  //!< Message type identifier (currently only 0 supported)
  /**
   * Default constructor.
   * Creates identifier for basic navigation data subscription.
   */
  NavDataId() : type(0) {}
};

/**
 * Gets listener for navigation data updates.
 *
 * Creates a listener to receive basic navigation data updates including
 * position, course, speed etc.
 *
 * @param id NavDataId identifying message type to listen for
 * @param ev Event type to generate on updates
 * @param handler Event handler to receive updates
 * @return Shared pointer to listener object
 */
extern DECL_EXP std::shared_ptr<ObservableListener> GetListener(
    NavDataId id, wxEventType ev, wxEvtHandler *handler);

/**
 * Basic navigation data structure.
 *
 * Contains essential navigation data available to plugins including position,
 * course, speed and heading information.
 */
struct PluginNavdata {
  double lat;   //!< Latitude in decimal degrees
  double lon;   //!< Longitude in decimal degrees
  double sog;   //!< Speed over ground in knots
  double cog;   //!< Course over ground in degrees
  double var;   //!< Magnetic variation in degrees
  double hdt;   //!< True heading in degrees
  time_t time;  //!< UTC timestamp of data
};

/** Return BasicNavDataMsg decoded data available in ev */
extern DECL_EXP PluginNavdata GetEventNavdata(ObservedEvt ev);

/** Plugin API supporting direct access to comm drivers for output purposes */
/*
 * Plugins may access comm ports for direct output.
 * The general program flow for a plugin may look something like this
 * pseudo-code:
 * 1.  Plugin will query OCPN core for a list of active comm drivers.
 * 2.  Plugin will inspect the list, and query OCPN core for driver
 *     attributes.
 * 3.  Plugin will select a comm driver with appropriate attributes for output.
 * 4.  Plugin will register a list of PGNs expected to be transmitted
 *     (N2K specific)
 * 5.  Plugin may then send a payload buffer to a specific comm driver for
 *     output as soon as possible.
 *
 * The mechanism for specifying a particular comm driver uses the notion of
 * "handles". Each active comm driver has an associated opaque handle, managed
 * by OCPN core. All references by a plugin to a driver are by means of its
 * handle. Handles should be considered to be "opaque", meaning that the exact
 * contents of the handle are of no specific value to the plugin, and only
 * have meaning to the OCPN core management of drivers.
 */

/** Definition of OCPN DriverHandle  */
typedef std::string DriverHandle;

/** Error return values  */

/**
 * Result codes for communication driver operations.
 *
 * These values indicate the result status of operations performed on
 * communication drivers like sending messages or registering parameters.
 */
typedef enum CommDriverResult {
  /** Operation completed successfully */
  RESULT_COMM_NO_ERROR = 0,
  /** Invalid or unknown driver handle specified */
  RESULT_COMM_INVALID_HANDLE,
  /** Invalid parameters provided to operation */
  RESULT_COMM_INVALID_PARMS,
  /** Error occurred during transmission */
  RESULT_COMM_TX_ERROR,
  /** Failed to register with gateway device */
  RESULT_COMM_REGISTER_GATEWAY_ERROR,
  /** Failed to register PGN parameters */
  RESULT_COMM_REGISTER_PGN_ERROR
} _CommDriverResult;

/** Query OCPN core for a list of active drivers  */
extern DECL_EXP std::vector<DriverHandle> GetActiveDrivers();

/**
 * Query a specific driver for attributes.
 *
 * Driver attributes are available from OCPN core as a hash map of
 * tag->attribute pairs. There is a defined set of common tags guaranteed
 * for every driver. Both tags and attributes are defined as std::string.
 * Here is the list of common tag-attribute pairs.
 *
 * Tag              Attribute definition
 * ----------       --------------------
 * "protocol"       Comm bus device protocol, such as "NMEA0183", "NMEA2000"
 *
 *
 */
extern DECL_EXP const std::unordered_map<std::string, std::string>
GetAttributes(DriverHandle handle);

/* Writing to a specific driver  */
/**
 * Send a non-NMEA2000 message. The call is not blocking.
 * @param handle Obtained from GetActiveDrivers()
 * @param payload Message data, for example a complete Nmea0183 message.
 *        From 1.19: if the handle "protocol" attribute is "internal" it is
 *        parsed as <id><space><message> where the id is used when listening/
 *        subscribing to message.
 * @return value number of bytes queued for transmission.
 */
extern DECL_EXP CommDriverResult WriteCommDriver(
    DriverHandle handle, const std::shared_ptr<std::vector<uint8_t>> &payload);

/** Send a PGN message to an NMEA2000 address.  */
extern DECL_EXP CommDriverResult WriteCommDriverN2K(
    DriverHandle handle, int PGN, int destinationCANAddress, int priority,
    const std::shared_ptr<std::vector<uint8_t>> &payload);

/**
 * Register PGNs that this application intends to transmit for some NMEA 2000
 * adapters like Actisense NGT-1.
 *
 * This function is required specifically for NMEA 2000 adapters like the
 * Actisense NGT-1. For these devices, registration of transmit PGNs is required
 * before sending any messages. This is an adapter-specific requirement, not a
 * requirement of the NMEA 2000 standard itself.
 *
 * This function is only implemented for serial NMEA 2000 adapters (specifically
 * the Actisense NGT-1). For other connection types (TCP, UDP), the function
 * will return success and perform no registration.
 *
 * @param handle    The driver handle obtained from GetActiveDrivers()
 * @param pgn_list  List of PGNs this application will transmit
 *
 * @return RESULT_COMM_NO_ERROR if registration successful
 *         RESULT_COMM_INVALID_PARMS if pgn_list is empty
 *         RESULT_COMM_INVALID_HANDLE if handle is invalid
 *         RESULT_COMM_REGISTER_PGN_ERROR if PGN registration failed
 *
 * Example usage:
 * @code
 *     // Register to transmit wind data and rudder commands
 *     std::vector<int> pgns = {130306,  // Wind Data
 *                             127245};  // Rudder
 *     auto result = RegisterTXPGNs(driver_handle, pgns);
 *     if (result != RESULT_COMM_NO_ERROR) {
 *         // Handle error
 *     }
 * @endcode
 *
 * @note For Actisense NGT-1 adapters, this registration must be done before
 * transmitting any NMEA 2000 messages. The registration remains in effect until
 * the application closes or explicitly registers a new list. Before any device
 * can transmit messages on an NMEA 2000 network, it must first announce which
 * message types (PGNs) it will transmit. This allows other devices on the
 * network to:
 *    - Know what data is available.
 *    - Request specific data from the transmitting device.
 *    - Properly handle network address claims.
 */
extern DECL_EXP CommDriverResult RegisterTXPGNs(DriverHandle handle,
                                                std::vector<int> &pgn_list);

// API 1.20
//

/**
 * Navigation mode options for chart display orientation.
 *
 * These values define how the chart display is oriented relative to north,
 * course, or heading. Different modes are useful for different navigation
 * scenarios.
 */
typedef enum _PI_NavMode {
  /**
   * North Up Mode - True north is always at the top of the display.
   * Most natural for general navigation and voyage planning.
   * - Static display orientation
   * - Easy chart reading and bearing plotting
   * - Traditional paper chart orientation
   */
  PI_NORTH_UP_MODE = 0,

  /**
   * Course Up Mode - Current course over ground at the top.
   * Useful for route following and collision avoidance.
   * - Display rotates to match vessel course
   * - Forward direction always at top
   * - Helps visualize vessel's path
   */
  PI_COURSE_UP_MODE,

  /**
   * Head Up Mode - Current vessel heading at the top.
   * Best for close quarters maneuvering and docking.
   * - Display rotates to match vessel heading
   * - Shows view as seen from vessel
   * - Matches visual references
   */
  PI_HEAD_UP_MODE,

} PI_NavMode;

/** Facade for NavAddrPluginMsg. */
struct PluginMsgId {
  const std::string id;
  PluginMsgId(const std::string &s) : id(s) {};
};

/**
 * Return listener for plugin messages, internal or received on the REST
 * interface.
 */
extern DECL_EXP std::shared_ptr<ObservableListener> GetListener(
    PluginMsgId id, wxEventType ev, wxEvtHandler *handler);

/**
 * Retrieve the string in a plugin message, internal or received on the
 * REST insterface.
 */
extern DECL_EXP std::string GetPluginMsgPayload(PluginMsgId id, ObservedEvt ev);

/**
 * Exits OpenCPN application.
 *
 * Triggers a clean shutdown of OpenCPN.
 */
extern DECL_EXP void ExitOCPN();

/**
 * Requests window refresh.
 *
 * Triggers a redraw of the specified window.
 *
 * @param win Window to refresh
 * @param eraseBackground True to erase background before redraw
 */
extern "C" DECL_EXP void RequestWindowRefresh(wxWindow *win,
                                              bool eraseBackground);

/**
 * Gets full screen state.
 *
 * @return True if application is in full screen mode
 */
extern DECL_EXP bool GetFullScreen();

/**
 * Sets full screen mode.
 *
 * @param full_screen_on True to enable full screen mode, false for windowed
 */
extern DECL_EXP void SetFullScreen(bool full_screen_on);

/**
 * Enables/disables touch interface mode.
 *
 * @param enable True to enable touch optimized interface
 */
extern DECL_EXP void EnableTouchMode(bool enable);

/**
 * Gets touch interface mode state.
 *
 * @return True if touch interface is enabled
 */
extern DECL_EXP bool GetTouchMode();

/**
 * Sets a color in the global color scheme.
 *
 * @param table Color scheme name ("DAY", "DUSK", "NIGHT")
 * @param name Color identifier within scheme
 * @param color New color value to set
 */
extern DECL_EXP void SetGlobalColor(std::string table, std::string name,
                                    wxColor color);

/**
 * Gets a color from the global color scheme.
 *
 * @param map_name Color scheme name ("DAY", "DUSK", "NIGHT")
 * @param name Color identifier within scheme
 * @return Color value, or wxNullColour if not found
 */
extern DECL_EXP wxColor GetGlobalColorD(std::string map_name, std::string name);

/**
 * Shows/hides the status bar.
 *
 * @param enable True to show status bar, false to hide
 */
extern DECL_EXP void EnableStatusBar(bool enable);

/**
 * Shows/hides the main menu bar.
 *
 * @param enable True to show menu bar, false to hide
 */
extern DECL_EXP void EnableMenu(bool enable);

/**
 * Gets status bar visibility state.
 *
 * @return True if status bar is enabled/visible
 */
extern DECL_EXP bool GetEnableStatusBar();

/**
 * Gets menu bar visibility state.
 *
 * @return True if menu bar is enabled/visible
 */
extern DECL_EXP bool GetEnableMenu();

/**
 * Sets the navigation mode for a specific chart canvas.
 *
 * @param mode Navigation mode to set:
 *        - PI_NORTH_UP_MODE: North always at top
 *        - PI_COURSE_UP_MODE: Course over ground at top
 *        - PI_HEAD_UP_MODE: Vessel heading at top
 * @param CanvasIndex Index of target canvas (0-based)
 */
extern DECL_EXP void SetNavigationMode(PI_NavMode mode, int CanvasIndex);

/**
 * Gets current navigation mode for a canvas.
 *
 * @param CanvasIndex Index of canvas to query
 * @return Current navigation mode
 */
extern DECL_EXP PI_NavMode GetNavigationMode(int CanvasIndex);

/**
 * Enables/disables look-ahead mode for a canvas.
 * Look-ahead mode shifts chart view ahead of vessel position.
 *
 * @param enable True to enable look-ahead
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableLookaheadMode(bool enable, int CanvasIndex);

/**
 * Gets look-ahead mode state for a canvas.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if look-ahead mode enabled
 */
extern DECL_EXP bool GetEnableLookaheadMode(int CanvasIndex);

/**
 * Controls visibility of MUI (Mobile/Touch User Interface) bar.
 *
 * @param enable True to show MUI bar
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableMUIBar(bool enable, int CanvasIndex);

/**
 * Controls visibility of compass/GPS status icon.
 *
 * @param enable True to show compass icon
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableCompassGPSIcon(bool enable, int CanvasIndex);

/**
 * Controls visibility of chart info bar.
 *
 * @param enable True to show chart bar
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableChartBar(bool enable, int CanvasIndex);

/**
 * Gets MUI bar visibility state.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if MUI bar is visible
 */
extern DECL_EXP bool GetEnableMUIBar(int CanvasIndex);

/**
 * Gets compass icon visibility state.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if compass icon is visible
 */
extern DECL_EXP bool GetEnableCompassGPSIcon(int CanvasIndex);

/**
 * Gets chart bar visibility state.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if chart bar is visible
 */
extern DECL_EXP bool GetEnableChartBar(int CanvasIndex);

/**
 * Controls visibility of canvas focus indicator.
 * Shows which canvas currently has input focus.
 *
 * @param enable True to show focus indicator
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableCanvasFocusBar(bool enable, int CanvasIndex);

/**
 * Gets focus indicator visibility state.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if focus indicator is visible
 */
extern DECL_EXP bool GetEnableCanvasFocusBar(int CanvasIndex);

/*
 *  Allow plugin control of "Chart Panel Options" dialog
 */

/**
 * Controls latitude/longitude grid display.
 *
 * @param enable True to show lat/lon grid lines
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableLatLonGrid(bool enable, int CanvasIndex);

/**
 * Controls chart outline display.
 *
 * @param enable True to show available chart boundaries
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableChartOutlines(bool enable, int CanvasIndex);

/**
 * Controls depth unit display.
 *
 * @param enable True to show depth unit indicators
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableDepthUnitDisplay(bool enable, int CanvasIndex);

/**
 * Controls AIS target display.
 *
 * @param enable True to show AIS targets
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableAisTargetDisplay(bool enable, int CanvasIndex);

/**
 * Controls tide station icon display.
 *
 * @param enable True to show tide station markers
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableTideStationsDisplay(bool enable, int CanvasIndex);

/**
 * Controls current station icon display.
 *
 * @param enable True to show current station markers
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableCurrentStationsDisplay(bool enable, int CanvasIndex);

/**
 * Controls ENC text label display.
 *
 * @param enable True to show ENC feature text labels
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableENCTextDisplay(bool enable, int CanvasIndex);

/**
 * Controls ENC depth sounding display.
 *
 * @param enable True to show depth soundings
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableENCDepthSoundingsDisplay(bool enable,
                                                    int CanvasIndex);

/**
 * Controls buoy/light name label display.
 *
 * @param enable True to show buoy/light names
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableBuoyLightLabelsDisplay(bool enable, int CanvasIndex);

/**
 * Controls light icon display.
 *
 * @param enable True to show light icons
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableLightsDisplay(bool enable, int CanvasIndex);

/**
 * Controls light description text display.
 *
 * @param enable True to show light descriptions
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void EnableLightDescriptionsDisplay(bool enable,
                                                    int CanvasIndex);

/**
 * Sets ENC (Electronic Navigation Chart) feature display category.
 *
 * @param cat Display category to use (BASE, STANDARD, OTHER, etc)
 * @param CanvasIndex Index of target canvas
 */
extern DECL_EXP void SetENCDisplayCategory(PI_DisCat cat, int CanvasIndex);

/**
 * Gets latitude/longitude grid visibility state.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if lat/lon grid lines are visible
 */
extern DECL_EXP bool GetEnableLatLonGrid(int CanvasIndex);

/**
 * Gets chart outline visibility state.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if chart boundaries are visible
 */
extern DECL_EXP bool GetEnableChartOutlines(int CanvasIndex);

/**
 * Gets depth unit display state.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if depth unit indicators are shown
 */
extern DECL_EXP bool GetEnableDepthUnitDisplay(int CanvasIndex);

/**
 * Gets AIS target display state.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if AIS targets are visible
 */
extern DECL_EXP bool GetEnableAisTargetDisplay(int CanvasIndex);

/**
 * Gets tide station icon visibility.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if tide station markers are shown
 */
extern DECL_EXP bool GetEnableTideStationsDisplay(int CanvasIndex);

/**
 * Gets current station icon visibility.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if current station markers are shown
 */
extern DECL_EXP bool GetEnableCurrentStationsDisplay(int CanvasIndex);

/**
 * Gets ENC text label visibility.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if ENC feature text labels are shown
 */
extern DECL_EXP bool GetEnableENCTextDisplay(int CanvasIndex);

/**
 * Gets ENC depth sounding visibility.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if depth soundings are shown
 */
extern DECL_EXP bool GetEnableENCDepthSoundingsDisplay(int CanvasIndex);

/**
 * Gets buoy/light label visibility.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if buoy/light name labels are shown
 */
extern DECL_EXP bool GetEnableBuoyLightLabelsDisplay(int CanvasIndex);

/**
 * Gets light icon visibility.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if light icons are shown
 */
extern DECL_EXP bool GetEnableLightsDisplay(int CanvasIndex);

/**
 * Gets light description text visibility.
 *
 * @param CanvasIndex Index of canvas to query
 * @return True if light descriptions are shown
 */
extern DECL_EXP bool GetEnableLightDescriptionsDisplay(int CanvasIndex);

/**
 * Gets current ENC display category.
 *
 * @param CanvasIndex Index of canvas to query
 * @return Current display category:
 *         - PI_DISPLAYBASE: Base features only
 *         - PI_STANDARD: Standard features
 *         - PI_OTHER: All features
 *         etc.
 */
extern DECL_EXP PI_DisCat GetENCDisplayCategory(int CanvasIndex);

/**
 * Set follow mode for a specific canvas.
 * When follow mode is enabled, the chart will automatically move with the
 * vessel position.
 *
 * @param CanvasIndex Index of the target canvas (0 for the first canvas).
 * @param enable_follow True to enable follow mode, false to disable.
 */
extern DECL_EXP void PluginSetFollowMode(int CanvasIndex, bool enable_follow);

/**
 * Get the current follow mode status for a specific canvas.
 *
 * @param CanvasIndex Index of the target canvas (0 for the first canvas).
 * @return True if follow mode is enabled, false otherwise.
 */
extern DECL_EXP bool PluginGetFollowMode(int CanvasIndex);

/**
 * Enable or disable tracking mode.
 * When tracking mode is enabled, the vessel's position is recorded in the
 * active track.
 *
 * @param enable True to enable tracking, false to disable.
 */
extern DECL_EXP void SetTrackingMode(bool enable);

/**
 * Get the current tracking mode status.
 *
 * @return True if tracking mode is enabled, false otherwise.
 */
extern DECL_EXP bool GetTrackingMode();

/**
 * Set the application color scheme.
 * Changes the color scheme for the entire application including charts, UI
 * elements, and plugin displays.
 *
 * @param cs The color scheme to apply (DAY, DUSK, or NIGHT).
 */
extern DECL_EXP void SetAppColorScheme(PI_ColorScheme cs);

/**
 * Get the current application color scheme.
 *
 * @return Current color scheme (DAY, DUSK, or NIGHT).
 */
extern DECL_EXP PI_ColorScheme GetAppColorScheme();

/**
 * Enable or disable the split-screen layout.
 * Controls whether OpenCPN uses multiple chart canvases in a split-screen
 * configuration.
 *
 * @param enable True to enable split-screen layout, false for single canvas
 * mode.
 */
extern DECL_EXP void EnableSplitScreenLayout(bool enable = true);

// ChartCanvas control utilities

/**
 * Zoom a specific chart canvas by the given factor.
 * Values greater than 1.0 zoom in, values less than 1.0 zoom out.
 *
 * @param CanvasIndex Index of the target canvas (0 for the first canvas).
 * @param factor Zoom factor to apply.
 */
extern DECL_EXP void PluginZoomCanvas(int CanvasIndex, double factor);

/**
 * Check if the main toolbar is enabled.
 *
 * @return True if the main toolbar is currently visible, false otherwise.
 */
extern DECL_EXP bool GetEnableMainToolbar();

/**
 * Show or hide the main toolbar.
 *
 * @param enable True to show the toolbar, false to hide it.
 */
extern DECL_EXP void SetEnableMainToolbar(bool enable);

/**
 * Display the global settings dialog.
 * Opens the main OpenCPN options/preferences dialog.
 */
extern DECL_EXP void ShowGlobalSettingsDialog();

/**
 * Center the chart view on the own ship position for a specific canvas.
 *
 * @param CanvasIndex Index of the target canvas (0 for the first canvas).
 */
extern DECL_EXP void PluginCenterOwnship(int CanvasIndex);

/**
 * Check if 10 Hz update rate is enabled.
 * When enabled, position updates and screen redraws occur at approximately 10
 * times per second.
 *
 * @return True if 10 Hz update is enabled, false otherwise.
 */
extern DECL_EXP bool GetEnableTenHertzUpdate();

/**
 * Enable or disable 10 Hz update rate.
 * Controls the frequency of position updates and screen redraws.
 *
 * @param enable True to enable 10 Hz updates, false for standard update rate.
 */
extern DECL_EXP void EnableTenHertzUpdate(bool enable);

/**
 * Flush configuration changes to disk and reload settings.
 * Forces immediate saving of any pending configuration changes and reloads all
 * settings.
 */
extern DECL_EXP void ConfigFlushAndReload();

/*
 * Reload and restore all connections by direct read of config file
 */
extern DECL_EXP void ReloadConfigConnections();

/**
 * Plugin Notification Framework support
 */
enum class PI_NotificationSeverity : int {
  PI_kInformational = 0,
  PI_kWarning = 1,
  PI_kCritical = 2
};

class PI_Notification {
public:
  PI_Notification(PI_NotificationSeverity _severity,
                  const std::string &_message, int _timeout_start,
                  int _timeout_left, std::string _guid);
  virtual ~PI_Notification() {};

  PI_NotificationSeverity severity;
  std::string message;
  int auto_timeout_left;
  int auto_timeout_start;
  std::string guid;
  std::string action_verb;  // Either "ACK" or "POST", when set by a
                            // PI_Notification message payload.
                            // Empty otherwise
};

extern DECL_EXP int GetActiveNotificationCount();
extern DECL_EXP PI_NotificationSeverity GetMaxActiveNotificationLevel();
extern DECL_EXP std::string RaiseNotification(
    const PI_NotificationSeverity _severity, const std::string &_message,
    int timeout_secs = -1);
extern DECL_EXP bool AcknowledgeNotification(const std::string &guid);
extern DECL_EXP std::vector<std::shared_ptr<PI_Notification>>
GetActiveNotifications();
extern DECL_EXP void EnableNotificationCanvasIcon(bool enable);

/*
 * Messaging interface for Notification Framework
 *
 */

/*
 * Typical use pattern
 *
 * 1)  Establish listener
  wxDEFINE_EVENT(EVT_NOTIFICATION_FRAME, ObservedEvt);
  static std::shared_ptr<ObservableListener> listener_note;
  NotificationMsgId note_id = NotificationMsgId();
  listener_note = GetListener(note_id, EVT_NOTIFICATION_FRAME, this);
  Bind(EVT_NOTIFICATION_FRAME, [&](ObservedEvt ev) { HandleNotification(ev); });
 *
 *
 *
 * 2)  Define actions on receipt
  static void HandleNotification(ObservedEvt &ev) {
    NotificationMsgId id;
    std::shared_ptr<PI_Notification>payload=GetNotificationMsgPayload(id, ev);
    if (payload->action_verb == "ACK"){
      // Do Acknowledge notification actions
    }
    else if (payload->action_verb == "POST") {
      // Do Add Notification actions
    }
  }

*/
/** Facade for NotificationMsg. */
struct NotificationMsgId {
  const std::string id;
  NotificationMsgId(const std::string &s) : id(s) {};
  NotificationMsgId() {};
};

/**
 * Return listener for Notification Framework messages
 * interface.
 */
extern DECL_EXP std::shared_ptr<ObservableListener> GetListener(
    NotificationMsgId id, wxEventType ev, wxEvtHandler *handler);

/**
 * Retrieve the Notification Event in a Notification message
 */
extern DECL_EXP std::shared_ptr<PI_Notification> GetNotificationMsgPayload(
    NotificationMsgId id, ObservedEvt ev);

//   * Plugin polled Comm Status support
enum class PI_Conn_Bus : int { N0183 = 0, Signalk = 1, N2000 = 2 };

enum class PI_Comm_State : int {
  Disabled = 0,
  NoStats = 1,
  NoData = 2,
  Unavailable = 3,
  Ok = 4
};

class PI_Comm_Status {
public:
  PI_Comm_State state;
  unsigned rx_count;     ///< Number of bytes received since program start.
  unsigned tx_count;     ///< Number of bytes sent since program start.
  unsigned error_count;  ///< Number of detected errors since program start.
};

extern DECL_EXP PI_Comm_Status GetConnState(const std::string &iface,
                                            PI_Conn_Bus _bus);

extern "C" DECL_EXP int AddCanvasContextMenuItemExt(
    wxMenuItem *pitem, opencpn_plugin *pplugin, const std::string object_type);

#endif  //_PLUGIN_H_
