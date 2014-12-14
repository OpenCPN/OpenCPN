///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  6 2014)
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
#include <wx/choice.h>
#include <wx/slider.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/dialog.h>
#include <wx/spinctrl.h>
#include <wx/radiobox.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/scrolwin.h>
#include <wx/grid.h>

///////////////////////////////////////////////////////////////////////////

#define ID_TIMELINE 1000
#define ID_CB_WIND 1001
#define ID_CB_WAVES 1002
#define ID_CB_CURRENT 1003
#define ID_CB_WIND_GUSTS 1004
#define ID_CB_PRESSURE 1005
#define ID_CB_RAINFALL 1006
#define ID_CB_CLOUD_COVER 1007
#define ID_CB_AIR_TEMP 1008
#define ID_CB_SEA_TEMP 1009
#define ID_CB_CAPE 1010
#define MAXLAT 1011
#define MAXLON 1012
#define MINLAT 1013
#define MINLON 1014

///////////////////////////////////////////////////////////////////////////////
/// Class GRIBUIDialogBase
///////////////////////////////////////////////////////////////////////////////
class GRIBUIDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxFlexGridSizer* m_fgTrackingDisplay;
		wxBitmapButton* m_bpPrev;
		wxChoice* m_cRecordForecast;
		wxBitmapButton* m_bpNext;
		wxBitmapButton* m_bpNow;
		wxBitmapButton* m_bpZoomToCenter;
		wxBitmapButton* m_bpPlay;
		wxBitmapButton* m_bpOpenFile;
		wxBitmapButton* m_bpSettings;
		wxBitmapButton* m_bpRequest;
		wxFlexGridSizer* m_fgTrackingControls;
		wxFlexGridSizer* m_fcAltitude;
		wxChoice* m_cbAltitude;
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
		wxTextCtrl* m_tcCAPE;
		wxStaticText* m_stAltitudeText;
		wxTextCtrl* m_tcAltitude;
		wxTextCtrl* m_tcTemp;
		wxTextCtrl* m_tcRelHumid;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnSize( wxSizeEvent& event ) { event.Skip(); }
		virtual void OnPrev( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRecordForecast( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnNext( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnNow( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnZoomToCenterClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPlayStop( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTimeline( wxScrollEvent& event ) { event.Skip(); }
		virtual void OnOpenFile( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSettings( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRequest( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAltitudeChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCBAny( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMouseEvent( wxMouseEvent& event ) { event.Skip(); }
		
	
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
		wxCheckBox* m_cbCAPE;
		
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
		wxStaticText* m_staticText26;
		wxChoice* m_cLoopStartPoint;
		wxSpinCtrl* m_sUpdatesPerSecond;
		wxStaticText* m_tSlicesPerUpdate;
		wxChoice* m_cDataType;
		wxChoice* m_cDataUnits;
		wxCheckBox* m_cbBarbedArrows;
		wxChoice* m_cBarbedColours;
		wxCheckBox* m_cBarbedVisibility;
		wxCheckBox* m_cbIsoBars;
		wxFlexGridSizer* m_fIsoBarSpacing;
		wxSpinCtrl* m_sIsoBarSpacing;
		wxFlexGridSizer* m_fIsoBarVisibility;
		wxCheckBox* m_sIsoBarVisibility;
		wxCheckBox* m_cbDirectionArrows;
		wxChoice* m_cDirectionArrowForm;
		wxChoice* m_cDirectionArrowSize;
		wxCheckBox* m_cbOverlayMap;
		wxStaticText* m_tOverlayColors;
		wxChoice* m_cOverlayColors;
		wxCheckBox* m_cbNumbers;
		wxStaticText* m_ctNumbers;
		wxSpinCtrl* m_sNumbersSpacing;
		wxCheckBox* m_cbParticles;
		wxStaticText* m_ctParticles;
		wxSlider* m_sParticleDensity;
		wxStaticText* m_staticText24;
		wxSlider* m_sTransparency;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnIntepolateChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDataTypeChoice( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTransparencyChange( wxScrollEvent& event ) { event.Skip(); }
		virtual void OnApply( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxCheckBox* m_cLoopMode;
		wxCheckBox* m_cInterpolate;
		wxChoice* m_sSlicesPerUpdate;
		wxStdDialogButtonSizer* m_sButton;
		wxButton* m_sButtonOK;
		wxButton* m_sButtonApply;
		wxButton* m_sButtonCancel;
		
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
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnStartOptionChange( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxCheckBox* m_cbUseHiDef;
		wxCheckBox* m_cbUseGradualColors;
		wxCheckBox* m_cbCopyFirstCumulativeRecord;
		wxCheckBox* m_cbCopyMissingWaveRecord;
		wxRadioBox* m_rbLoadOptions;
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
		wxFlexGridSizer* m_fgScrollSizer;
		wxFlexGridSizer* m_pSenderSizer;
		wxTextCtrl* m_pSenderAddress;
		wxChoice* m_pMailTo;
		wxChoice* m_pModel;
		wxCheckBox* m_cMovingGribEnabled;
		wxFlexGridSizer* m_fgMovingParams;
		wxSpinCtrl* m_sMovingSpeed;
		wxSpinCtrl* m_sMovingCourse;
		wxStaticText* m_sCourseUnit;
		wxFlexGridSizer* m_fgLog;
		wxTextCtrl* m_pLogin;
		wxTextCtrl* m_pCode;
		wxChoice* m_pResolution;
		wxStaticText* m_tResUnit;
		wxChoice* m_pInterval;
		wxChoice* m_pTimeRange;
		wxStaticText* m_staticText21;
		wxCheckBox* m_cManualZoneSel;
		wxToggleButton* m_toggleSelection;
		wxFlexGridSizer* fgZoneCoordinatesSizer;
		wxSpinCtrl* m_spMaxLat;
		wxStaticText* m_stMaxLatNS;
		wxStaticText* m_staticText36;
		wxSpinCtrl* m_spMaxLon;
		wxStaticText* m_stMaxLonEW;
		wxSpinCtrl* m_spMinLat;
		wxStaticText* m_stMinLatNS;
		wxSpinCtrl* m_spMinLon;
		wxStaticText* m_stMinLonEW;
		wxCheckBox* m_pWind;
		wxCheckBox* m_pPress;
		wxCheckBox* m_pWindGust;
		wxCheckBox* m_pRainfall;
		wxCheckBox* m_pCloudCover;
		wxCheckBox* m_pAirTemp;
		wxCheckBox* m_pSeaTemp;
		wxCheckBox* m_pCurrent;
		wxCheckBox* m_pCAPE;
		wxCheckBox* m_pWaves;
		wxChoice* m_pWModel;
		wxCheckBox* m_pAltitudeData;
		wxFlexGridSizer* m_fgAltitudeData;
		wxCheckBox* m_p850hpa;
		wxCheckBox* m_p700hpa;
		wxCheckBox* m_p500hpa;
		wxCheckBox* m_p300hpa;
		wxTextCtrl* m_MailImage;
		wxFlexGridSizer* m_fgFixedSizer;
		wxStaticText* m_tFileSize;
		wxStaticText* m_tLimit;
		wxStdDialogButtonSizer* m_rButton;
		wxButton* m_rButtonYes;
		wxButton* m_rButtonApply;
		wxButton* m_rButtonCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnTopChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMovingClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAnyChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTimeRangeChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnZoneSelectionModeChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnTooggleSelection( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCoordinatesChange( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnSaveMail( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSendMaiL( wxCommandEvent& event ) { event.Skip(); }


	public:
		wxScrolledWindow* m_sScrolledDialog;

		GribRequestSettingBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Write and send eMail request"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE );
		~GribRequestSettingBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class GRIBTableBase
///////////////////////////////////////////////////////////////////////////////
class GRIBTableBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_pPositionText;
		wxStaticText* m_pCursorPosition;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnClick( wxGridEvent& event ) { event.Skip(); }
		virtual void OnRangeClick( wxGridRangeSelectEvent& event ) { event.Skip(); }
		virtual void OnOKButton( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxGrid* m_pGribTable;
		wxStdDialogButtonSizer* m_pButtonTable;
		wxButton* m_pButtonTableOK;
		
		GRIBTableBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Grib Data Table"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER ); 
		~GRIBTableBase();
	
};

#endif //__GRIBUIDIALOGBASE_H__
