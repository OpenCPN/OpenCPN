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
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/slider.h>
#include "GribRecordTree.h"
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/choice.h>

///////////////////////////////////////////////////////////////////////////

#define ID_GRIBDIR 1000
#define ID_TIMELINE 1001
#define ID_GRIBRECORDTREE 1002
#define ID_CB_WIND_SPEED 1003
#define ID_CB_SIG_WAVE_HEIGHT 1004
#define ID_CB_CURRENT_VELOCITY 1005
#define ID_CB_PRESSURE 1006
#define ID_CB_SEA_TEMPERATURE 1007

///////////////////////////////////////////////////////////////////////////////
/// Class GRIBUIDialogBase
///////////////////////////////////////////////////////////////////////////////
class GRIBUIDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxDirPickerCtrl* m_dirPicker;
		wxButton* m_bConfig;
		wxBitmapButton* m_bbPlayPause;
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
		wxCheckBox* m_cbWind;
		wxCheckBox* m_cbWave;
		wxCheckBox* m_cbCurrent;
		wxCheckBox* m_cbPressure;
		wxCheckBox* m_cbSeaTemperature;
		
		GRIBUIDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("GRIB Display Control"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,520 ), long style = wxCAPTION|wxDIALOG_NO_PARENT|wxRESIZE_BORDER|wxSYSTEM_MENU ); 
		~GRIBUIDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class GRIBConfigDialog
///////////////////////////////////////////////////////////////////////////////
class GRIBConfigDialog : public wxDialog 
{
	private:
	
	protected:
		wxCheckBox* m_checkBox10;
		wxCheckBox* m_checkBox11;
		wxStaticText* m_staticText5;
		wxSlider* m_slider2;
		wxChoice* m_cDataType;
		wxStaticText* m_staticText12;
		wxChoice* m_cDataUnits;
		wxCheckBox* m_cbDirectionArrows;
		wxCheckBox* m_cbIsoBars;
		wxCheckBox* m_cbOverlayMap;
		wxCheckBox* m_cbNumbers;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
	
	public:
		
		GRIBConfigDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("GRIB Config"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~GRIBConfigDialog();
	
};

#endif //__GRIBUIDIALOGBASE_H__
