/////////////////////////////////////////////////////////////////////////////
// Name:        scrollingdialog.h
// Purpose:     wxScrollingDialog
// Author:      Julian Smart
// Modified by:
// Created:     2007-12-11
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SCROLLINGDIALOG_H_
#define _WX_SCROLLINGDIALOG_H_

#include "wx/dialog.h"
#include "wx/propdlg.h"

/*!
 * Base class for layout adapters - code that, for example, turns a dialog into a
 * scrolling dialog if there isn't enough screen space. You can derive further
 * adapter classes to do any other kind of adaptation, such as applying a watermark, or adding
 * a help mechanism.
 */

class wxScrollingDialog;
class wxDialogHelper;

class wxDialogLayoutAdapter: public wxObject
{
    DECLARE_CLASS(wxDialogLayoutAdapter)
public:
    wxDialogLayoutAdapter() {}

    /// Override this function to indicate that adaptation should be done
    virtual bool CanDoLayoutAdaptation(wxDialogHelper* dialog) = 0;

    /// Override this function to do the adaptation
    virtual bool DoLayoutAdaptation(wxDialogHelper* dialog) = 0;
};

/*!
 * Standard adapter. Does scrolling adaptation for paged and regular dialogs.
 *
 */

class wxStandardDialogLayoutAdapter: public wxDialogLayoutAdapter
{
    DECLARE_CLASS(wxStandardDialogLayoutAdapter)
public:
    wxStandardDialogLayoutAdapter() {}

// Overrides

    /// Indicate that adaptation should be done
    virtual bool CanDoLayoutAdaptation(wxDialogHelper* dialog);

    /// Do layout adaptation
    virtual bool DoLayoutAdaptation(wxDialogHelper* dialog);

// Implementation

    /// Find a standard or horizontal box sizer
    virtual wxSizer* FindButtonSizer(bool stdButtonSizer, wxDialogHelper* dialog, wxSizer* sizer, int& retBorder, int accumlatedBorder = 0);

    /// Check if this sizer contains standard buttons, and so can be repositioned in the dialog
    virtual bool IsOrdinaryButtonSizer(wxDialogHelper* dialog, wxBoxSizer* sizer);

    /// Check if this is a standard button
    virtual bool IsStandardButton(wxDialogHelper* dialog, wxButton* button);

    /// Find 'loose' main buttons in the existing layout and add them to the standard dialog sizer
    virtual bool FindLooseButtons(wxDialogHelper* dialog, wxStdDialogButtonSizer* buttonSizer, wxSizer* sizer, int& count);

    /// Reparent the controls to the scrolled window, except those in buttonSizer
    virtual void ReparentControls(wxWindow* parent, wxWindow* reparentTo, wxSizer* buttonSizer = NULL);

    /// A function to fit the dialog around its contents, and then adjust for screen size.
    /// If scrolled windows are passed, scrolling is enabled in the required orientation(s).
    virtual bool FitWithScrolling(wxDialog* dialog, wxScrolledWindow* scrolledWindow);
    virtual bool FitWithScrolling(wxDialog* dialog, wxWindowList& windows);

    /// Find whether scrolling will be necessary for the dialog, returning wxVERTICAL, wxHORIZONTAL or both
    virtual int MustScroll(wxDialog* dialog, wxSize& windowSize, wxSize& displaySize);
};

/*!
 * A base class for dialogs that have adaptation. In wxWidgets 3.0, this will not
 * be needed since the new functionality will be implemented in wxDialogBase.
 */

class wxDialogHelper
{
public:

    wxDialogHelper(wxDialog* dialog = NULL) { Init(); m_dialog = dialog; }
    virtual ~wxDialogHelper() {}

    void Init();

    void SetDialog(wxDialog* dialog) { m_dialog = dialog; }
    wxDialog* GetDialog() const { return m_dialog; }

    /// Do the adaptation
    virtual bool DoLayoutAdaptation();

    /// Can we do the adaptation?
    virtual bool CanDoLayoutAdaptation();

    /// Returns a content window if there is one
    virtual wxWindow* GetContentWindow() const { return NULL; }

    /// Add an id to the list of custom button identifiers that should be in the button sizer
    void AddButtonId(wxWindowID id) { m_buttonIds.Add((int) id); }
    wxArrayInt& GetButtonIds() { return m_buttonIds; }

    /// Is this id in the custom button id array?
    bool IsUserButtonId(wxWindowID id) { return (m_buttonIds.Index((int) id) != wxNOT_FOUND); }

// ACCESSORS

    /// Level of adaptation, from none (Level 0) to full (Level 3). To disable adaptation,
    /// set level 0, for example in your dialog constructor. You might
    /// do this if you know that you are displaying on a large screen and you don't want the
    /// dialog changed.
    void SetLayoutAdaptationLevel(int level) { m_layoutAdaptationLevel = level; }

    /// Get level of adaptation
    int GetLayoutAdaptationLevel() const { return m_layoutAdaptationLevel; }

    /// Returns true if the adaptation has been done
    void SetLayoutAdaptationDone(bool adaptationDone) { m_layoutLayoutAdaptationDone = adaptationDone; }
    bool GetLayoutAdaptationDone() const { return m_layoutLayoutAdaptationDone; }

    /// Set layout adapter class, returning old adapter
    static wxDialogLayoutAdapter* SetLayoutAdapter(wxDialogLayoutAdapter* adapter);
    static wxDialogLayoutAdapter* GetLayoutAdapter() { return sm_layoutAdapter; }

    /// Global switch for layout adaptation
    static bool GetLayoutAdaptation() { return sm_layoutAdaptation; }
    static void SetLayoutAdaptation(bool enable) { sm_layoutAdaptation = enable; }

protected:

    wxDialog*                           m_dialog;
    bool                                m_layoutLayoutAdaptationDone;
    wxArrayInt                          m_buttonIds;
    int                                 m_layoutAdaptationLevel;
    static wxDialogLayoutAdapter*       sm_layoutAdapter;
    static bool                         sm_layoutAdaptation;
};

/*!
 * A class that makes its content scroll if necessary
 */

class wxScrollingDialog: public wxDialog, public wxDialogHelper
{
    DECLARE_CLASS(wxScrollingDialog)
public:

    wxScrollingDialog() { Init(); }
    wxScrollingDialog(wxWindow *parent,
             int id = wxID_ANY,
             const wxString& title = wxEmptyString,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_DIALOG_STYLE)
    {
        Init();
        Create(parent, id, title, pos, size, style);
    }
    bool Create(wxWindow *parent,
             int id = wxID_ANY,
             const wxString& title = wxEmptyString,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxDEFAULT_DIALOG_STYLE);

    void Init();

    /// Override Show to rejig the control and sizer hierarchy if necessary
    virtual bool Show(bool show = true);

    /// Override ShowModal to rejig the control and sizer hierarchy if necessary
    virtual int ShowModal();
};

/*!
 * A wxPropertySheetDialog class that makes its content scroll if necessary.
 */

class wxScrollingPropertySheetDialog : public wxPropertySheetDialog, public wxDialogHelper
{
public:
    wxScrollingPropertySheetDialog() : wxPropertySheetDialog() { Init(); }

    wxScrollingPropertySheetDialog(wxWindow* parent, wxWindowID id,
                       const wxString& title,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& sz = wxDefaultSize,
                       long style = wxDEFAULT_DIALOG_STYLE,
                       const wxString& name = wxDialogNameStr)
    {
        Init();
        Create(parent, id, title, pos, sz, style, name);
    }

//// Accessors

    /// Returns the content window
    virtual wxWindow* GetContentWindow() const;

/// Operations

    /// Override Show to rejig the control and sizer hierarchy if necessary
    virtual bool Show(bool show = true);

    /// Override ShowModal to rejig the control and sizer hierarchy if necessary
    virtual int ShowModal();

private:
    void Init();

protected:

    DECLARE_DYNAMIC_CLASS(wxScrollingPropertySheetDialog)
};

#endif
 // _WX_SCROLLINGDIALOG_H_

