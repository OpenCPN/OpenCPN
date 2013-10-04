///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __GRIBUIDIALOGBASE_H__
#define __GRIBUIDIALOGBASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/slider.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/statline.h>
#include <wx/radiobox.h>

///////////////////////////////////////////////////////////////////////////

#define ID_TIMELINE 1000
#define ID_CB_WIND_SPEED 1001
#define ID_CB_SIG_WAVE_HEIGHT 1002
#define ID_CB_CURRENT_VELOCITY 1003
#define ID_CB_PRESSURE 1004
#define ID_CB_SEA_TEMPERATURE 1005

///////////////////////////////////////////////////////////////////////////////
/// Class GRIBUIDialogBase
///////////////////////////////////////////////////////////////////////////////
class GRIBUIDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxBitmapButton* m_bpPrev;
		wxComboBox* m_cRecordForecast;
		wxBitmapButton* m_bpNext;
		wxBitmapButton* m_bpNow;
		wxBitmapButton* m_bpPlay;
		wxBitmapButton* m_bpOpenFile;
		wxBitmapButton* m_bpSettings;
		wxBitmapButton* m_bpRequest;
		wxFlexGridSizer* m_fgTrackingControls;
		wxTextCtrl* m_tcWindSpeed;
		wxTextCtrl* m_tcWindDirection;
		wxTextCtrl* m_tcWaveHeight;
		wxTextCtrl* m_tcWaveDirection;
		wxTextCtrl* m_tcCurrentVelocity;
		wxTextCtrl* m_tcCurrentDirection;
		wxTextCtrl* m_tcWindGust;
		wxTextCtrl* m_tcPressure;
		wxTextCtrl* m_tcPrecipitation;
		wxTextCtrl* m_tcCloud;
		wxTextCtrl* m_tcAirTemperature;
		wxTextCtrl* m_tcSeaTemperature;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnSize( wxSizeEvent& event ) { event.Skip(); }
		virtual void OnPrev( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRecordForecast( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnNext( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnNow( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPlayStop( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTimeline( wxScrollEvent& event ) { event.Skip(); }
		virtual void OnOpenFile( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSettings( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRequest( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCBAny( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxSlider* m_sTimeline;
		wxCheckBox* m_cbWind;
		wxCheckBox* m_cbWave;
		wxCheckBox* m_cbCurrent;
		wxCheckBox* m_cbWindGust;
		wxCheckBox* m_cbPressure;
		wxCheckBox* m_cbPrecipitation;
		wxCheckBox* m_cbCloud;
		wxCheckBox* m_cbAirTemperature;
		wxCheckBox* m_cbSeaTemperature;
		
		GRIBUIDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("GRIB Display Control"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxCLOSE_BOX|wxDIALOG_NO_PARENT|wxRESIZE_BORDER|wxSYSTEM_MENU ); 
		~GRIBUIDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class GribSettingsDialogBase
///////////////////////////////////////////////////////////////////////////////
class GribSettingsDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxSpinCtrl* m_sUpdatesPerSecond;
		wxStaticText* m_tSlicesPerUpdate;
		wxSpinCtrl* m_sSlicesPerUpdate;
		wxStaticText* m_tHourDivider;
		wxChoice* m_cDataType;
		wxChoice* m_cDataUnits;
		wxCheckBox* m_cbBarbedArrows;
		wxStaticText* m_tBarbedRange;
		wxSpinCtrl* m_sBarbedRange;
		wxCheckBox* m_cbIsoBars;
		wxStaticText* m_tIsoBarSpacing;
		wxSpinCtrl* m_sIsoBarSpacing;
		wxCheckBox* m_cbDirectionArrows;
		wxStaticText* m_tDirectionArrowSize;
		wxSpinCtrl* m_sDirectionArrowSize;
		wxCheckBox* m_cbOverlayMap;
		wxStaticText* m_tOverlayColors;
		wxChoice* m_cOverlayColors;
		wxCheckBox* m_cbNumbers;
		wxSpinCtrl* m_sNumbersSpacing;
		wxStaticLine* m_staticline1;
		wxStaticLine* m_staticline2;
		wxStaticLine* m_staticline3;
		wxStaticText* m_staticText24;
		wxSlider* m_sTransparency;
		wxStdDialogButtonSizer* m_sButton;
		wxButton* m_sButtonOK;
		wxButton* m_sButtonApply;
		wxButton* m_sButtonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnIntepolateChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDataTypeChoice( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTransparencyChange( wxScrollEvent& event ) { event.Skip(); }
		virtual void OnApply( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxCheckBox* m_cLoopMode;
		wxCheckBox* m_cInterpolate;
		wxSpinCtrl* m_sHourDivider;
		
		GribSettingsDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Grib Settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~GribSettingsDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class GribPreferencesDialogBase
///////////////////////////////////////////////////////////////////////////////
class GribPreferencesDialogBase : public wxDialog 
{
	private:
	
	protected:
	
	public:
		wxCheckBox* m_cbUseHiDef;
		wxCheckBox* m_cbUseGradualColors;
		wxCheckBox* m_cbCopyFirstCumulativeRecord;
		wxCheckBox* m_cbCopyMissingWaveRecord;
		wxRadioBox* m_rbStartOptions;
		wxRadioBox* m_rbTimeFormat;
		
		GribPreferencesDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Grib Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~GribPreferencesDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class GribRequestSettingBase
///////////////////////////////////////////////////////////////////////////////
class GribRequestSettingBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_tLogin;
		wxStaticText* m_tCode;
		wxStaticText* m_staticText21;
		wxStaticText* m_tWModel;
		wxTextCtrl* m_MailImage;
		wxStaticText* m_tFileSize;
		wxStaticText* m_tLimit;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnTopChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAnyChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveMail( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSendMaiL( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxChoice* m_pMailTo;
		wxChoice* m_pModel;
		wxTextCtrl* m_pLogin;
		wxTextCtrl* m_pCode;
		wxChoice* m_pResolution;
		wxStaticText* m_tResUnit;
		wxChoice* m_pInterval;
		wxChoice* m_pTimeRange;
		wxCheckBox* m_pWind;
		wxCheckBox* m_pPress;
		wxCheckBox* m_pWindGust;
		wxCheckBox* m_pRainfall;
		wxCheckBox* m_pCloudCover;
		wxCheckBox* m_pAirTemp;
		wxCheckBox* m_pSeaTemp;
		wxCheckBox* m_pCurrent;
		wxCheckBox* m_pWaves;
		wxChoice* m_pWModel;
		wxStdDialogButtonSizer* m_rButton;
		wxButton* m_rButtonYes;
		wxButton* m_rButtonApply;
		wxButton* m_rButtonCancel;
		
		GribRequestSettingBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Write and send GRIB request setting"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~GribRequestSettingBase();
	
};

#endif //__GRIBUIDIALOGBASE_H__
