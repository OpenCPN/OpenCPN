/***************************************************************************
 *
 *
 * Project:  OpenCPN
 * Purpose:  PlugIn Manager Object
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
 **************************************************************************/

#ifndef _PLUGINMGR_H_
#define _PLUGINMGR_H_

#include <wx/wx.h>
#include <wx/dynarray.h>
#include <wx/dynlib.h>

#include <memory>

#include "config.h"

#include "ocpn_plugin.h"
//#include "chcanv.h"                 // for ViewPort
#include "OCPN_Sound.h"
#include "chartimg.h"
#include "catalog_parser.h"
#include "plugin_blacklist.h"
#include "observable.h"
#include "ais_target_data.h"
#include "comm_navmsg.h"
#include "s57chart.h"  // for Object list
#include "semantic_vers.h"

// For widgets...
#include <wx/hyperlink.h>
#include <wx/choice.h>
#include <wx/tglbtn.h>
#include <wx/bmpcbox.h>

#ifndef __OCPN__ANDROID__
#ifdef OCPN_USE_CURL
#include <wx/curl/http.h>
#include <wx/curl/dialog.h>
#endif
#endif

//    Include wxJSON headers
//    We undefine MIN/MAX so avoid warning of redefinition coming from
//    json_defs.h
//    Definitions checked manually, and are identical
#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

#include <wx/json_defs.h>
#include <wx/jsonwriter.h>
#include "plugin_loader.h"

//    Assorted static helper routines

PlugIn_AIS_Target *Create_PI_AIS_Target(AisTargetData *ptarget);

class PluginListPanel;
class PluginPanel;
class pluginUtilHandler;
class MyFrame;

//----------------------------------------------------------------------------
// PlugIn Messaging scheme Event
//----------------------------------------------------------------------------

class OCPN_MsgEvent : public wxEvent {
public:
  OCPN_MsgEvent(wxEventType commandType = wxEVT_NULL, int id = 0);

  OCPN_MsgEvent(const OCPN_MsgEvent &event)
      : wxEvent(event),
        m_MessageID(event.m_MessageID),
        m_MessageText(event.m_MessageText) {}

  ~OCPN_MsgEvent();

  // accessors
  wxString GetID() { return m_MessageID; }
  wxString GetJSONText() { return m_MessageText; }

  void SetID(const wxString &string) { m_MessageID = string; }
  void SetJSONText(const wxString &string) { m_MessageText = string; }

  // required for sending with wxPostEvent()
  wxEvent *Clone() const;

private:
  wxString m_MessageID;
  wxString m_MessageText;
};

extern const wxEventType wxEVT_OCPN_MSG;

enum ActionVerb {
  NOP = 0,
  UPGRADE_TO_MANAGED_VERSION,
  UPGRADE_INSTALLED_MANAGED_VERSION,
  REINSTALL_MANAGED_VERSION,
  DOWNGRADE_INSTALLED_MANAGED_VERSION,
  UNINSTALL_MANAGED_VERSION,
  INSTALL_MANAGED_VERSION
};


class PlugInMenuItemContainer {
public:
  wxMenuItem *pmenu_item;
  opencpn_plugin *m_pplugin;
  bool b_viz;
  bool b_grey;
  int id;
  wxString m_in_menu;
};

//    Define an array of PlugIn MenuItem Containers
WX_DEFINE_ARRAY_PTR(PlugInMenuItemContainer *, ArrayOfPlugInMenuItems);

class PlugInToolbarToolContainer {
public:
  PlugInToolbarToolContainer();
  ~PlugInToolbarToolContainer();

  opencpn_plugin *m_pplugin;
  int id;
  wxString label;
  wxBitmap *bitmap_day;
  wxBitmap *bitmap_dusk;
  wxBitmap *bitmap_night;
  wxBitmap *bitmap_Rollover_day;
  wxBitmap *bitmap_Rollover_dusk;
  wxBitmap *bitmap_Rollover_night;

  wxItemKind kind;
  wxString shortHelp;
  wxString longHelp;
  wxObject *clientData;
  int position;
  bool b_viz;
  bool b_toggle;
  int tool_sel;
  wxString pluginNormalIconSVG;
  wxString pluginRolloverIconSVG;
  wxString pluginToggledIconSVG;
};

//    Define an array of PlugIn ToolbarTool Containers
WX_DEFINE_ARRAY_PTR(PlugInToolbarToolContainer *, ArrayOfPlugInToolbarTools);

//-----------------------------------------------------------------------------------------------------
//
//          The PlugIn Manager Specification
//
//-----------------------------------------------------------------------------------------------------

class BlacklistUI;

class PlugInManager : public wxEvtHandler {
public:
  PlugInManager(MyFrame *parent);
  virtual ~PlugInManager();

  bool RenderAllCanvasOverlayPlugIns(ocpnDC &dc, const ViewPort &vp,
                                     int canvasIndex, int priority);
  bool RenderAllGLCanvasOverlayPlugIns(wxGLContext *pcontext,
                                       const ViewPort &vp, int canvasIndex,
                                       int priority);
  void SendCursorLatLonToAllPlugIns(double lat, double lon);
  void SendViewPortToRequestingPlugIns(ViewPort &vp);
  void PrepareAllPluginContextMenus();

  void NotifySetupOptions();
  void ClosePlugInPanel(PlugInContainer *pic, int ix);
  void CloseAllPlugInPanels(int);

  ArrayOfPlugInToolbarTools &GetPluginToolbarToolArray() {
    return m_PlugInToolbarTools;
  }
  int AddToolbarTool(wxString label, wxBitmap *bitmap, wxBitmap *bmpRollover,
                     wxItemKind kind, wxString shortHelp, wxString longHelp,
                     wxObject *clientData, int position, int tool_sel,
                     opencpn_plugin *pplugin);

  void RemoveToolbarTool(int tool_id);
  void SetToolbarToolViz(int tool_id, bool viz);
  void SetToolbarItemState(int tool_id, bool toggle);
  void SetToolbarItemBitmaps(int item, wxBitmap *bitmap, wxBitmap *bmpDisabled);

  int AddToolbarTool(wxString label, wxString SVGfile, wxString SVGRolloverfile,
                     wxString SVGToggledfile, wxItemKind kind,
                     wxString shortHelp, wxString longHelp,
                     wxObject *clientData, int position, int tool_sel,
                     opencpn_plugin *pplugin);

  void SetToolbarItemBitmaps(int item, wxString SVGfile,
                             wxString SVGfileRollover, wxString SVGfileToggled);

  opencpn_plugin *FindToolOwner(const int id);
  wxString GetToolOwnerCommonName(const int id);
  void ShowDeferredBlacklistMessages();

  ArrayOfPlugInMenuItems &GetPluginContextMenuItemArray() {
    return m_PlugInMenuItems;
  }
  int AddCanvasContextMenuItem(wxMenuItem *pitem, opencpn_plugin *pplugin,
                               const char *name = "");
  void RemoveCanvasContextMenuItem(int item, const char *name = "");
  void SetCanvasContextMenuItemViz(int item, bool viz, const char *name = "");
  void SetCanvasContextMenuItemGrey(int item, bool grey, const char *name = "");

  void SendNMEASentenceToAllPlugIns(const wxString &sentence);
  void SendPositionFixToAllPlugIns(GenericPosDatEx *ppos);
  void SendActiveLegInfoToAllPlugIns(const ActiveLegDat *infos);
  void SendAISSentenceToAllPlugIns(const wxString &sentence);
  void SendJSONMessageToAllPlugins(const wxString &message_id, wxJSONValue v);
  void SendMessageToAllPlugins(const wxString &message_id,
                               const wxString &message_body);
  bool UpDateChartDataTypes();
  void FinalizePluginLoadall();

  int GetJSONMessageTargetCount();
  bool UpdateConfig();
  void SendResizeEventToAllPlugIns(int x, int y);
  void SetColorSchemeForAllPlugIns(ColorScheme cs);
  void NotifyAuiPlugIns(void);
  bool CallLateInit(void);

  bool IsAnyPlugInChartEnabled();

  void SendVectorChartObjectInfo(const wxString &chart, const wxString &feature,
                                 const wxString &objname, double &lat,
                                 double &lon, double &scale, int &nativescale);

  bool SendMouseEventToPlugins(wxMouseEvent &event);
  bool SendKeyEventToPlugins(wxKeyEvent &event);

  void SendBaseConfigToAllPlugIns();
  void SendS52ConfigToAllPlugIns(bool bReconfig = false);
  void SendSKConfigToAllPlugIns();

  void UpdateManagedPlugins();
  bool CheckBlacklistedPlugin(const PluginMetadata plugin);

  void InitCommListeners(void);
  void HandleN0183( std::shared_ptr <const Nmea0183Msg> n0183_msg );
  void HandleSignalK(std::shared_ptr<const SignalkMsg> sK_msg);

  wxArrayString GetPlugInChartClassNameArray(void);

  ListOfPI_S57Obj *GetPlugInObjRuleListAtLatLon(ChartPlugInWrapper *target,
                                                float zlat, float zlon,
                                                float SelectRadius,
                                                const ViewPort &vp);
  wxString CreateObjDescriptions(ChartPlugInWrapper *target,
                                 ListOfPI_S57Obj *rule_list);

  wxString GetLastError();
  MyFrame *GetParentFrame() { return pParent; }

  void DimeWindow(wxWindow *win);
  pluginUtilHandler *GetUtilHandler() { return m_utilHandler; }
  void SetListPanelPtr(PluginListPanel *ptr) { m_listPanel = ptr; }

  ListOfPI_S57Obj *GetLightsObjRuleListVisibleAtLatLon(
      ChartPlugInWrapper *target, float zlat, float zlon, const ViewPort &vp);

private:
  bool CheckBlacklistedPlugin(wxString name, int major, int minor);
  bool CheckBlacklistedPlugin(opencpn_plugin *plugin);

  ObservableListener evt_ais_json_listener;
  ObservableListener evt_blacklisted_plugin_listener;
  ObservableListener evt_deactivate_plugin_listener;
  ObservableListener evt_download_failed_listener;
  ObservableListener evt_download_ok_listener;
  ObservableListener evt_incompatible_plugin_listener;
  ObservableListener evt_load_directory_listener;
  ObservableListener evt_load_plugin_listener;
  ObservableListener evt_plugin_loadall_finalize_listener;
  ObservableListener evt_pluglist_change_listener;
  ObservableListener evt_unreadable_plugin_listener;
  ObservableListener evt_update_chart_types_listener;
  ObservableListener evt_version_incompatible_listener;
  ObservableListener evt_version_incompatible_plugin_listener;
  ObservableListener evt_json_to_all_plugins_listener;
  ObservableListener evt_routeman_json_listener;
  ObservableListener evt_routeman_leginfo_listener;

  ObservableListener m_listener_N0183_all;
  ObservableListener m_listener_SignalK;

  wxBitmap *BuildDimmedToolBitmap(wxBitmap *pbmp_normal,
                                  unsigned char dim_ratio);

  void ProcessLateInit(PlugInContainer *pic);
  void OnPluginDeactivate(const PlugInContainer* pic);
  void HandlePluginLoaderEvents();
  void HandlePluginHandlerEvents();

  MyFrame *pParent;
  std::unique_ptr<BlacklistUI> m_blacklist_ui;

  wxString m_last_error_string;

  ArrayOfPlugInMenuItems m_PlugInMenuItems;
  ArrayOfPlugInToolbarTools m_PlugInToolbarTools;

  wxString m_plugin_location;

  int m_plugin_tool_id_next;
  int m_plugin_menu_item_id_next;
  wxBitmap m_cached_overlay_bm;

  wxArrayString m_plugin_order;
  void SetPluginOrder(wxString serialized_names);
  wxString GetPluginOrder();

  pluginUtilHandler *m_utilHandler;
  PluginListPanel *m_listPanel;
  std::unique_ptr<AbstractBlacklist> m_blacklist;
  std::unordered_map<std::string, std::vector<std::string>> files_by_plugin;

#ifndef __OCPN__ANDROID__
#ifdef OCPN_USE_CURL

public:
  wxCurlDownloadThread *m_pCurlThread;
  // The libcurl handle being re used for the transfer.
  std::shared_ptr<wxCurlBase> m_pCurl;

  // returns true if the error can be ignored
  bool HandleCurlThreadError(wxCurlThreadError err, wxCurlBaseThread *p,
                             const wxString &url = wxEmptyString);
  void OnEndPerformCurlDownload(wxCurlEndPerformEvent &ev);
  void OnCurlDownload(wxCurlDownloadEvent &ev);

  wxEvtHandler *m_download_evHandler;
  long *m_downloadHandle;
  bool m_last_online;
  long m_last_online_chk;
#endif
#endif

  DECLARE_EVENT_TABLE()
};

WX_DEFINE_ARRAY_PTR(PluginPanel *, ArrayOfPluginPanel);

class PluginDownloadDialog;

/*
 * Panel with a single + sign which opens the "Add/download plugins" dialog.
 */
class AddPluginPanel : public wxPanel {
public:
  AddPluginPanel(wxWindow *parent);
  void OnClick(wxMouseEvent &event);
  ~AddPluginPanel();

protected:
  wxBitmap m_bitmap;
  wxStaticBitmap *m_staticBitmap;
  wxWindow *m_parent;
};

/*
 * Panel with buttons to control plugin catalog management.
 */
class CatalogMgrPanel : public wxPanel {
public:
  CatalogMgrPanel(wxWindow *parent);
  ~CatalogMgrPanel();
  void OnUpdateButton(wxCommandEvent &event);
  void SetListPanelPtr(PluginListPanel *listPanel) {
    m_PluginListPanel = listPanel;
  }
  void OnTarballButton(wxCommandEvent &event);
  void OnPluginSettingsButton(wxCommandEvent &event);

protected:
  wxString GetCatalogText(bool);
  void SetUpdateButtonLabel();
  wxString GetImportInitDir();

  wxButton *m_updateButton, *m_advancedButton, *m_tarballButton;
  wxButton *m_adv_button;
  wxStaticText *m_catalogText;
  wxWindow *m_parent;
  PluginListPanel *m_PluginListPanel;
  ObservableListener catalog_listener;
};

#define ID_CMD_BUTTON_PERFORM_ACTION 27663

class PluginListPanel : public wxScrolledWindow {
  DECLARE_EVENT_TABLE()

public:
  PluginListPanel(wxWindow *parent, wxWindowID id, const wxPoint &pos,
                  const wxSize &size, ArrayOfPlugIns *pPluginArray);
  ~PluginListPanel();

  void SelectPlugin(PluginPanel *pi);
  void MoveUp(PluginPanel *pi);
  void MoveDown(PluginPanel *pi);
  void UpdateSelections();
  void UpdatePluginsOrder();

  /** Complete reload from plugins array. */
  void ReloadPluginPanels();
  void SelectByName(wxString &name);

private:
  void AddPlugin(PlugInContainer *pic);
  int ComputePluginSpace(ArrayOfPluginPanel plugins, wxBoxSizer *sizer);
  // void Clear();

  ArrayOfPlugIns *m_pPluginArray;
  ArrayOfPluginPanel m_PluginItems;
  PluginPanel *m_PluginSelected;
  wxString m_selectedName;
  int m_pluginSpacer;
};

/** Invokes client browser on plugin info_url when clicked. */
class WebsiteButton : public wxPanel {
public:
  WebsiteButton(wxWindow *parent, const char *url);
  ~WebsiteButton(){};
  void SetURL(std::string url) { m_url = url; }

protected:
  std::string m_url;
};

class PluginPanel : public wxPanel {
  DECLARE_EVENT_TABLE()

public:
  PluginPanel(wxPanel *parent, wxWindowID id, const wxPoint &pos,
              const wxSize &size, PlugInContainer *p_plugin);
  ~PluginPanel();

  void OnPluginSelected(wxMouseEvent &event);
  void OnPluginSelectedUp(wxMouseEvent &event);
  void DoPluginSelect();

  void SetSelected(bool selected);
  void OnPluginPreferences(wxCommandEvent &event);
  void OnPluginEnableToggle(wxCommandEvent &event);
  void OnPluginAction(wxCommandEvent &event);
  void OnPluginUninstall(wxCommandEvent &event);
  void OnPluginUp(wxCommandEvent &event);
  void OnPluginDown(wxCommandEvent &event);
  void SetEnabled(bool enabled);
  bool GetSelected() { return m_bSelected; }
  PlugInContainer *GetPluginPtr() { return m_pPlugin; };
  void SetActionLabel(wxString &label);
  ActionVerb GetAction() { return m_action; }
  PlugInContainer *GetPlugin() { return m_pPlugin; }
  void OnPaint(wxPaintEvent &event);

private:
  PluginListPanel *m_PluginListPanel;
  bool m_bSelected;
  PlugInContainer *m_pPlugin;
  wxStaticText *m_pName;
  wxStaticText *m_pVersion;
  wxStaticText *m_pDescription;
  wxBoxSizer *m_pButtons;
  wxStaticBitmap *m_itemStaticBitmap;
  wxStaticBitmap *m_itemStatusIconBitmap;
  wxButton *m_pButtonPreferences;
  wxButton *m_pButtonAction, *m_pButtonUninstall;

  wxCheckBox *m_cbEnable;
  WebsiteButton *m_info_btn;
  ActionVerb m_action;
  int m_penWidthUnselected, m_penWidthSelected;
};

//  API 1.11 adds access to S52 Presentation library
//  These are some wrapper conversion utilities

class S52PLIB_Context {
public:
  S52PLIB_Context() {
    bBBObj_valid = false;
    bCS_Added = false;
    bFText_Added = false;
    CSrules = NULL;
    FText = NULL;
    ChildRazRules = NULL;
    MPSRulesList = NULL;
    LUP = NULL;
  };

  ~S52PLIB_Context(){};

  BoundingBox BBObj;  // lat/lon BBox of the rendered object
  bool bBBObj_valid;    // set after the BBObj has been calculated once.

  Rules *CSrules;  // per object conditional symbology
  int bCS_Added;

  S52_TextC *FText;
  int bFText_Added;
  wxRect rText;

  LUPrec *LUP;
  ObjRazRules *ChildRazRules;
  mps_container *MPSRulesList;
};

void CreateCompatibleS57Object(PI_S57Obj *pObj, S57Obj *cobj,
                               chart_context *pctx);
void UpdatePIObjectPlibContext(PI_S57Obj *pObj, S57Obj *cobj);

#endif  // _PLUGINMGR_H_
