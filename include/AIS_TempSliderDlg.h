/***************************************************************
 * Name:      AIS_weight_settingsMain.h
 * Purpose:   Defines Application Frame
 * Author:    Dirk ()
 * Created:   2015-12-13
 * Copyright: Dirk ()
 * License:
 **************************************************************/

#ifndef AIS_WEIGHT_SETTINGSMAIN_H
#define AIS_WEIGHT_SETTINGSMAIN_H

//(*Headers(AIS_weight_settingsFrame)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/frame.h>
#include <wx/stattext.h>
//*)

class AIS_weight_settingsFrame: public wxDialog
{
    public:

        AIS_weight_settingsFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~AIS_weight_settingsFrame();

    private:

        //(*Handlers(AIS_weight_settingsFrame)

        void OnSliderCmdScrollChanged(wxScrollEvent& event);
        //*)

        //(*Identifiers(AIS_weight_settingsFrame)
        static const long ID_SLIDER1;
        static const long ID_STATICTEXT1;
        static const long ID_SLIDER3;
        static const long ID_STATICTEXT2;
        static const long ID_SLIDER2;
        static const long ID_STATICTEXT3;
        static const long ID_SLIDER4;
        static const long ID_STATICTEXT4;
        static const long ID_SLIDER5;
        static const long ID_STATICTEXT5;
        //*)

        //(*Declarations(AIS_weight_settingsFrame)
        wxSlider* TCPASlider;
        wxSlider* RangeSlider;
        wxStaticText* StaticText1;
        wxStaticText* StaticText3;
        wxSlider* CPASlider;
        wxSlider* SOGSlider;
        wxStaticText* StaticText4;
        wxStaticText* StaticText5;
        wxStaticText* StaticText2;
        wxSlider* SizeSlider;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // AIS_WEIGHT_SETTINGSMAIN_H
