#ifndef __GOTOPOSITIONDIALOG_H__
#define __GOTOPOSITIONDIALOG_H__

#include <wx/dialog.h>
#include "ocpn_types.h"

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_GOTOPOS 8100
#define SYMBOL_GOTOPOS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_GOTOPOS_TITLE _("Center View")
#define SYMBOL_GOTOPOS_IDNAME ID_GOTOPOS
#define SYMBOL_GOTOPOS_SIZE wxSize(200, 300)
#define SYMBOL_GOTOPOS_POSITION wxDefaultPosition
#define ID_GOTOPOS_CANCEL 8101
#define ID_GOTOPOS_OK 8102


////@end control identifiers

/*!
 * GoToPositionDialog class declaration
 */
class GoToPositionDialog: public wxDialog
{
      DECLARE_DYNAMIC_CLASS( GoToPositionDialog )
      DECLARE_EVENT_TABLE()

      public:
    /// Constructors
            GoToPositionDialog( );
            GoToPositionDialog( wxWindow* parent, wxWindowID id = SYMBOL_GOTOPOS_IDNAME,
                                const wxString& caption = SYMBOL_GOTOPOS_TITLE,
                                const wxPoint& pos = SYMBOL_GOTOPOS_POSITION,
                                const wxSize& size = SYMBOL_GOTOPOS_SIZE,
                                long style = SYMBOL_GOTOPOS_STYLE );

            ~GoToPositionDialog();

    /// Creation
            bool Create( wxWindow* parent, wxWindowID id = SYMBOL_GOTOPOS_IDNAME,
                         const wxString& caption = SYMBOL_GOTOPOS_TITLE,
                         const wxPoint& pos = SYMBOL_GOTOPOS_POSITION,
                         const wxSize& size = SYMBOL_GOTOPOS_SIZE, long style = SYMBOL_GOTOPOS_STYLE );

            void SetColorScheme(ColorScheme cs);

            void CreateControls();

            void OnGoToPosCancelClick( wxCommandEvent& event );
            void OnGoToPosOkClick( wxCommandEvent& event );
            void OnPositionCtlUpdated( wxCommandEvent& event );
            void CheckPasteBufferForPosition();

      /// Should we show tooltips?
            static bool ShowToolTips();

            wxTextCtrl*   m_MarkLatCtl;
            wxTextCtrl*   m_MarkLonCtl;
            wxButton*     m_CancelButton;
            wxButton*     m_OKButton;

            double        m_lat_save;
            double        m_lon_save;
};

#endif
