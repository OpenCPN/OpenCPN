/////////////////////////////////////////////////////////////////////////////
// Name:        scrollingdialog.cpp
// Purpose:     wxScrollingDialog
// Author:      Julian Smart
// Modified by:
// Created:     2007-12-11
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wx.h"
#include "wx/module.h"
#include "wx/display.h"
#include "wx/bookctrl.h"

#include "scrollingdialog.h"

// Allow for caption size on wxWidgets < 2.9
#if defined(__WXGTK__) && !wxCHECK_VERSION(2,9,0)
#define wxEXTRA_DIALOG_HEIGHT 30
#else
#define wxEXTRA_DIALOG_HEIGHT 0
#endif

IMPLEMENT_CLASS(wxDialogLayoutAdapter, wxObject)

/*!
 * Dialog helper. This contains the extra members that in wxWidgets 3.0 will be
 * in wxDialogBase.
 */

wxDialogLayoutAdapter* wxDialogHelper::sm_layoutAdapter = NULL;
bool wxDialogHelper::sm_layoutAdaptation = true;

void wxDialogHelper::Init()
{
    m_layoutAdaptationLevel = 3;
    m_layoutLayoutAdaptationDone = FALSE;
}

/// Do the adaptation
bool wxDialogHelper::DoLayoutAdaptation()
{
    if (GetLayoutAdapter())
        return GetLayoutAdapter()->DoLayoutAdaptation(this);
    else
        return false;
}

/// Can we do the adaptation?
bool wxDialogHelper::CanDoLayoutAdaptation()
{
    return (GetLayoutAdaptation() && !m_layoutLayoutAdaptationDone && GetLayoutAdaptationLevel() != 0 && GetLayoutAdapter() != NULL && GetLayoutAdapter()->CanDoLayoutAdaptation(this));
}

/// Set scrolling adapter class, returning old adapter
wxDialogLayoutAdapter* wxDialogHelper::SetLayoutAdapter(wxDialogLayoutAdapter* adapter)
{
    wxDialogLayoutAdapter* oldLayoutAdapter = sm_layoutAdapter;
    sm_layoutAdapter = adapter;
    return oldLayoutAdapter;
}

/*!
 * Standard adapter
 */

IMPLEMENT_CLASS(wxStandardDialogLayoutAdapter, wxDialogLayoutAdapter)

/// Indicate that adaptation should be done
bool wxStandardDialogLayoutAdapter::CanDoLayoutAdaptation(wxDialogHelper* dialog)
{
    if (dialog->GetDialog()->GetSizer())
    {
        wxSize windowSize, displaySize;
        return MustScroll(dialog->GetDialog(), windowSize, displaySize) != 0;
    }
    else
        return false;
}

bool wxStandardDialogLayoutAdapter::DoLayoutAdaptation(wxDialogHelper* dialog)
{
    if (dialog->GetDialog()->GetSizer())
    {
        // The wxRTTI is wrong for wxNotebook in < 2.8.8 and 2.9, so use dynamic_cast instead
#if !wxCHECK_VERSION(2,8,8) || (wxCHECK_VERSION(2,9,0) && !wxCHECK_VERSION(3,0,0))
        wxBookCtrlBase* bookContentWindow = dynamic_cast<wxBookCtrlBase*>(dialog->GetContentWindow());
#else
        wxBookCtrlBase* bookContentWindow = wxDynamicCast(dialog->GetContentWindow(), wxBookCtrlBase);
#endif

        if (bookContentWindow)
        {
            // If we have a book control, make all the pages (that use sizers) scrollable
            wxWindowList windows;
            for (size_t i = 0; i < bookContentWindow->GetPageCount(); i++)
            {
                wxWindow* page = bookContentWindow->GetPage(i);

                wxScrolledWindow* scrolledWindow = wxDynamicCast(page, wxScrolledWindow);
                if (scrolledWindow)
                    windows.Append(scrolledWindow);
                else if (!scrolledWindow && page->GetSizer())
                {
                    // Create a scrolled window and reparent
                    scrolledWindow = new wxScrolledWindow(page, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxVSCROLL|wxHSCROLL|wxBORDER_NONE);
                    wxSizer* oldSizer = page->GetSizer();

                    wxSizer* newSizer = new wxBoxSizer(wxVERTICAL);
                    newSizer->Add(scrolledWindow,1, wxEXPAND, 0);

                    page->SetSizer(newSizer, false /* don't delete the old sizer */);

                    scrolledWindow->SetSizer(oldSizer);

                    ReparentControls(page, scrolledWindow, NULL);

                    windows.Append(scrolledWindow);
                }
            }

            FitWithScrolling(dialog->GetDialog(), windows);
        }
        else
        {
            // If we have an arbitrary dialog, create a scrolling area for the main content, and a button sizer
            // for the main buttons.
            wxScrolledWindow* scrolledWindow = new wxScrolledWindow(dialog->GetDialog(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxVSCROLL|wxHSCROLL|wxBORDER_NONE);

            int buttonSizerBorder = 0;

            // First try to find a wxStdDialogButtonSizer
            wxSizer* buttonSizer = FindButtonSizer(true /* find std button sizer */, dialog, dialog->GetDialog()->GetSizer(), buttonSizerBorder);

            // Next try to find a wxBoxSizer containing the controls
            if (!buttonSizer && dialog->GetLayoutAdaptationLevel() > 1)
                buttonSizer = FindButtonSizer(false /* find ordinary sizer */, dialog, dialog->GetDialog()->GetSizer(), buttonSizerBorder);

            // If we still don't have a button sizer, collect any 'loose' buttons in the layout
            if (!buttonSizer && dialog->GetLayoutAdaptationLevel() > 2)
            {
                int count = 0;
                wxStdDialogButtonSizer* stdButtonSizer = new wxStdDialogButtonSizer;
                buttonSizer = stdButtonSizer;

                FindLooseButtons(dialog, stdButtonSizer, dialog->GetDialog()->GetSizer(), count);
                if (count > 0)
                    stdButtonSizer->Realize();
                else
                {
                    delete buttonSizer;
                    buttonSizer = NULL;
                }
            }

            if (buttonSizerBorder == 0)
                buttonSizerBorder = 5;

            ReparentControls(dialog->GetDialog(), scrolledWindow, buttonSizer);

            wxBoxSizer* newTopSizer = new wxBoxSizer(wxVERTICAL);
            wxSizer* oldSizer = dialog->GetDialog()->GetSizer();

            dialog->GetDialog()->SetSizer(newTopSizer, false /* don't delete old sizer */);

            newTopSizer->Add(scrolledWindow, 1, wxEXPAND|wxALL, 0);
            if (buttonSizer)
                newTopSizer->Add(buttonSizer, 0, wxEXPAND|wxALL, buttonSizerBorder);

            scrolledWindow->SetSizer(oldSizer);

            FitWithScrolling(dialog->GetDialog(), scrolledWindow);
        }
    }

    dialog->SetLayoutAdaptationDone(true);
    return true;
}

/// Find and remove the button sizer, if any
wxSizer* wxStandardDialogLayoutAdapter::FindButtonSizer(bool stdButtonSizer, wxDialogHelper* dialog, wxSizer* sizer, int& retBorder, int accumlatedBorder)
{
    for ( wxSizerItemList::compatibility_iterator node = sizer->GetChildren().GetFirst();
          node; node = node->GetNext() )
    {
        wxSizerItem *item = node->GetData();
        wxSizer *childSizer = item->GetSizer();

        if ( childSizer )
        {
            int newBorder = accumlatedBorder;
            if (item->GetFlag() & wxALL)
                newBorder += item->GetBorder();

            if (stdButtonSizer) // find wxStdDialogButtonSizer
            {
                wxStdDialogButtonSizer* buttonSizer = wxDynamicCast(childSizer, wxStdDialogButtonSizer);
                if (buttonSizer)
                {
                    sizer->Detach(childSizer);
                    retBorder = newBorder;
                    return buttonSizer;
                }
            }
            else // find a horizontal box sizer containing standard buttons
            {
                wxBoxSizer* buttonSizer = wxDynamicCast(childSizer, wxBoxSizer);
                if (buttonSizer && IsOrdinaryButtonSizer(dialog, buttonSizer))
                {
                    sizer->Detach(childSizer);
                    retBorder = newBorder;
                    return buttonSizer;
                }
            }

            wxSizer* s = FindButtonSizer(stdButtonSizer, dialog, childSizer, retBorder, newBorder);
            if (s)
                return s;
        }
    }
    return NULL;
}

/// Check if this sizer contains standard buttons, and so can be repositioned in the dialog
bool wxStandardDialogLayoutAdapter::IsOrdinaryButtonSizer(wxDialogHelper* dialog, wxBoxSizer* sizer)
{
    if (sizer->GetOrientation() != wxHORIZONTAL)
        return false;

    for ( wxSizerItemList::compatibility_iterator node = sizer->GetChildren().GetFirst();
          node; node = node->GetNext() )
    {
        wxSizerItem *item = node->GetData();
        wxButton *childButton = wxDynamicCast(item->GetWindow(), wxButton);

        if (childButton && IsStandardButton(dialog, childButton))
            return true;
    }
    return false;
}

/// Check if this is a standard button
bool wxStandardDialogLayoutAdapter::IsStandardButton(wxDialogHelper* dialog, wxButton* button)
{
    wxWindowID id = button->GetId();

    return (id == wxID_OK || id == wxID_CANCEL || id == wxID_YES || id == wxID_NO || id == wxID_SAVE ||
            id == wxID_APPLY || id == wxID_HELP || id == wxID_CONTEXT_HELP || dialog->IsUserButtonId(id));
}

/// Find 'loose' main buttons in the existing layout and add them to the standard dialog sizer
bool wxStandardDialogLayoutAdapter::FindLooseButtons(wxDialogHelper* dialog, wxStdDialogButtonSizer* buttonSizer, wxSizer* sizer, int& count)
{
    wxSizerItemList::compatibility_iterator node = sizer->GetChildren().GetFirst();
    while (node)
    {
        wxSizerItemList::compatibility_iterator next = node->GetNext();
        wxSizerItem *item = node->GetData();
        wxSizer *childSizer = item->GetSizer();
        wxButton *childButton = wxDynamicCast(item->GetWindow(), wxButton);

        if (childButton && IsStandardButton(dialog, childButton))
        {
            sizer->Detach(childButton);
            buttonSizer->AddButton(childButton);
            count ++;
        }

        if (childSizer)
            FindLooseButtons(dialog, buttonSizer, childSizer, count);

        node = next;
    }
    return true;
}

/// Reparent the controls to the scrolled window
void wxStandardDialogLayoutAdapter::ReparentControls(wxWindow* parent, wxWindow* reparentTo, wxSizer* buttonSizer)
{
    wxWindowList::compatibility_iterator node = parent->GetChildren().GetFirst();
    while (node)
    {
        wxWindowList::compatibility_iterator next = node->GetNext();

        wxWindow *win = node->GetData();

        // Don't reparent the scrolled window or buttons in the button sizer
        if (win != reparentTo && (!buttonSizer || !buttonSizer->GetItem(win)))
        {
            win->Reparent(reparentTo);
#ifdef __WXMSW__
            // Restore correct tab order
            ::SetWindowPos((HWND) win->GetHWND(), HWND_BOTTOM, -1, -1, -1, -1, SWP_NOMOVE|SWP_NOSIZE);
#endif
        }

        node = next;
    }
}

/// Find whether scrolling will be necessary for the dialog, returning wxVERTICAL, wxHORIZONTAL or both
int wxStandardDialogLayoutAdapter::MustScroll(wxDialog* dialog, wxSize& windowSize, wxSize& displaySize)
{
    wxSize minWindowSize = dialog->GetSizer()->GetMinSize();
    windowSize = dialog->GetSize();
    windowSize = wxSize(wxMax(windowSize.x, minWindowSize.x), wxMax(windowSize.y, minWindowSize.y));
    displaySize = wxDisplay(wxDisplay::GetFromWindow(dialog)).GetClientArea().GetSize();

    int flags = 0;

    if (windowSize.y >= (displaySize.y - wxEXTRA_DIALOG_HEIGHT))
        flags |= wxVERTICAL;
    if (windowSize.x >= displaySize.x)
        flags |= wxHORIZONTAL;

    return flags;
}

// A function to fit the dialog around its contents, and then adjust for screen size.
// If scrolled windows are passed, scrolling is enabled in the required orientation(s).
bool wxStandardDialogLayoutAdapter::FitWithScrolling(wxDialog* dialog, wxWindowList& windows)
{
    wxSizer* sizer = dialog->GetSizer();
    if (!sizer)
        return false;

    sizer->SetSizeHints(dialog);

    wxSize windowSize, displaySize;
    int scrollFlags = MustScroll(dialog, windowSize, displaySize);
    int scrollBarSize = 20;

    if (scrollFlags)
    {
        int scrollBarExtraX = 0, scrollBarExtraY = 0;
        bool resizeHorizontally = (scrollFlags & wxHORIZONTAL) != 0;
        bool resizeVertically = (scrollFlags & wxVERTICAL) != 0;

        if (windows.GetCount() != 0)
        {
            // Allow extra for a scrollbar, assuming we resizing in one direction only.
            if ((resizeVertically && !resizeHorizontally) && (windowSize.x < (displaySize.x - scrollBarSize)))
                scrollBarExtraX = scrollBarSize;
            if ((resizeHorizontally && !resizeVertically) && (windowSize.y < (displaySize.y - scrollBarSize)))
                scrollBarExtraY = scrollBarSize;
        }

        wxWindowList::compatibility_iterator node = windows.GetFirst();
        while (node)
        {
            wxWindow *win = node->GetData();
            wxScrolledWindow* scrolledWindow = wxDynamicCast(win, wxScrolledWindow);
            if (scrolledWindow)
            {
                scrolledWindow->SetScrollRate(resizeHorizontally ? 10 : 0, resizeVertically ? 10 : 0);

                if (scrolledWindow->GetSizer())
                    scrolledWindow->GetSizer()->Fit(scrolledWindow);
            }

            node = node->GetNext();
        }

        wxSize limitTo = windowSize + wxSize(scrollBarExtraX, scrollBarExtraY);
        if (resizeVertically)
            limitTo.y = displaySize.y - wxEXTRA_DIALOG_HEIGHT;
        if (resizeHorizontally)
            limitTo.x = displaySize.x;

        dialog->SetMinSize(limitTo);
        dialog->SetSize(limitTo);

        dialog->SetSizeHints( limitTo.x, limitTo.y, dialog->GetMaxWidth(), dialog->GetMaxHeight() );
    }

    return true;
}

// A function to fit the dialog around its contents, and then adjust for screen size.
// If a scrolled window is passed, scrolling is enabled in the required orientation(s).
bool wxStandardDialogLayoutAdapter::FitWithScrolling(wxDialog* dialog, wxScrolledWindow* scrolledWindow)
{
    wxWindowList windows;
    windows.Append(scrolledWindow);
    return FitWithScrolling(dialog, windows);
}

/*!
 * Module to initialise standard adapter
 */

class wxDialogLayoutAdapterModule: public wxModule
{
    DECLARE_DYNAMIC_CLASS(wxDialogLayoutAdapterModule)
public:
    wxDialogLayoutAdapterModule() {}
    virtual void OnExit() { delete wxDialogHelper::SetLayoutAdapter(NULL); }
    virtual bool OnInit() { wxDialogHelper::SetLayoutAdapter(new wxStandardDialogLayoutAdapter); return true; }
};

IMPLEMENT_DYNAMIC_CLASS(wxDialogLayoutAdapterModule, wxModule)

/*!
 * wxScrollingDialog
 */

IMPLEMENT_CLASS(wxScrollingDialog, wxDialog)

void wxScrollingDialog::Init()
{
    wxDialogHelper::SetDialog(this);
}

bool wxScrollingDialog::Create(wxWindow *parent, int id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
{
    return wxDialog::Create(parent, id, title, pos, size, style);
}

/// Override Show to rejig the control and sizer hierarchy if necessary
bool wxScrollingDialog::Show(bool show)
{
    if (CanDoLayoutAdaptation())
        DoLayoutAdaptation();

    return wxDialog::Show(show);
}

/// Override ShowModal to rejig the control and sizer hierarchy if necessary
int wxScrollingDialog::ShowModal()
{
    if (CanDoLayoutAdaptation())
        DoLayoutAdaptation();

    return wxDialog::ShowModal();
}

/*!
 * wxScrollingPropertySheetDialog
 */

IMPLEMENT_DYNAMIC_CLASS(wxScrollingPropertySheetDialog, wxPropertySheetDialog)

void wxScrollingPropertySheetDialog::Init()
{
    wxDialogHelper::SetDialog(this);
}

/// Returns the content window
wxWindow* wxScrollingPropertySheetDialog::GetContentWindow() const
{
    return GetBookCtrl();
}

/// Override Show to rejig the control and sizer hierarchy if necessary
bool wxScrollingPropertySheetDialog::Show(bool show)
{
    if (CanDoLayoutAdaptation())
        DoLayoutAdaptation();

    return wxPropertySheetDialog::Show(show);
}

/// Override ShowModal to rejig the control and sizer hierarchy if necessary
int wxScrollingPropertySheetDialog::ShowModal()
{
    if (CanDoLayoutAdaptation())
        DoLayoutAdaptation();

    return wxPropertySheetDialog::ShowModal();
}

