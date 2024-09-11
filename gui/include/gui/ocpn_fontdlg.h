/////////////////////////////////////////////////////////////////////////////
// Name:        ocpn_fontdlg.h
// Purpose:     Generic font dialog for OpenCPN
// Author:      Julian Smart
// Modified by: David S Register
// Created:     04/01/98
// Copyright:   (c) Julian Smart, David S Register
// Licence:     wxWindows licence

#ifndef _OCPN_GENERIC_FONTDLGG_H
#define _OCPN_GENERIC_FONTDLGG_H

#include "wx/gdicmn.h"
#include "wx/font.h"

#define USE_SPINCTRL_FOR_POINT_SIZE 0

/*
 * FONT DIALOG
 */

class WXDLLIMPEXP_FWD_CORE wxChoice;
class WXDLLIMPEXP_FWD_CORE wxText;
class WXDLLIMPEXP_FWD_CORE wxCheckBox;
class WXDLLIMPEXP_FWD_CORE OCPNFontPreviewer;

enum {
  wxID_FONT_UNDERLINE = 3000,
  wxID_FONT_STYLE,
  wxID_FONT_WEIGHT,
  wxID_FONT_FAMILY,
  wxID_FONT_COLOUR,
  wxID_FONT_SIZE
};

class WXDLLIMPEXP_CORE ocpnGenericFontDialog : public wxFontDialogBase {
public:
  ocpnGenericFontDialog() { Init(); }
  ocpnGenericFontDialog(wxWindow *parent) : wxFontDialogBase(parent) { Init(); }
  ocpnGenericFontDialog(wxWindow *parent, const wxFontData &data)
      : wxFontDialogBase(parent, data) {
    Init();
  }
  virtual ~ocpnGenericFontDialog();

  virtual int ShowModal() override;

  // Internal functions
  void OnCloseWindow(wxCloseEvent &event);

  virtual void CreateWidgets();
  virtual void InitializeFont();

  void OnChangeFont(wxCommandEvent &event);

#if USE_SPINCTRL_FOR_POINT_SIZE
  void OnChangeSize(wxSpinEvent &event);
#endif

protected:
  virtual bool DoCreate(wxWindow *parent) override;

private:
  // common part of all ctors
  void Init();

  void DoChangeFont();

  wxFont m_dialogFont;

  wxChoice *m_familyChoice;
  wxChoice *m_styleChoice;
  wxChoice *m_weightChoice;
  wxChoice *m_colourChoice;
  wxCheckBox *m_underLineCheckBox;

#if USE_SPINCTRL_FOR_POINT_SIZE
  wxSpinCtrl *m_pointSizeSpin;
#else
  wxChoice *m_pointSizeChoice;
#endif

  OCPNFontPreviewer *m_previewer;
  bool m_useEvents;

  //  static bool fontDialogCancelled;
  wxDECLARE_EVENT_TABLE();
  wxDECLARE_DYNAMIC_CLASS(ocpnGenericFontDialog);
};

#endif  // _OCPN_GENERIC_FONTDLGG_H
