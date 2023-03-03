///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __WMMUIDIALOG_H__
#define __WMMUIDIALOG_H__

#include <wx/frame.h>
#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/radiobox.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class WmmUIDialogBase
///////////////////////////////////////////////////////////////////////////////
class WmmUIDialogBase : public wxFrame {
private:
protected:
  wxStaticText* m_staticText8;
  wxStaticText* m_staticText9;
  wxStaticText* m_staticText10;
  wxStaticText* m_staticText11;
  wxStaticText* m_staticText12;
  wxStaticText* m_staticText14;
  wxStaticText* m_staticText13;
  wxStaticText* m_staticText81;
  wxStaticText* m_staticText91;
  wxStaticText* m_staticText101;
  wxStaticText* m_staticText111;
  wxStaticText* m_staticText121;
  wxStaticText* m_staticText141;
  wxStaticText* m_staticText131;

  // Virtual event handlers, overide them in your derived class
  virtual void EnablePlotChanged(wxCommandEvent& event) { event.Skip(); }
  virtual void PlotSettings(wxCommandEvent& event) { event.Skip(); }

public:
  wxBoxSizer* bSframe;
  wxStaticBoxSizer* sbSboat;
  wxFlexGridSizer* gSboat;
  wxTextCtrl* m_tbF;
  wxTextCtrl* m_tbH;
  wxTextCtrl* m_tbX;
  wxTextCtrl* m_tbY;
  wxTextCtrl* m_tbZ;
  wxTextCtrl* m_tbI;
  wxTextCtrl* m_tbD;
  wxStaticBoxSizer* sbScursor;
  wxFlexGridSizer* gScursor;
  wxTextCtrl* m_tcF;
  wxTextCtrl* m_tcH;
  wxTextCtrl* m_tcX;
  wxTextCtrl* m_tcY;
  wxTextCtrl* m_tcZ;
  wxTextCtrl* m_tcI;
  wxTextCtrl* m_tcD;
  wxStaticBoxSizer* sbPlot;
  wxCheckBox* m_cbEnablePlot;
  wxButton* m_bPlotSettings;

    WmmUIDialogBase(wxWindow* parent, wxWindowID id = wxID_ANY,
                  const wxString& title = _("WMM"),
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxSize(250, 485),
                  long style = wxCAPTION | wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR);
  ~WmmUIDialogBase();
  void OnKey(wxKeyEvent& ke);
  void OnClose(wxCloseEvent& event);
  void OnClose(wxCommandEvent& event);

  DECLARE_EVENT_TABLE()
};

///////////////////////////////////////////////////////////////////////////////
/// Class WmmPrefsDialog
///////////////////////////////////////////////////////////////////////////////
class WmmPrefsDialog : public wxDialog {
private:
protected:
  wxStdDialogButtonSizer* m_sdbSizer1;
  wxButton* m_sdbSizer1OK;
  wxButton* m_sdbSizer1Cancel;

public:
  wxRadioBox* m_rbViewType;
  wxCheckBox* m_cbShowPlotOptions;
  wxCheckBox* m_cbShowAtCursor;
  wxCheckBox* m_cbLiveIcon;
  wxCheckBox* m_cbShowIcon;
  wxSlider* m_sOpacity;

  WmmPrefsDialog(wxWindow* parent, wxWindowID id = wxID_ANY,
                 const wxString& title = _("WMM Preferences"),
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxCAPTION | wxDEFAULT_DIALOG_STYLE);
  ~WmmPrefsDialog();
};

///////////////////////////////////////////////////////////////////////////////
/// Class WmmPlotSettingsDialogBase
///////////////////////////////////////////////////////////////////////////////
class WmmPlotSettingsDialogBase : public wxDialog {
private:
protected:
  wxStaticText* m_staticText15;
  wxStaticText* m_staticText18;
  wxStaticText* m_staticText19;
  wxStaticText* m_staticText181;
  wxStaticText* m_staticText191;
  wxStaticText* m_staticText20;
  wxButton* m_bAbout;
  wxButton* m_bSave;
  wxButton* m_bCancel;

  // Virtual event handlers, overide them in your derived class
  virtual void About(wxCommandEvent& event) { event.Skip(); }
  virtual void Save(wxCommandEvent& event) { event.Skip(); }
  virtual void Cancel(wxCommandEvent& event) { event.Skip(); }

public:
  wxCheckBox* m_cbDeclination;
  wxSpinCtrl* m_scDeclinationSpacing;
  wxCheckBox* m_cbInclination;
  wxSpinCtrl* m_scInclinationSpacing;
  wxCheckBox* m_cbFieldStrength;
  wxSpinCtrl* m_scFieldStrengthSpacing;
  // wxDatePickerCtrl* m_dpDate;
  wxSlider* m_sStep;
  wxSlider* m_sPoleAccuracy;

  WmmPlotSettingsDialogBase(wxWindow* parent, wxWindowID id = wxID_ANY,
                            const wxString& title = _("Magnetic Plot Settings"),
                            const wxPoint& pos = wxDefaultPosition,
                            const wxSize& size = wxSize(375, 270),
                            long style = wxDEFAULT_DIALOG_STYLE);
  ~WmmPlotSettingsDialogBase();
};

#endif  //__WMMUIDIALOG_H__
