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
#include <wx/string.h>
#include <wx/filepicker.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/tglbtn.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include "GribRecordTree.h"
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>

///////////////////////////////////////////////////////////////////////////

#define ID_GRIBDIR 1000
#define ID_CONFIG 1001
#define ID_PLAYSTOP 1002
#define ID_TIMELINE 1003
#define ID_GRIBRECORDTREE 1004
#define ID_CB_WIND_SPEED 1005
#define ID_CB_SIG_WAVE_HEIGHT 1006
#define ID_CB_CURRENT_VELOCITY 1007
#define ID_CB_PRESSURE 1008
#define ID_CB_SEA_TEMPERATURE 1009

///////////////////////////////////////////////////////////////////////////////
/// Class GRIBUIDialogBase
///////////////////////////////////////////////////////////////////////////////
class GRIBUIDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxDirPickerCtrl* m_dirPicker;
		wxButton* m_bConfig;
		wxToggleButton* m_tbPlayStop;
		GribRecordTree *m_pRecordTree;
		wxTextCtrl* m_tcWindSpeed;
		wxTextCtrl* m_tcWindDirection;
		wxTextCtrl* m_tcWaveHeight;
		wxTextCtrl* m_tcWaveDirection;
		wxTextCtrl* m_tcCurrentVelocity;
		wxTextCtrl* m_tcCurrentDirection;
		wxTextCtrl* m_tcPressure;
		wxTextCtrl* m_tcSeaTemperature;
	
	public:
		wxSlider* m_sTimeline;
		wxStaticText* m_stDateTime;
		wxCheckBox* m_cbWind;
		wxCheckBox* m_cbWave;
		wxCheckBox* m_cbCurrent;
		wxCheckBox* m_cbPressure;
		wxCheckBox* m_cbSeaTemperature;
		
		GRIBUIDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("GRIB Display Control"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,520 ), long style = wxCAPTION|wxDIALOG_NO_PARENT|wxRESIZE_BORDER|wxSYSTEM_MENU ); 
		~GRIBUIDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class GRIBConfigDialogBase
///////////////////////////////////////////////////////////////////////////////
class GRIBConfigDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText5;
		wxSpinCtrl* m_sSlicesPerUpdate;
		wxStaticText* m_staticText9;
		wxSpinCtrl* m_sUpdatesPerSecond;
		wxStaticText* m_staticText4;
		wxChoice* m_cDataType;
		wxStaticText* m_staticText12;
		wxChoice* m_cDataUnits;
		wxCheckBox* m_cbBarbedArrows;
		wxStaticText* m_staticText10;
		wxStaticText* m_staticText11;
		wxCheckBox* m_cbIsoBars;
		wxStaticText* m_staticText6;
		wxSpinCtrl* m_sIsoBarSpacing;
		wxCheckBox* m_cbDirectionArrows;
		wxStaticText* m_staticText51;
		wxSpinCtrl* m_sDirectionArrowSize;
		wxCheckBox* m_cbOverlayMap;
		wxStaticText* m_staticText7;
		wxChoice* m_cOverlayColors;
		wxCheckBox* m_cbNumbers;
		wxStaticText* m_staticText8;
		wxSpinCtrl* m_sNumbersSpacing;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnDataTypeChoice( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxCheckBox* m_cInterpolate;
		wxCheckBox* m_cLoopMode;
		wxSpinCtrl* m_sHourDivider;
		
		GRIBConfigDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("GRIB Config"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~GRIBConfigDialogBase();
	
};

#endif //__GRIBUIDIALOGBASE_H__
