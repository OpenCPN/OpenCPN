
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/aui/aui.h>

class OCPN_AUIManager : public wxAuiManager
{
public:
    OCPN_AUIManager(wxWindow* managed_wnd = NULL, unsigned int flags = wxAUI_MGR_DEFAULT);
    ~OCPN_AUIManager();
 
    void OnMotionx(wxMouseEvent& event);

    //void OnPaint(wxPaintEvent& evt){ wxAuiManager::OnPaint(evt); }
    //void OnEraseBackground(wxEraseEvent& evt){ wxAuiManager::OnEraseBackground(evt); }
//     void OnSize(wxSizeEvent& evt);
//     void OnSetCursor(wxSetCursorEvent& evt);
//     void OnLeftDown(wxMouseEvent& evt);
//     void OnLeftUp(wxMouseEvent& evt);
//     void OnMotion(wxMouseEvent& evt);
//     void OnCaptureLost(wxMouseCaptureLostEvent& evt);
//     void OnLeaveWindow(wxMouseEvent& evt);
//     void OnChildFocus(wxChildFocusEvent& evt);
//     void OnHintFadeTimer(wxTimerEvent& evt);
//     void OnFindManager(wxAuiManagerEvent& evt);
    
    DECLARE_EVENT_TABLE()
    
};