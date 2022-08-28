/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  X11 font picker widget
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

#ifndef _X11_FONT_PICKER_H__
#define _X11_FONT_PICKER_H__

#include <wx/arrstr.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/event.h>
#include <wx/fontdata.h>
#include <wx/fontdlg.h>
#include <wx/window.h>

class WXDLLEXPORT MyFontPreviewer;

class WXDLLEXPORT X11FontPicker : public wxFontDialogBase {
public:
  X11FontPicker() { Init(); }
  X11FontPicker(wxWindow *parent, const wxFontData &data)
      : wxFontDialogBase(parent, data) {
    Init();
  }
  virtual ~X11FontPicker();

  virtual int ShowModal();

  // deprecated, for backwards compatibility only
  //            X11FontPicker(wxWindow *parent, const wxFontData *data)
  //      : wxFontDialogBase(parent, data) { Init(); }

  // Internal functions
  void OnCloseWindow(wxCloseEvent &event);

  virtual void CreateWidgets();
  virtual void InitializeFont();

  void OnChangeFont(wxCommandEvent &event);
  void OnChangeFace(wxCommandEvent &event);

protected:
  // common part of all ctors
  void Init();

  virtual bool DoCreate(wxWindow *parent);
  void InitializeAllAvailableFonts();
  void SetChoiceOptionsFromFacename(const wxString &facename);
  void DoFontChange(void);

  wxFont dialogFont;

  wxChoice *familyChoice;
  wxChoice *styleChoice;
  wxChoice *weightChoice;
  wxChoice *colourChoice;
  wxCheckBox *underLineCheckBox;
  wxChoice *pointSizeChoice;

  MyFontPreviewer *m_previewer;
  bool m_useEvents;

  wxArrayString *pFaceNameArray;

  wxFont *pPreviewFont;

  //  static bool fontDialogCancelled;
  DECLARE_EVENT_TABLE()
  DECLARE_DYNAMIC_CLASS(X11FontPicker)
};

#endif
