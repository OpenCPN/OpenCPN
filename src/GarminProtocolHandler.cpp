/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Garmin NMEA Data Stream Object
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
 ***************************************************************************

 ***************************************************************************
 *  Parts of this file were adapted from source code found in              *
 *  John F. Waers (jfwaers@csn.net) public domain program MacGPS45         *
 ***************************************************************************
 *
 */
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "wx/tokenzr.h"
#include <wx/datetime.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef __WXMSW__
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#include "dychart.h"

#include "datastream.h"
#include "OCPN_DataStreamEvent.h"
#include "OCP_DataStreamInput_Thread.h"
#include "garmin_wrapper.h"
#include "GarminProtocolHandler.h"
#include "nmea0183.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

#include <vector>

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif



extern bool g_benableUDPNullHeader;

#define N_DOG_TIMEOUT   5

#ifdef __WXMSW__
// {2C9C45C2-8E7D-4C08-A12D-816BBAE722C0}
DEFINE_GUID(GARMIN_GUID1, 0x2c9c45c2L, 0x8e7d, 0x4c08, 0xa1, 0x2d, 0x81, 0x6b, 0xba, 0xe7, 0x22, 0xc0);
#endif

#ifdef __OCPN__ANDROID__
#include <netdb.h>
int gethostbyaddr_r(const char *, int, int, struct hostent *, char *, size_t, struct hostent **, int *)
{
    wxLogMessage(_T("Called stub gethostbyaddr_r()"));
    return 0;
}
#endif




//----------------------------------------------------------------------------
// Garmin Device Management
// Handle USB and Serial Port Garmin PVT protocol data interface.
//----------------------------------------------------------------------------

#ifdef __WXMSW__
BOOL IsUserAdmin(VOID)
/*++
 *            Routine Description: This routine returns TRUE if the caller's
 *            process is a member of the Administrators local group. Caller is NOT
 *            expected to be impersonating anyone and is expected to be able to
 *            open its own process and process token.
 *            Arguments: None.
 * Return Value:
 *            TRUE - Caller has Administrators local group.
 *            FALSE - Caller does not have Administrators local group. --
 */
{
    BOOL b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    b = AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup);
    if(b)
    {
        if (!CheckTokenMembership( NULL, AdministratorsGroup, &b))
        {
            b = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }

    return(b);
}

void
le_write16(void *addr, const unsigned value)
{
    unsigned char *p = (unsigned char *)addr;
    p[0] = value;
    p[1] = value >> 8;

}

void
le_write32(void *addr, const unsigned value)
{
    unsigned char *p = (unsigned char *)addr;
    p[0] = value;
    p[1] = value >> 8;
    p[2] = value >> 16;
    p[3] = value >> 24;
}

signed int
le_read16(const void *addr)
{
    const unsigned char *p = (const unsigned char *)addr;
    return p[0] | (p[1] << 8);
}

signed int
le_read32(const void *addr)
{
    const unsigned char *p = (const unsigned char *)addr;
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}


#endif




BEGIN_EVENT_TABLE(GarminProtocolHandler, wxEvtHandler)
EVT_TIMER(TIMER_GARMIN1, GarminProtocolHandler::OnTimerGarmin1)
END_EVENT_TABLE()


GarminProtocolHandler::GarminProtocolHandler(DataStream *parent, wxEvtHandler *MessageTarget, bool bsel_usb)
{
    m_pparent = parent;
    m_pMainEventHandler = MessageTarget;
    m_garmin_serial_thread = NULL;
    m_garmin_usb_thread = NULL;
    m_bOK = false;
    m_busb = bsel_usb;

    //      Connect(wxEVT_OCPN_GARMIN, (wxObjectEventFunction)(wxEventFunction)&GarminProtocolHandler::OnEvtGarmin);

    char  pvt_on[14] =
            {20, 0, 0, 0, 10, 0, 0, 0, 2, 0, 0, 0, 49, 0};

    char  pvt_off[14] =
            {20, 0, 0, 0, 10, 0, 0, 0, 2, 0, 0, 0, 50, 0};

#ifdef __WXMSW__
    if(m_busb) {
        m_usb_handle = INVALID_HANDLE_VALUE;

        m_bneed_int_reset = true;
        m_receive_state = rs_fromintr;
        m_ndelay = 0;

        wxLogMessage(_T("Searching for Garmin DeviceInterface and Device..."));

        if(!FindGarminDeviceInterface()) {
            wxLogMessage(_T("   Find:Is the Garmin USB driver installed?"));
        }
        else {
            if(!ResetGarminUSBDriver())
                wxLogMessage(_T("   Reset:Is the Garmin USB Device plugged in?"));
        }
    }
#endif

    //  Not using USB, so try a Garmin port open and device ident
    if(! m_busb ) {
        m_port =  m_pparent->GetPort().AfterFirst(':');      // strip "Serial:"

        // Start handler thread
        m_garmin_serial_thread = new GARMIN_Serial_Thread(this, m_pparent, m_pMainEventHandler, m_port );
        m_Thread_run_flag = 1;
        m_garmin_serial_thread->Run();
    }

    TimerGarmin1.SetOwner(this, TIMER_GARMIN1);
    TimerGarmin1.Start(100);
}



GarminProtocolHandler::~GarminProtocolHandler()
{
}

void GarminProtocolHandler::Close( void )
{
    TimerGarmin1.Stop();

    StopIOThread(true);
    StopSerialThread();

}

void GarminProtocolHandler::StopSerialThread(void)
{
    if(m_garmin_serial_thread)
    {
        wxLogMessage(_T("Stopping Garmin Serial thread"));
        m_Thread_run_flag = 0;

        int tsec = 5;
        while((m_Thread_run_flag >= 0) && (tsec--))
        {
            wxSleep(1);
        }

        wxString msg;
        if(m_Thread_run_flag < 0)
            msg.Printf(_T("Stopped in %d sec."), 5 - tsec);
        else
            msg.Printf(_T("Not Stopped after 5 sec."));
        wxLogMessage(msg);
    }

    m_garmin_serial_thread = NULL;

}



void GarminProtocolHandler::StopIOThread(bool b_pause)
{
    if(b_pause)
        TimerGarmin1.Stop();

    if(m_garmin_usb_thread)
    {
        wxLogMessage(_T("Stopping Garmin USB thread"));
        m_Thread_run_flag = 0;

        int tsec = 5;
        while((m_Thread_run_flag >= 0) && (tsec--))
        {
            wxSleep(1);
        }

        wxString msg;
        if(m_Thread_run_flag < 0)
            msg.Printf(_T("Stopped in %d sec."), 5 - tsec);
        else
            msg.Printf(_T("Not Stopped after 5 sec."));
        wxLogMessage(msg);
    }

    m_garmin_usb_thread = NULL;

#ifdef __WXMSW__
    if(m_busb && (m_usb_handle != INVALID_HANDLE_VALUE) )
        CloseHandle(m_usb_handle);
    m_usb_handle = INVALID_HANDLE_VALUE;
#endif

    m_ndelay = 30;          // Fix delay for next restart

}

void GarminProtocolHandler::RestartIOThread(void)
{
    wxLogMessage(_T("Restarting Garmin I/O thread"));
    TimerGarmin1.Start(1000);
}




void GarminProtocolHandler::OnTimerGarmin1(wxTimerEvent& event)
{
    char  pvt_on[14] =
            {20, 0, 0, 0, 10, 0, 0, 0, 2, 0, 0, 0, 49, 0};

    TimerGarmin1.Stop();

    if(m_busb) {
#ifdef __WXMSW__
        //  Try to open the Garmin USB device
        if(INVALID_HANDLE_VALUE == m_usb_handle)
        {
            if(INVALID_HANDLE_VALUE != garmin_usb_start())
            {
                //    Send out a request for Garmin PVT data
                m_receive_state = rs_fromintr;
                gusb_cmd_send((const garmin_usb_packet *) pvt_on, sizeof(pvt_on));

                //    Start the pump
                m_garmin_usb_thread = new GARMIN_USB_Thread(this, m_pparent,
						m_pMainEventHandler, (wxIntPtr)m_usb_handle, m_max_tx_size);
                m_Thread_run_flag = 1;
                m_garmin_usb_thread->Run();
            }
        }
#endif
    }

    TimerGarmin1.Start(1000);
}

#ifdef __WXMSW__
bool GarminProtocolHandler::ResetGarminUSBDriver()
{
    OSVERSIONINFO version_info;
    version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if(GetVersionEx(&version_info))
    {
        if(version_info.dwMajorVersion > 5)
        {
            if(!IsUserAdmin())
            {
                wxLogMessage(_T("    GarminUSBDriver Reset skipped, requires elevated privileges on Vista and later...."));
                return true;
            }
        }
    }


    HDEVINFO devs;
    SP_DEVINFO_DATA devInfo;
    SP_PROPCHANGE_PARAMS pchange;

    devs = SetupDiGetClassDevs( (GUID *) &GARMIN_GUID1, NULL, NULL,
                                DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    if (devs == INVALID_HANDLE_VALUE)
        return false;

    devInfo.cbSize = sizeof(devInfo);
    if(!SetupDiEnumDeviceInfo(devs,0,&devInfo))
    {
        wxLogMessage(_T("   GarminUSBDriver Reset0 failed..."));
        return false;
    }

    pchange.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pchange.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    pchange.StateChange = DICS_PROPCHANGE;
    pchange.Scope = DICS_FLAG_CONFIGSPECIFIC;
    pchange.HwProfile = 0;

    if(!SetupDiSetClassInstallParams(devs,&devInfo,&pchange.ClassInstallHeader,sizeof(pchange)))
    {
        wxLogMessage(_T("   GarminUSBDriver Reset1 failed..."));
        return false;
    }

    if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,devs,&devInfo))
    {
        wxLogMessage(_T("   GarminUSBDriver Reset2 failed..."));
        return false;
    }

    wxLogMessage(_T("GarminUSBDriver Reset succeeded."));

    return true;

}



bool GarminProtocolHandler::FindGarminDeviceInterface()
{      //    Search for a useable Garmin Device Interface Class

HDEVINFO hdevinfo;
SP_DEVINFO_DATA devInfo;

hdevinfo = SetupDiGetClassDevs( (GUID *) &GARMIN_GUID1, NULL, NULL,
                                DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

if (hdevinfo != INVALID_HANDLE_VALUE)
{
    devInfo.cbSize = sizeof(devInfo);
    if(!SetupDiEnumDeviceInfo(hdevinfo,0,&devInfo))
    {
        return false;
    }
}

return true;
}


bool GarminProtocolHandler::IsGarminPlugged()
{
    DWORD size = 0;

    HDEVINFO hdevinfo;
    SP_DEVICE_INTERFACE_DATA infodata;

    //    Search for the Garmin Device Interface Class
    hdevinfo = SetupDiGetClassDevs( (GUID *) &GARMIN_GUID1, NULL, NULL,
                                    DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

    if (hdevinfo == INVALID_HANDLE_VALUE)
        return INVALID_HANDLE_VALUE;

    infodata.cbSize = sizeof(infodata);

    bool bgarmin_unit_found = (SetupDiEnumDeviceInterfaces(hdevinfo,
                                                           NULL,(GUID *) &GARMIN_GUID1, 0, &infodata) != 0);

    if(!bgarmin_unit_found)
        return false;

    PSP_INTERFACE_DEVICE_DETAIL_DATA pdd = NULL;
    SP_DEVINFO_DATA devinfo;

    SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata,
                                    NULL, 0, &size, NULL);

    pdd = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(size);
    pdd->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

    devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
    if (!SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata,
        pdd, size, NULL, &devinfo))
    {
        free(pdd);
        return false;
    }

    free(pdd);

    return true;
}


HANDLE GarminProtocolHandler::garmin_usb_start()
{
    DWORD size = 0;

    HDEVINFO hdevinfo;
    SP_DEVICE_INTERFACE_DATA infodata;

    //    Search for the Garmin Device Interface Class
    hdevinfo = SetupDiGetClassDevs( (GUID *) &GARMIN_GUID1, NULL, NULL,
                                    DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

    if (hdevinfo == INVALID_HANDLE_VALUE)
        return INVALID_HANDLE_VALUE;

    infodata.cbSize = sizeof(infodata);

    bool bgarmin_unit_found = (SetupDiEnumDeviceInterfaces(hdevinfo,
                                                           NULL,(GUID *) &GARMIN_GUID1, 0, &infodata) != 0);

    if(!bgarmin_unit_found)
        return INVALID_HANDLE_VALUE;

    wxLogMessage(_T("Garmin USB Device Found"));

    if((m_usb_handle == INVALID_HANDLE_VALUE) || (m_usb_handle == 0))
    {
        PSP_INTERFACE_DEVICE_DETAIL_DATA pdd = NULL;
        SP_DEVINFO_DATA devinfo;

        SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata,
                                        NULL, 0, &size, NULL);

        pdd = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(size);
        pdd->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

        devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
        if (!SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata,
            pdd, size, NULL, &devinfo))
        {
            wxLogMessage(_T("   SetupDiGetDeviceInterfaceDetail failed for Garmin Device..."));
            free(pdd);
            return INVALID_HANDLE_VALUE;
        }

        /* Whew.  All that just to get something we can open... */
        //            wxString msg;
        //            msg.Printf(_T("Windows GUID for interface is %s"),pdd->DevicePath);
        //            wxLogMessage(msg);

        if(m_bneed_int_reset)
        {
            ResetGarminUSBDriver();
            m_bneed_int_reset = false;
        }

        m_usb_handle = CreateFile(pdd->DevicePath, GENERIC_READ|GENERIC_WRITE,
                                  0, NULL, OPEN_EXISTING, 0, NULL );


        if (m_usb_handle == INVALID_HANDLE_VALUE)
        {
            wxString msg;
            msg.Printf(_T("   (usb) CreateFile on '%s' failed"), pdd->DevicePath);
            wxLogMessage(msg);
        }

/*
        DEV_BROADCAST_HANDLE filterData;
        filterData.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
        filterData.dbch_devicetype = DBT_DEVTYP_HANDLE;
        filterData.dbch_reserved = 0;
        filterData.dbch_handle = m_usb_handle;     // file handle used in call to RegisterDeviceNotification
        filterData.dbch_hdevnotify = 0;            // returned from RegisterDeviceNotification

        HDEVNOTIFY m_hDevNotify = RegisterDeviceNotification( GetHWND(), &filterData, DEVICE_NOTIFY_WINDOW_HANDLE);
*/

        free(pdd);
    }

    m_max_tx_size = 0;

    if(!DeviceIoControl(m_usb_handle, IOCTL_GARMIN_USB_BULK_OUT_PACKET_SIZE,
        NULL, 0, &m_max_tx_size, GARMIN_USB_INTERRUPT_DATA_SIZE,
        &size, NULL))
    {
        wxLogMessage(_T("   Couldn't get Garmin USB packet size."));
        CloseHandle(m_usb_handle);
        m_usb_handle = INVALID_HANDLE_VALUE;
        return INVALID_HANDLE_VALUE;
    }

    if(!gusb_syncup())
    {
        CloseHandle(m_usb_handle);
        m_usb_handle = INVALID_HANDLE_VALUE;
    }

    return m_usb_handle;
}


bool GarminProtocolHandler::gusb_syncup(void)
{
    static int unit_number;
    static const char  oinit[12] =
    {0, 0, 0, 0, GUSB_SESSION_START, 0, 0, 0, 0, 0, 0, 0};
    garmin_usb_packet iresp;
    int i;

    /*
     * This is our first communication with the unit.
     */


    m_receive_state = rs_fromintr;

    for(i = 0; i < 25; i++) {
        le_write16(&iresp.gusb_pkt.pkt_id[0], 0);
        le_write32(&iresp.gusb_pkt.datasz[0], 0);
        le_write32(&iresp.gusb_pkt.databuf[0], 0);

        if(gusb_cmd_send((const garmin_usb_packet *) oinit, sizeof(oinit)))
        {
            gusb_cmd_get(&iresp, sizeof(iresp));

            if ((le_read16(iresp.gusb_pkt.pkt_id) == GUSB_SESSION_ACK) &&
                (le_read32(iresp.gusb_pkt.datasz) == 4))
            {
                //                unsigned serial_number = le_read32(iresp.gusb_pkt.databuf);
                //                garmin_unit_info[unit_number].serial_number = serial_number;
                //                gusb_id_unit(&garmin_unit_info[unit_number]);

                unit_number++;

                wxLogMessage(_T("Successful Garmin USB syncup."));
                return true;;
            }
        }
    }
    wxLogMessage(_T("   Unable to establish Garmin USB syncup."));
    return false;
}

int GarminProtocolHandler::gusb_cmd_send(const garmin_usb_packet *opkt, size_t sz)
{
    unsigned int rv;

    unsigned char *obuf = (unsigned char *) &opkt->dbuf[0];

    rv = gusb_win_send(opkt, sz);

    /*
     * Recursion, when used in a disciplined way, can be our friend.
     *
     * The Garmin protocol requires that packets that are exactly
     * a multiple of the max tx size be followed by a zero length
     * packet.  Do that here so we can see it in debugging traces.
     */

    if (sz && !(sz % m_max_tx_size)) {
        wxLogMessage(_T("win_send_call1"));
        gusb_win_send(opkt, 0);
        wxLogMessage(_T("win_send_ret1"));
    }

    return (rv);
}


int GarminProtocolHandler::gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz)
{
    int rv = 0;
    unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];
    int orig_receive_state;
    top:
    orig_receive_state = m_receive_state;
    switch (m_receive_state) {
        case rs_fromintr:
            rv = gusb_win_get(ibuf, sz);
            break;
        case rs_frombulk:
            rv = gusb_win_get_bulk(ibuf, sz);
            break;

    }

    /* Adjust internal state and retry the read */
    if ((rv > 0) && (ibuf->gusb_pkt.pkt_id[0] == GUSB_REQUEST_BULK)) {
        m_receive_state = rs_frombulk;
        goto top;
    }
    /*
     * If we were reading from the bulk pipe and we just got
     * a zero request, adjust our internal state.
     * It's tempting to retry the read here to hide this "stray"
     * packet from our callers, but that only works when you know
     * there's another packet coming.   That works in every case
     * except the A000 discovery sequence.
     */
    if ((m_receive_state == rs_frombulk) && (rv <= 0)) {
        m_receive_state = rs_fromintr;
    }

    return rv;
}





int GarminProtocolHandler::gusb_win_get(garmin_usb_packet *ibuf, size_t sz)
{
    DWORD rxed = GARMIN_USB_INTERRUPT_DATA_SIZE;
    unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];
    int tsz=0;

    while (sz)
    {
        /* The driver wrongly (IMO) rejects reads smaller than
         * GARMIN_USB_INTERRUPT_DATA_SIZE
         */
        if(!DeviceIoControl(m_usb_handle, IOCTL_GARMIN_USB_INTERRUPT_IN, NULL, 0,
            buf, GARMIN_USB_INTERRUPT_DATA_SIZE, &rxed, NULL))
        {
            //                GPS_Serial_Error("Ioctl");
            //                fatal("ioctl\n");
        }

        buf += rxed;
        sz  -= rxed;
        tsz += rxed;
        if (rxed < GARMIN_USB_INTERRUPT_DATA_SIZE)
            break;
    }
    return tsz;
}

int GarminProtocolHandler::gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz)
{
    int n;
    DWORD rsz;
    unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];

    n = ReadFile(m_usb_handle, buf, sz, &rsz, NULL);

    return rsz;
}

int GarminProtocolHandler::gusb_win_send(const garmin_usb_packet *opkt, size_t sz)
{
    DWORD rsz;
    unsigned char *obuf = (unsigned char *) &opkt->dbuf[0];

    /* The spec warns us about making writes an exact multiple
     * of the packet size, but isn't clear whether we can issue
     * data in a single call to WriteFile if it spans buffers.
     */
    WriteFile(m_usb_handle, obuf, sz, &rsz, NULL);
	int err = GetLastError();

    //    if (rsz != sz)
    //          fatal ("Error sending %d bytes.   Successfully sent %ld\n", sz, rsz);


    return rsz;
}

/*
WXLRESULT GarminProtocolHandler::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    // did we process the message?
    bool processed = false;

    // the return value
    bool rc;
    PDEV_BROADCAST_HDR pDBHdr;
    PDEV_BROADCAST_HANDLE pDBHandle;

    // for most messages we should return 0 when we do process the message
    rc = 0;

    switch ( message )
    {
        case WM_DEVICECHANGE:
            switch (wParam)
            {
                case DBT_DEVICEREMOVEPENDING:
                case DBT_DEVICEREMOVECOMPLETE:
                    pDBHdr = (PDEV_BROADCAST_HDR) lParam;
                    switch (pDBHdr->dbch_devicetype)
                        case DBT_DEVTYP_HANDLE:
                            // A Device has been removed
                            // Stop the IO thread and close open handle  to device

                            pDBHandle = (PDEV_BROADCAST_HANDLE) pDBHdr;
                            HANDLE target_handle = pDBHandle->dbch_handle;

                            wxLogMessage(_T("Garmin USB Device Removed"));
                            StopIOThread(false);
                            m_bneed_int_reset = true;
                            processed = true;
                            break;
            }

            break;

    }

    if ( !processed )
    {
        rc = (MSWDefWindowProc(message, wParam, lParam) != 0);
    }

    return rc;
}
*/
#endif

D800_Pvt_Data_Type_Aligned mypvt;

//-------------------------------------------------------------------------------------------------------------
//
//    Garmin Serial Port Worker Thread
//
//    This thread manages reading the positioning data stream from the declared Garmin GRMN Mode serial device
//
//-------------------------------------------------------------------------------------------------------------
GARMIN_Serial_Thread::GARMIN_Serial_Thread(GarminProtocolHandler *parent,
                                           DataStream *GParentStream,
                                           wxEvtHandler *MessageTarget,
                                           wxString port)
{
m_parent = parent;                          // This thread's immediate "parent"
m_parent_stream = GParentStream;
m_pMessageTarget = MessageTarget;
m_port = port;

Create();
}

GARMIN_Serial_Thread::~GARMIN_Serial_Thread(void)
{
}


//    Entry Point
void *GARMIN_Serial_Thread::Entry()
{
    //   m_parent->SetSecThreadActive();               // I am alive
    m_bdetected = false;
    m_bconnected = false;

    bool not_done = true;
    wxDateTime last_rx_time;


#ifdef USE_GARMINHOST
    //    The main loop

    while((not_done) && (m_parent->m_Thread_run_flag > 0)) {

        if(TestDestroy()) {
            not_done = false;                               // smooth exit
            goto thread_exit;
        }

        while( !m_bdetected ) {

            //  Try to init the port once
            int v_init = Garmin_GPS_Init(m_port);
            if( v_init < 0 ){           //  Open failed, so sleep and try again
                for( int i=0 ; i < 4 ; i++) {
                    wxSleep(1);
                    if(TestDestroy())
                        goto thread_exit;
                    if( !m_parent->m_Thread_run_flag )
                        goto thread_exit;
                }
            }
            else
                m_bdetected = true;
        }                       // while not detected

        // Detected OK

        //      Start PVT packet transmission
        if( !m_bconnected ) {
            if( !Garmin_GPS_PVT_On( m_port) ) {
                m_bdetected = false;            // error, would not accept PVT On
                m_bconnected = false;
            }
            else
                m_bconnected = true;
        }

        if( m_bconnected ) {

            D800_Pvt_Data_Type_Aligned *ppvt = &mypvt;
            int ret = Garmin_GPS_GetPVT(&ppvt);
            if(ret > 0) {
                if((mypvt.fix) >= 2 && (mypvt.fix <= 5)) {
                    // Synthesize an NMEA GMRMC message
                    SENTENCE snt;
                    NMEA0183 oNMEA0183;
                    oNMEA0183.TalkerID = _T ( "GM" );

                    if ( mypvt.lat < 0. )
                        oNMEA0183.Rmc.Position.Latitude.Set ( -mypvt.lat, _T ( "S" ) );
                    else
                        oNMEA0183.Rmc.Position.Latitude.Set ( mypvt.lat, _T ( "N" ) );

                    if ( mypvt.lon < 0. )
                        oNMEA0183.Rmc.Position.Longitude.Set ( -mypvt.lon, _T ( "W" ) );
                    else
                        oNMEA0183.Rmc.Position.Longitude.Set ( mypvt.lon, _T ( "E" ) );

                    /* speed over ground */
                    double sog = sqrt(mypvt.east*mypvt.east + mypvt.north*mypvt.north) * 3.6 / 1.852;
                    oNMEA0183.Rmc.SpeedOverGroundKnots = sog;

                    /* course over ground */
                    double course = atan2(mypvt.east, mypvt.north);
                    if (course < 0)
                        course += 2*PI;
                    double cog = course * 180 / PI;
                    oNMEA0183.Rmc.TrackMadeGoodDegreesTrue = cog;

                    oNMEA0183.Rmc.IsDataValid = NTrue;

                    oNMEA0183.Rmc.Write ( snt );
                    wxString message = snt.Sentence;

                    if( m_pMessageTarget ) {
                        OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
                        wxCharBuffer buffer=message.ToUTF8();
                        if(buffer.data()) {
                            Nevent.SetNMEAString( buffer.data() );
                            Nevent.SetStream( m_parent_stream );

                            m_pMessageTarget->AddPendingEvent(Nevent);
                        }
                    }

                    last_rx_time = wxDateTime::Now();

                }
            }
            else {
                wxDateTime now = wxDateTime::Now();
                if( last_rx_time.IsValid() ) {
                    wxTimeSpan delta_time = now - last_rx_time;
                    if( delta_time.GetSeconds() > 5 ) {
                        m_bdetected = false;
                        m_bconnected = false;
                        Garmin_GPS_ClosePortVerify();
                    }
                }
            }
        }
    }                          // the big while...

    thread_exit:

    Garmin_GPS_PVT_Off( m_port);
    Garmin_GPS_ClosePortVerify();

#else           //#ifdef USE_GARMINHOST

    while((not_done) && (m_parent->m_Thread_run_flag > 0)) {

        wxSleep(1);
        if(TestDestroy()) {
            not_done = false;                               // smooth exit
            goto thread_exit;
        }
    }

thread_exit:

#endif          //#ifdef USE_GARMINHOST

    m_parent->m_Thread_run_flag = -1;   // in GarminProtocolHandler
    return 0;
}


//-------------------------------------------------------------------------------------------------------------
//    GARMIN_USB_Thread Implementation
//-------------------------------------------------------------------------------------------------------------

GARMIN_USB_Thread::GARMIN_USB_Thread(GarminProtocolHandler *parent,
                                     DataStream *GParentStream,
                                     wxEvtHandler *MessageTarget,
                                     unsigned int device_handle,
size_t max_tx_size)
{
m_parent = parent;                        // This thread's immediate "parent"
m_parent_stream = GParentStream;
m_pMessageTarget = MessageTarget;
m_max_tx_size = max_tx_size;

#ifdef __WXMSW__
m_usb_handle = (HANDLE)(device_handle & 0xffff);
#endif

Create();
}

GARMIN_USB_Thread::~GARMIN_USB_Thread()
{
}

void *GARMIN_USB_Thread::Entry()
{
    garmin_usb_packet iresp;
    int n_short_read = 0;
    m_receive_state = rs_fromintr;

    //    Here comes the big while loop
    while(m_parent->m_Thread_run_flag > 0)
    {
        if(TestDestroy())
            goto thread_prexit;                               // smooth exit

        //    Get one  packet

        int nr = gusb_cmd_get(&iresp, sizeof(iresp));

        if(iresp.gusb_pkt.pkt_id[0] == GUSB_RESPONSE_SDR)     //Satellite Data Record
        {
            unsigned char *t = (unsigned char *)&(iresp.gusb_pkt.databuf[0]);
            for(int i=0 ; i < 12 ; i++)
            {
                m_sat_data[i].svid =  *t++;
                m_sat_data[i].snr =   ((*t)<<8) + *(t+1); t += 2;
                m_sat_data[i].elev =  *t++;
                m_sat_data[i].azmth = ((*t)<<8) + *(t+1); t += 2;
                m_sat_data[i].status = *t++;
            }

            m_nSats = 0;
            for(int i=0 ; i < 12 ; i++)
            {
                if(m_sat_data[i].svid != 255)
                    m_nSats++;
            }

            // Synthesize an NMEA GMGSV message
            SENTENCE snt;
            NMEA0183 oNMEA0183;
            oNMEA0183.TalkerID = _T ( "GM" );
            oNMEA0183.Gsv.SatsInView = m_nSats;

            oNMEA0183.Gsv.Write ( snt );
            wxString message = snt.Sentence;

            if( m_pMessageTarget ) {
                OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
                wxCharBuffer buffer=message.ToUTF8();
                if(buffer.data()) {
                    Nevent.SetNMEAString( buffer.data() );
                    Nevent.SetStream( m_parent_stream );

                    m_pMessageTarget->AddPendingEvent(Nevent);
                }
            }

        }

        if(iresp.gusb_pkt.pkt_id[0] == GUSB_RESPONSE_PVT)     //PVT Data Record
        {


            D800_Pvt_Data_Type *ppvt = (D800_Pvt_Data_Type *)&(iresp.gusb_pkt.databuf[0]);

            if((ppvt->fix) >= 2 && (ppvt->fix <= 5)) {
                // Synthesize an NMEA GMRMC message
                SENTENCE snt;
                NMEA0183 oNMEA0183;
                oNMEA0183.TalkerID = _T ( "GM" );

                if ( ppvt->lat < 0. )
                    oNMEA0183.Rmc.Position.Latitude.Set ( -ppvt->lat*180./PI, _T ( "S" ) );
                else
                    oNMEA0183.Rmc.Position.Latitude.Set ( ppvt->lat*180./PI, _T ( "N" ) );

                if ( ppvt->lon < 0. )
                    oNMEA0183.Rmc.Position.Longitude.Set ( -ppvt->lon*180./PI, _T ( "W" ) );
                else
                    oNMEA0183.Rmc.Position.Longitude.Set ( ppvt->lon*180./PI, _T ( "E" ) );

                /* speed over ground */
                double sog = sqrt(ppvt->east*ppvt->east + ppvt->north*ppvt->north) * 3.6 / 1.852;
                oNMEA0183.Rmc.SpeedOverGroundKnots = sog;

                /* course over ground */
                double course = atan2(ppvt->east, ppvt->north);
                if (course < 0)
                    course += 2*PI;
                double cog = course * 180 / PI;
                oNMEA0183.Rmc.TrackMadeGoodDegreesTrue = cog;

                oNMEA0183.Rmc.IsDataValid = NTrue;

                oNMEA0183.Rmc.Write ( snt );
                wxString message = snt.Sentence;

                if( m_pMessageTarget ) {
                    OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
                    wxCharBuffer buffer=message.ToUTF8();
                    if(buffer.data()) {
                        Nevent.SetNMEAString( buffer.data() );
                        Nevent.SetStream( m_parent_stream );

                        m_pMessageTarget->AddPendingEvent(Nevent);
                    }
                }

            }

        }

    }
    thread_prexit:
    m_parent->m_Thread_run_flag = -1;
    return 0;
}


int GARMIN_USB_Thread::gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz)
{
    int rv = 0;
    unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];
    int orig_receive_state;
    top:
    orig_receive_state = m_receive_state;
    switch (m_receive_state) {
        case rs_fromintr:
            rv = gusb_win_get(ibuf, sz);
            break;
        case rs_frombulk:
            rv = gusb_win_get_bulk(ibuf, sz);
            break;
    }

    /* Adjust internal state and retry the read */
    if ((rv > 0) && (ibuf->gusb_pkt.pkt_id[0] == GUSB_REQUEST_BULK)) {
        m_receive_state = rs_frombulk;
        goto top;
    }
    /*
    * If we were reading from the bulk pipe and we just got
    * a zero request, adjust our internal state.
    * It's tempting to retry the read here to hide this "stray"
    * packet from our callers, but that only works when you know
    * there's another packet coming.   That works in every case
    * except the A000 discovery sequence.
    */
    if ((m_receive_state == rs_frombulk) && (rv <= 0)) {
        m_receive_state = rs_fromintr;
    }

    return rv;
}

int GARMIN_USB_Thread::gusb_win_get(garmin_usb_packet *ibuf, size_t sz)
{
    int tsz=0;
#ifdef __WXMSW__
    DWORD rxed = GARMIN_USB_INTERRUPT_DATA_SIZE;
      unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];

      while (sz)
      {
            /* The driver wrongly (IMO) rejects reads smaller than
            * GARMIN_USB_INTERRUPT_DATA_SIZE
            */
            if(!DeviceIoControl(m_usb_handle, IOCTL_GARMIN_USB_INTERRUPT_IN, NULL, 0,
                buf, GARMIN_USB_INTERRUPT_DATA_SIZE, &rxed, NULL))
            {
//                GPS_Serial_Error("Ioctl");
//                fatal("ioctl\n");
            }

            buf += rxed;
            sz  -= rxed;
            tsz += rxed;
            if (rxed < GARMIN_USB_INTERRUPT_DATA_SIZE)
                  break;
      }

#endif
    return tsz;
}

int GARMIN_USB_Thread::gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz)
{
    int n;
    int ret_val = 0;
#ifdef __WXMSW__
    DWORD rsz;
      unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];

      n = ReadFile(m_usb_handle, buf, sz, &rsz, NULL);
      ret_val = rsz;
#endif

    return ret_val;
}

