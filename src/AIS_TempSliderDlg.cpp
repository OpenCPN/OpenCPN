/***************************************************************
 * Name:      AIS_weight_settingsMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Dirk ()
 * Created:   2015-12-13
 * Copyright: Dirk ()
 * License:
 **************************************************************/

#include "AIS_TempSliderDlg.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(AIS_weight_settingsFrame)
#include <wx/string.h>
#include <wx/intl.h>
//*)
extern int                     g_ScaledNumWeightSOG;
extern int                     g_ScaledNumWeightCPA;
extern int                     g_ScaledNumWeightTCPA;
extern int                     g_ScaledNumWeightRange;
extern int                     g_ScaledNumWeightClassB;
extern int                     g_ScaledSizeMinimal;

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(AIS_weight_settingsFrame)
const long AIS_weight_settingsFrame::ID_SLIDER1 = wxNewId();
const long AIS_weight_settingsFrame::ID_STATICTEXT1 = wxNewId();
const long AIS_weight_settingsFrame::ID_SLIDER3 = wxNewId();
const long AIS_weight_settingsFrame::ID_STATICTEXT2 = wxNewId();
const long AIS_weight_settingsFrame::ID_SLIDER2 = wxNewId();
const long AIS_weight_settingsFrame::ID_STATICTEXT3 = wxNewId();
const long AIS_weight_settingsFrame::ID_SLIDER4 = wxNewId();
const long AIS_weight_settingsFrame::ID_STATICTEXT4 = wxNewId();
const long AIS_weight_settingsFrame::ID_SLIDER5 = wxNewId();
const long AIS_weight_settingsFrame::ID_STATICTEXT5 = wxNewId();
//*)

BEGIN_EVENT_TABLE(AIS_weight_settingsFrame,wxDialog)
    //(*EventTable(AIS_weight_settingsFrame)
    //*)
END_EVENT_TABLE()

AIS_weight_settingsFrame::AIS_weight_settingsFrame(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(AIS_weight_settingsFrame)
    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP|wxDEFAULT_DIALOG_STYLE, _T("id"));
    SetClientSize(wxSize(227,39));
    SetMinSize(wxSize(-1,-1));
    SetMaxSize(wxSize(-1,-1));
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    SOGSlider = new wxSlider(this, ID_SLIDER1, 0, 0, 100, wxDefaultPosition, wxSize(100,25), 0, wxDefaultValidator, _T("ID_SLIDER1"));
    FlexGridSizer1->Add(SOGSlider, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("SOG         "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CPASlider = new wxSlider(this, ID_SLIDER3, 0, 0, 100, wxDefaultPosition, wxSize(100,25), 0, wxDefaultValidator, _T("ID_SLIDER3"));
    FlexGridSizer1->Add(CPASlider, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("CPA"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TCPASlider = new wxSlider(this, ID_SLIDER2, 0, 0, 100, wxDefaultPosition, wxSize(100,25), 0, wxDefaultValidator, _T("ID_SLIDER2"));
    FlexGridSizer1->Add(TCPASlider, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("TCPA"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RangeSlider = new wxSlider(this, ID_SLIDER4, 0, 0, 100, wxDefaultPosition, wxSize(100,25), 0, wxDefaultValidator, _T("ID_SLIDER4"));
    FlexGridSizer1->Add(RangeSlider, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Range"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SizeSlider = new wxSlider(this, ID_SLIDER5, 0, 0, 100, wxDefaultPosition, wxSize(100,25), 0, wxDefaultValidator, _T("ID_SLIDER5"));
    FlexGridSizer1->Add(SizeSlider, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
  
SOGSlider->SetValue( g_ScaledNumWeightSOG);
CPASlider->SetValue( g_ScaledNumWeightCPA);
TCPASlider->SetValue( g_ScaledNumWeightTCPA) ;
RangeSlider->SetValue( g_ScaledNumWeightRange) ;
SizeSlider->SetValue( g_ScaledNumWeightClassB) ;

    Connect(ID_SLIDER1,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&AIS_weight_settingsFrame::OnSliderCmdScrollChanged);
    Connect(ID_SLIDER3,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&AIS_weight_settingsFrame::OnSliderCmdScrollChanged);
    Connect(ID_SLIDER2,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&AIS_weight_settingsFrame::OnSliderCmdScrollChanged);
    Connect(ID_SLIDER4,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&AIS_weight_settingsFrame::OnSliderCmdScrollChanged);
    Connect(ID_SLIDER5,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&AIS_weight_settingsFrame::OnSliderCmdScrollChanged);
    Connect(ID_SLIDER1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&AIS_weight_settingsFrame::OnSliderCmdScrollChanged);
    Connect(ID_SLIDER2,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&AIS_weight_settingsFrame::OnSliderCmdScrollChanged);
    Connect(ID_SLIDER3,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&AIS_weight_settingsFrame::OnSliderCmdScrollChanged);
    Connect(ID_SLIDER4,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&AIS_weight_settingsFrame::OnSliderCmdScrollChanged);
    Connect(ID_SLIDER5,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&AIS_weight_settingsFrame::OnSliderCmdScrollChanged);
        //*)
}

AIS_weight_settingsFrame::~AIS_weight_settingsFrame()
{
    //(*Destroy(AIS_weight_settingsFrame)
    //*)
}


void AIS_weight_settingsFrame::OnSliderCmdScrollChanged(wxScrollEvent& event)
{
    g_ScaledNumWeightSOG =  SOGSlider->GetValue();
    wxString SogStr = wxString::Format(wxT("SOG:%i"), g_ScaledNumWeightSOG);
    StaticText1->SetLabel(SogStr);
    g_ScaledNumWeightCPA = CPASlider->GetValue() ;
    wxString CpaStr = wxString::Format(wxT("CPA:%i"), g_ScaledNumWeightCPA);
    StaticText2->SetLabel(CpaStr);
    g_ScaledNumWeightTCPA = TCPASlider->GetValue() ;
    wxString TcpaStr = wxString::Format(wxT("TCPA:%i"), g_ScaledNumWeightTCPA);
    StaticText3->SetLabel(TcpaStr);
    g_ScaledNumWeightRange = RangeSlider->GetValue() ;
    wxString RangeStr = wxString::Format(wxT("Range:%i"), g_ScaledNumWeightRange);
    StaticText4->SetLabel(RangeStr);
    g_ScaledNumWeightClassB = SizeSlider->GetValue() ;
    wxString SizeStr = wxString::Format(wxT("Size:%i"), g_ScaledNumWeightClassB);
    StaticText5->SetLabel(SizeStr);
}
