
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <OCPN_AUIManager.h>

//static void DrawResizeHint(wxDC& dc, const wxRect& rect);
//extern bool wxAuiManager_HasLiveResize(wxAuiManager& manager);


// -- static utility functions --

static wxBitmap wxOPaneCreateStippleBitmap()
{
    unsigned char data[] = { 0,0,0,192,192,192, 192,192,192,0,0,0 };
    wxImage img(2,2,data,true);
    return wxBitmap(img);
}

static void ODrawResizeHint(wxDC& dc, const wxRect& rect)
{
#if 0    
    wxBitmap stipple = wxOPaneCreateStippleBitmap();
    wxBrush brush(stipple);
    dc.SetBrush(brush);
#ifdef __WXMSW__
    wxMSWDCImpl *impl = (wxMSWDCImpl*) dc.GetImpl();
    PatBlt(GetHdcOf(*impl), rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight(), PATINVERT);
#else
    dc.SetPen(*wxTRANSPARENT_PEN);
    
    dc.SetLogicalFunction(wxXOR);
    dc.DrawRectangle(rect);
#endif
#endif
}

// OCPN_AUIManager Implementation

// BEGIN_EVENT_TABLE(OCPN_AUIManager, wxEvtHandler)
 //EVT_MOTION(OCPN_AUIManager::OnMotionx)
// END_EVENT_TABLE()

BEGIN_EVENT_TABLE(OCPN_AUIManager, wxEvtHandler)
EVT_AUI_PANE_BUTTON(wxAuiManager::OnPaneButton)
EVT_AUI_RENDER(wxAuiManager::OnRender)
EVT_PAINT(OCPN_AUIManager::OnPaint)
EVT_ERASE_BACKGROUND(OCPN_AUIManager::OnEraseBackground)
EVT_SIZE(OCPN_AUIManager::OnSize)
EVT_SET_CURSOR(OCPN_AUIManager::OnSetCursor)
EVT_LEFT_DOWN(OCPN_AUIManager::OnLeftDown)
EVT_LEFT_UP(OCPN_AUIManager::OnLeftUp)
EVT_MOTION(OCPN_AUIManager::OnMotion)
EVT_LEAVE_WINDOW(OCPN_AUIManager::OnLeaveWindow)
EVT_MOUSE_CAPTURE_LOST(OCPN_AUIManager::OnCaptureLost)
EVT_CHILD_FOCUS(OCPN_AUIManager::OnChildFocus)
EVT_AUI_FIND_MANAGER(OCPN_AUIManager::OnFindManager)
END_EVENT_TABLE()


OCPN_AUIManager::OCPN_AUIManager(wxWindow* managed_wnd, unsigned int flags) :
    wxAuiManager(managed_wnd, flags)

{
}


OCPN_AUIManager::~OCPN_AUIManager()
{
}


void OCPN_AUIManager::OnMotionx(wxMouseEvent& event)
{
    // sometimes when Update() is called from inside this method,
    // a spurious mouse move event is generated; this check will make
    // sure that only real mouse moves will get anywhere in this method;
    // this appears to be a bug somewhere, and I don't know where the
    // mouse move event is being generated.  only verified on MSW
    
    wxPoint mouse_pos = event.GetPosition();
    if (m_lastMouseMove == mouse_pos)
        return;
    m_lastMouseMove = mouse_pos;
    
    
    if (m_action == actionResize)
    {
        // It's necessary to reset m_actionPart since it destroyed
        // by the Update within DoEndResizeAction.
        if (m_currentDragItem != -1)
            m_actionPart = & (m_uiParts.Item(m_currentDragItem));
        else
            m_currentDragItem = m_uiParts.Index(* m_actionPart);
        
        if (m_actionPart)
        {
            wxPoint pos = m_actionPart->rect.GetPosition();
            if (m_actionPart->orientation == wxHORIZONTAL)
                pos.y = wxMax(0, event.m_y - m_actionOffset.y);
            else
                pos.x = wxMax(0, event.m_x - m_actionOffset.x);
            
            if (0/*wxAuiManager_HasLiveResize(*this)*/)
            {
 
#if 1                
                wxSize client_size = m_frame->GetClientSize();
                int used_width = 0, used_height = 0;
                
                size_t dock_i, dock_count = m_docks.GetCount();
                for (dock_i = 0; dock_i < dock_count; ++dock_i)
                {
                    wxAuiDockInfo& dock = m_docks.Item(dock_i);
                    if (dock.dock_direction == wxAUI_DOCK_TOP ||
                        dock.dock_direction == wxAUI_DOCK_BOTTOM)
                    {
                        used_height += dock.size;
                    }
                    if (dock.dock_direction == wxAUI_DOCK_LEFT ||
                        dock.dock_direction == wxAUI_DOCK_RIGHT)
                    {
                        used_width += dock.size;
                    }
//                     if (dock.resizable)
//                         used_width += sashSize;
                }
                
                //printf("dock width:  %d          frame width:  %d\n", used_width, client_size.x);
#endif

                m_frame->ReleaseMouse();
                if(used_width < client_size.x * 9 / 10)
                    DoEndResizeAction(event);
                
                m_frame->CaptureMouse();
            }
            else
            {
                wxRect rect(m_frame->ClientToScreen(pos),
                            m_actionPart->rect.GetSize());
                wxScreenDC dc;
                
                if (!m_actionHintRect.IsEmpty())
                {
                    // remove old resize hint
                    ODrawResizeHint(dc, m_actionHintRect);
                    m_actionHintRect = wxRect();
                }
                
                // draw new resize hint, if it's inside the managed frame
                wxRect frameScreenRect = m_frame->GetScreenRect();
                if (frameScreenRect.Contains(rect))
                {
                    ODrawResizeHint(dc, rect);
                    m_actionHintRect = rect;
                }
            }
        }
    }
    else{
        OnMotion(event);
    }
    
#if 0    
    else if (m_action == actionClickCaption)
    {
        int drag_x_threshold = wxSystemSettings::GetMetric(wxSYS_DRAG_X);
        int drag_y_threshold = wxSystemSettings::GetMetric(wxSYS_DRAG_Y);
        
        // caption has been clicked.  we need to check if the mouse
        // is now being dragged. if it is, we need to change the
        // mouse action to 'drag'
        if (m_actionPart &&
            (abs(event.m_x - m_actionStart.x) > drag_x_threshold ||
            abs(event.m_y - m_actionStart.y) > drag_y_threshold))
        {
            wxAuiPaneInfo* paneInfo = m_actionPart->pane;
            
            if (!paneInfo->IsToolbar())
            {
                if ((m_flags & wxAUI_MGR_ALLOW_FLOATING) &&
                    paneInfo->IsFloatable())
                {
                    m_action = actionDragFloatingPane;
                    
                    // set initial float position
                    wxPoint pt = m_frame->ClientToScreen(event.GetPosition());
                    paneInfo->floating_pos = wxPoint(pt.x - m_actionOffset.x,
                                                     pt.y - m_actionOffset.y);
                    
                    // float the window
                    if (paneInfo->IsMaximized())
                        RestorePane(*paneInfo);
                    paneInfo->Float();
                    Update();
                    
                    m_actionWindow = paneInfo->frame;
                    
                    // action offset is used here to make it feel "natural" to the user
                    // to drag a docked pane and suddenly have it become a floating frame.
                    // Sometimes, however, the offset where the user clicked on the docked
                    // caption is bigger than the width of the floating frame itself, so
                    // in that case we need to set the action offset to a sensible value
                    wxSize frame_size = m_actionWindow->GetSize();
                    if (frame_size.x <= m_actionOffset.x)
                        m_actionOffset.x = 30;
                }
            }
            else
            {
                m_action = actionDragToolbarPane;
                m_actionWindow = paneInfo->window;
            }
        }
    }
    else if (m_action == actionDragFloatingPane)
    {
        if (m_actionWindow)
        {
            // We can't move the child window so we need to get the frame that
            // we want to be really moving. This is probably not the best place
            // to do this but at least it fixes the bug (#13177) for now.
            if (!wxDynamicCast(m_actionWindow, wxAuiFloatingFrame))
            {
                wxAuiPaneInfo& pane = GetPane(m_actionWindow);
                m_actionWindow = pane.frame;
            }
            
            wxPoint pt = m_frame->ClientToScreen(event.GetPosition());
            m_actionWindow->Move(pt.x - m_actionOffset.x,
                                 pt.y - m_actionOffset.y);
        }
    }
    else if (m_action == actionDragToolbarPane)
    {
        wxAuiPaneInfo& pane = GetPane(m_actionWindow);
        wxASSERT_MSG(pane.IsOk(), wxT("Pane window not found"));
        
        pane.SetFlag(wxAuiPaneInfo::actionPane, true);
        
        wxPoint point = event.GetPosition();
        DoDrop(m_docks, m_panes, pane, point, m_actionOffset);
        
        // if DoDrop() decided to float the pane, set up
        // the floating pane's initial position
        if (pane.IsFloating())
        {
            wxPoint pt = m_frame->ClientToScreen(event.GetPosition());
            pane.floating_pos = wxPoint(pt.x - m_actionOffset.x,
                                        pt.y - m_actionOffset.y);
        }
        
        // this will do the actiual move operation;
        // in the case that the pane has been floated,
        // this call will create the floating pane
        // and do the reparenting
        Update();
        
        // if the pane has been floated, change the mouse
        // action actionDragFloatingPane so that subsequent
        // EVT_MOTION() events will move the floating pane
        if (pane.IsFloating())
        {
            pane.state &= ~wxAuiPaneInfo::actionPane;
            m_action = actionDragFloatingPane;
            m_actionWindow = pane.frame;
        }
    }
    else
    {
        wxAuiDockUIPart* part = HitTest(event.GetX(), event.GetY());
        if (part && part->type == wxAuiDockUIPart::typePaneButton)
        {
            if (part != m_hoverButton)
            {
                // make the old button normal
                if (m_hoverButton)
                {
                    UpdateButtonOnScreen(m_hoverButton, event);
                    Repaint();
                }
                
                // mouse is over a button, so repaint the
                // button in hover mode
                UpdateButtonOnScreen(part, event);
                m_hoverButton = part;
                
            }
        }
        else
        {
            if (m_hoverButton)
            {
                m_hoverButton = NULL;
                Repaint();
            }
            else
            {
                event.Skip();
            }
        }
    }
#endif    
}
