/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Main wxWidgets Program
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

#ifdef __MSVC__
#include "winsock2.h"
#include "wx/msw/winundef.h"
#endif

#include "config.h"

#include <iostream>
#include <regex>
#include <string>
#include <unordered_set>
#include <vector>

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#include "qdebug.h"
#endif

#include <wx/arrstr.h>
#include <wx/log.h>
#include <wx/utils.h>

#ifdef OCPN_USE_NEWSERIAL
#include "serial/serial.h"
#endif

#ifdef HAVE_LIBUDEV
#include "libudev.h"
#endif

#ifdef HAVE_DIRENT_H
#include "dirent.h"
#endif

#ifdef HAVE_LINUX_SERIAL_H
#include "linux/serial.h"
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_READLINK
#include <unistd.h>
#endif

#ifdef __linux__
#include <termios.h>
#include <linux/serial.h>
#endif


#ifdef __WXMSW__
#include <windows.h>
#include <setupapi.h>
#endif

#ifdef __WXOSX__
#include "macutils.h"
#endif

#include "gui_lib.h"
#include "GarminProtocolHandler.h"

#ifdef __WXMSW__
DEFINE_GUID(GARMIN_DETECT_GUID, 0x2c9c45c2L, 0x8e7d, 0x4c08, 0xa1, 0x2d, 0x81,
            0x6b, 0xba, 0xe7, 0x22, 0xc0);
#endif

#ifdef __MINGW32__  // do I need this because of mingw, or because I am running
                    // mingw under wine?
#ifndef GUID_CLASS_COMPORT
DEFINE_GUID(GUID_CLASS_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08,
            0x00, 0x3e, 0x30, 0x1f, 0x73);
#endif
#endif

extern int g_nCOMPortCheck;

struct device_data {
  std::string info;  // Free format info text, possibly empty
  std::string path;  // Complete /dev device path
  device_data(const std::string& p, const std::string& i) : info(i), path(p) {}
};

struct symlink {
  std::string path;
  std::string target;
  symlink(const std::string& p, const std::string& t) : path(p), target(t) {}
};

#ifdef __NetBSD__
static int isTTYreal(const char* dev) {
  if (strncmp("/dev/tty0", dev, 9) == 0) return 1;
  if (strncmp("/dev/ttyU", dev, 9) == 0) return 1;
  if (strcmp("/dev/gps", dev) == 0) return 1;
  return 0;
}

#elif defined(HAVE_LINUX_SERIAL_H) && defined(HAVE_SYS_STAT_H)

/** For /sysfs paths return corresponding /dev path, otherwise return dev */
static std::string device_path(const char* dev) {
    if (strstr(dev, "/sysfs/") != 0) return std::string(dev);
    std::string path(dev);
    return std::string("/dev") + path.substr(path.rfind('/'));
}

static int isTTYreal(const char* dev) {

  // Drop non-readable devices
  std::string path = device_path(dev);
  int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK | O_NOCTTY);
  if (fd < 0) return 0;

  // This original check does not work in kernels > 5.12.
  // See: https://github.com/torvalds/linux/commit/f64d74a59c476
  bool ok = false;
  struct serial_struct serinfo;
  if (ioctl(fd, TIOCGSERIAL, &serinfo) == 0) {
    ok = serinfo.type != PORT_UNKNOWN;
  }
  if (!ok) {
    // Accept any device with hardware lines DSR or CTS set.
    int modem_sts;
    if (ioctl(fd, TIOCMGET, &modem_sts) == 0) {
      ok = (modem_sts & (TIOCM_CTS | TIOCM_LE | TIOCM_DSR)) != 0;
    }
  }
  if (!ok) {
    // Accept standard ttyS0..ttyS3 + devices configured by udev:
    static const std::vector<std::regex> patterns = {
      std::regex("ttyS[0-3]$", std::regex_constants::ECMAScript),
      std::regex("ttyUSB", std::regex_constants::ECMAScript),
      std::regex("ttyACM", std::regex_constants::ECMAScript),
      std::regex("ttyAMA", std::regex_constants::ECMAScript)
    };
    for (auto re : patterns) {
      if (std::regex_search(dev, re)) {
          ok = true;
          break;
      }
    }
  }
  close(fd);
  return ok ? 1 : 0;
}

#else
static int isTTYreal(const char* dev) { return 1; }

#endif /* !NetBSD */

static bool isTTYreal(const device_data& data) {
  return isTTYreal(data.path.c_str());
}

#if defined(HAVE_DIRENT_H) && defined(HAVE_READLINK)

#define HAVE_SYSFS_PORTS

/** Return list of full paths to all possible ttys. */
static std::vector<std::string> get_device_candidates() {
  std::vector<std::string> devices;
  DIR* dir;
  struct dirent* ent;
  dir = opendir("/sys/class/tty");
  if (dir == 0) {
    wxLogWarning("Cannot open /sys/class/tty: %s", strerror(errno));
    return devices;
  }
  const std::string prefix("/dev/");
  for (ent = readdir(dir); ent; ent = readdir(dir)) {
    devices.push_back(prefix + ent->d_name);
  }
  closedir(dir);
  return devices;
}

/** Return all symlinks directly under /dev. */
static std::vector<struct symlink> get_all_links() {
  std::vector<struct symlink> links;
  DIR* dir;
  struct dirent* ent;
  dir = opendir("/dev");
  if (dir == 0) {
    wxLogError("Cannot open /dev: %s", strerror(errno));
    return links;
  }
  const std::string prefix("/dev/");
  for (ent = readdir(dir); ent; ent = readdir(dir)) {
    struct stat buf;
    const std::string path(prefix + ent->d_name);
    int r = lstat(path.c_str(), &buf);
    if (r == -1) {
      wxLogDebug("get_all_links: Cannot stat %s: %s", path.c_str(),
                 strerror(errno));
    } else if (S_ISLNK(buf.st_mode)) {
      char buff[PATH_MAX + 1];
      readlink(path.c_str(), buff, PATH_MAX);
      std::string target(buff);
      struct symlink link(path.c_str(), prefix + target);
      links.push_back(link);
    }
  }
  closedir(dir);
  return links;
}

/** Return list of full paths to active, serial ports based on /sys files */
static wxArrayString* EnumerateSysfsSerialPorts(void) {
  std::vector<std::string> ports;
  auto all_ports = get_device_candidates();
  wxLogDebug("Enumerate: found %d candidates", all_ports.size());
  for (auto p : all_ports) {
    if (isTTYreal(p.c_str())) ports.push_back(p);
  }
  wxLogDebug("Enumerate: found %d good ports", ports.size());
  const auto targets =
      std::unordered_set<std::string>(ports.begin(), ports.end());

  auto all_links = get_all_links();
  wxLogDebug("Enumerate: found %d links", all_links.size());
  for (auto l : all_links) {
    if (targets.find(l.target) != targets.end()) ports.push_back(l.path);
  }
  wxLogDebug("Enumerate: found %d devices", ports.size());

  auto wx_ports = new wxArrayString();
  for (auto p : ports) {
    wx_ports->Add(p);
  }
  return wx_ports;
}

#endif  // HAVE_DIRENT_H && defined(HAVE_READLINK)

#if defined(HAVE_LIBUDEV)

/** Return a single string of free-format device info, possibly empty. */
static std::string get_device_info(struct udev_device* ud) {
  std::string info;
  const char* prop = udev_device_get_property_value(ud, "ID_VENDOR");
  if (prop) info += prop;
  prop = udev_device_get_property_value(ud, "ID_MODEL");
  if (prop) info += std::string(" - ") + prop;
  return info;
}

/** Return list of device links pointing to dev. */
static std::vector<struct device_data> get_links(struct udev_device* dev,
                                                 const std::regex& exclude) {
  std::vector<struct device_data> items;
  std::string info(" link -> ");
  info += udev_device_get_devnode(dev);
  struct udev_list_entry* link = udev_device_get_devlinks_list_entry(dev);
  while (link) {
    const char* linkname = udev_list_entry_get_name(link);
    if (!std::regex_search(linkname, exclude)) {
      struct device_data item(linkname, info);
      items.push_back(item);
    }
    link = udev_list_entry_get_next(link);
  }
  return items;
}

static std::vector<struct device_data> enumerate_udev_ports(struct udev* udev) {
  struct udev_enumerate* enumerate = udev_enumerate_new(udev);
  udev_enumerate_add_match_subsystem(enumerate, "tty");
  udev_enumerate_scan_devices(enumerate);
  struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);

  const std::regex bad_ttys(".*tty[0-9][0-9]|^/dev/serial/.*|.*ttyS[0-9][0-9]");
  std::vector<struct device_data> items;
  struct udev_list_entry* entry;
  udev_list_entry_foreach(entry, devices) {
    const char* const path = udev_list_entry_get_name(entry);
    struct udev_device* device = udev_device_new_from_syspath(udev, path);
    const char* const devnode = udev_device_get_devnode(device);
    struct device_data item(devnode, get_device_info(device));
    if (!std::regex_search(devnode, bad_ttys) &&
        (isTTYreal(path) || item.info.length() > 0)) {
      items.push_back(item);
      auto links = get_links(device, bad_ttys);
      items.insert(items.end(), links.begin(), links.end());
    }
    udev_device_unref(device);
  }
  return items;
}

static wxArrayString* EnumerateUdevSerialPorts(void) {
  struct udev* udev = udev_new();
  auto dev_items = enumerate_udev_ports(udev);
  wxArrayString* ports = new wxArrayString;
  for (auto item : dev_items) {
    ports->Add((item.path + " - " + item.info).c_str());
  }
  return ports;
}

#endif  // HAVE_LIBUDEV

#ifdef __WXMSW__
static wxArrayString* EnumerateWindowsSerialPorts(void) {
  wxArrayString* preturn = new wxArrayString;
  /*************************************************************************
   * Windows provides no system level enumeration of available serial ports
   * There are several ways of doing this.
   *
   *************************************************************************/

  //    Method 1:  Use GetDefaultCommConfig()
  // Try first {g_nCOMPortCheck} possible COM ports, check for a default
  // configuration
  //  This method will not find some Bluetooth SPP ports
  for (int i = 1; i < g_nCOMPortCheck; i++) {
    wxString s;
    s.Printf(_T("COM%d"), i);

    COMMCONFIG cc;
    DWORD dwSize = sizeof(COMMCONFIG);
    if (GetDefaultCommConfig(s.fn_str(), &cc, &dwSize))
      preturn->Add(wxString(s));
  }

#if 0
    // Method 2:  Use FileOpen()
    // Try all 255 possible COM ports, check to see if it can be opened, or if
    // not, that an expected error is returned.

    BOOL bFound;
    for (int j=1; j<256; j++)
    {
        char s[20];
        sprintf(s, "\\\\.\\COM%d", j);

        // Open the port tentatively
        BOOL bSuccess = FALSE;
        HANDLE hComm = ::CreateFile(s, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

        //  Check for the error returns that indicate a port is there, but not currently useable
        if (hComm == INVALID_HANDLE_VALUE)
        {
            DWORD dwError = GetLastError();

            if (dwError == ERROR_ACCESS_DENIED ||
                    dwError == ERROR_GEN_FAILURE ||
                    dwError == ERROR_SHARING_VIOLATION ||
                    dwError == ERROR_SEM_TIMEOUT)
            bFound = TRUE;
        }
        else
        {
            bFound = TRUE;
            CloseHandle(hComm);
        }

        if (bFound)
        preturn->Add(wxString(s));
    }
#endif  // 0

  // Method 3:  WDM-Setupapi
  //  This method may not find XPort virtual ports,
  //  but does find Bluetooth SPP ports

  GUID* guidDev = (GUID*)&GUID_CLASS_COMPORT;

  HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;

  hDevInfo = SetupDiGetClassDevs(guidDev, NULL, NULL,
                                 DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

  if (hDevInfo != INVALID_HANDLE_VALUE) {
    BOOL bOk = TRUE;
    SP_DEVICE_INTERFACE_DATA ifcData;

    ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    for (DWORD ii = 0; bOk; ii++) {
      bOk = SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guidDev, ii, &ifcData);
      if (bOk) {
        // Got a device. Get the details.

        SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};
        bOk = SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, NULL, 0, NULL,
                                              &devdata);

        //      We really only need devdata
        if (!bOk) {
          if (GetLastError() ==
              122)  // ERROR_INSUFFICIENT_BUFFER, OK in this case
            bOk = true;
        }

        //      We could get friendly name and/or description here
        TCHAR fname[256] = {0};
        TCHAR desc[256] = {0};
        if (bOk) {
          BOOL bSuccess = SetupDiGetDeviceRegistryProperty(
              hDevInfo, &devdata, SPDRP_FRIENDLYNAME, NULL, (PBYTE)fname,
              sizeof(fname), NULL);

          bSuccess = bSuccess && SetupDiGetDeviceRegistryProperty(
                                     hDevInfo, &devdata, SPDRP_DEVICEDESC, NULL,
                                     (PBYTE)desc, sizeof(desc), NULL);
        }

        //  Get the "COMn string from the registry key
        if (bOk) {
          bool bFoundCom = false;
          TCHAR dname[256];
          HKEY hDeviceRegistryKey =
              SetupDiOpenDevRegKey(hDevInfo, &devdata, DICS_FLAG_GLOBAL, 0,
                                   DIREG_DEV, KEY_QUERY_VALUE);
          if (INVALID_HANDLE_VALUE != hDeviceRegistryKey) {
            DWORD RegKeyType;
            wchar_t wport[80];
            LPCWSTR cstr = wport;
            MultiByteToWideChar(0, 0, "PortName", -1, wport, 80);
            DWORD len = sizeof(dname);

            int result = RegQueryValueEx(hDeviceRegistryKey, cstr, 0,
                                         &RegKeyType, (PBYTE)dname, &len);
            if (result == 0) bFoundCom = true;
          }

          if (bFoundCom) {
            wxString port(dname, wxConvUTF8);

            //      If the port has already been found, remove the prior entry
            //      in favor of this entry, which will have descriptive
            //      information appended
            for (unsigned int n = 0; n < preturn->GetCount(); n++) {
              if ((preturn->Item(n)).IsSameAs(port)) {
                preturn->RemoveAt(n);
                break;
              }
            }
            wxString desc_name(desc, wxConvUTF8);  // append "description"
            port += _T(" ");
            port += desc_name;

            preturn->Add(port);
          }
        }
      }
    }  // for
  }    // if

  //  Search for Garmin device driver on Windows platforms

  HDEVINFO hdeviceinfo = INVALID_HANDLE_VALUE;

  hdeviceinfo = SetupDiGetClassDevs((GUID*)&GARMIN_DETECT_GUID, NULL, NULL,
                                    DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

  if (hdeviceinfo != INVALID_HANDLE_VALUE) {
    if (GarminProtocolHandler::IsGarminPlugged()) {
      wxLogMessage(_T("EnumerateSerialPorts() Found Garmin USB Device."));
      preturn->Add(_T("Garmin-USB"));  // Add generic Garmin selectable device
    }
  }

#if 0
    SP_DEVICE_INTERFACE_DATA deviceinterface;
    deviceinterface.cbSize = sizeof(deviceinterface);

    if (SetupDiEnumDeviceInterfaces(hdeviceinfo,
                    NULL,
                    (GUID *) &GARMIN_DETECT_GUID,
                    0,
                    &deviceinterface))
    {
        wxLogMessage(_T("Found Garmin Device."));

        preturn->Add(_T("GARMIN"));         // Add generic Garmin selectable device
    }
#endif  // 0
  return preturn;
}

#endif  // __WXMSW__


#if defined(OCPN_USE_SYSFS_PORTS) && defined(HAVE_SYSFS_PORTS)

wxArrayString* EnumerateSerialPorts(void) {
  return EnumerateSysfsSerialPorts();
}

#elif defined(OCPN_USE_UDEV_PORTS) && defined(HAVE_LIBUDEV)

wxArrayString* EnumerateSerialPorts(void) { return EnumerateUdevSerialPorts(); }

#elif defined(__OCPN__ANDROID__)

wxArrayString* EnumerateSerialPorts(void) {
  return androidGetSerialPortsArray();
}

#elif defined(__WXOSX__)

wxArrayString* EnumerateSerialPorts(void) {
  wxArrayString* preturn = new wxArrayString;
  char* paPortNames[MAX_SERIAL_PORTS];
  int iPortNameCount;

  memset(paPortNames, 0x00, sizeof(paPortNames));
  iPortNameCount = FindSerialPortNames(&paPortNames[0], MAX_SERIAL_PORTS);
  for (int iPortIndex = 0; iPortIndex < iPortNameCount; iPortIndex++) {
    wxString sm(paPortNames[iPortIndex], wxConvUTF8);
    preturn->Add(sm);
    free(paPortNames[iPortIndex]);
  }
  return preturn;
}

#elif defined(__WXMSW__)

wxArrayString* EnumerateSerialPorts(void) {
  return EnumerateWindowsSerialPorts();
}

#elif defined(OCPN_USE_NEWSERIAL)

wxArrayString* EnumerateSerialPorts(void) {
  wxArrayString* preturn = new wxArrayString;
  std::vector<serial::PortInfo> ports = serial::list_ports();
  for (auto it = ports.begin(); it != ports.end(); ++it) {
    wxString port(it->port);
    if (it->description.length() > 0 && it->description != "n/a") {
      port.Append(" - ");
      port.Append(wxString::FromUTF8((it->description).c_str()));
    }
    preturn->Add(port);
  }
  return preturn;
}

#else

#error "Cannot enumerate serial ports (missing libudev.h?)"

#endif  // outermost if - elif - else
