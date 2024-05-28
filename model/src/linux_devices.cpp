/***************************************************************************
 *   Copyright (C) 2011 - 2024 Alec Leamas                                 *
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

/** \file linux_devices.cpp Implement linux_devices.h. */

#include "config.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

#include <stdlib.h>

#ifndef HAVE_UNISTD_H
#error linux_devices requires unistd.h to be available
#endif
#include <unistd.h>

#include <sys/sysmacros.h>
#include <sys/stat.h>

#ifndef HAVE_LIBUSB_10
#error linux_devices requires libusb-1.0 to be available
#endif
#include <libusb.h>

#include "model/linux_devices.h"
#include "model/logger.h"
#include "model/ocpn_utils.h"

typedef struct usbdata {
  std::string vendor_id;
  std::string product_id;
  std::string vendor;
  std::string product;
  std::string serial_nr;

  usbdata(std::string v, std::string p, const char* s = 0)
      : vendor_id(v), product_id(p), serial_nr(s ? s : "") {}
  bool is_ok() { return vendor_id.length() > 0; }
} usbdata;

static const int DONGLE_VENDOR = 0x1547;
static const int DONGLE_PRODUCT = 0x1000;

static const char* const DONGLE_RULE = R"--(
ATTRS{idVendor}=="@vendor@", ATTRS{idProduct}=="@product@", MODE="0666"
)--";

static const char* const DEVICE_RULE = R"--(
ATTRS{idVendor}=="@vendor@", ATTRS{idProduct}=="@product@", \
    MODE="0666", SYMLINK+="@symlink@"
)--";

static const char* const DEVICE_RULE_TTYS = R"--(
KERNEL=="ttyS@s_index@", MODE="0666"
)--";

static const char* const DONGLE_RULE_NAME = "65-ocpn-dongle.rules";

/** Add more data available using libusb. */
static void ReadUsbdata(libusb_device* dev, libusb_device_handle* handle,
                        usbdata* data) {
  struct libusb_device_descriptor desc;
  libusb_get_device_descriptor(dev, &desc);

  unsigned char buff[256];
  int r;
  if (desc.iProduct) {
    r = libusb_get_string_descriptor_ascii(handle, desc.iProduct, buff,
                                           sizeof(buff));
    if (r > 0) data->product = reinterpret_cast<char*>(buff);
  }
  if (desc.iManufacturer) {
    r = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, buff,
                                           sizeof(buff));
    if (r > 0) data->vendor = reinterpret_cast<char*>(buff);
  }
  if (desc.iSerialNumber) {
    r = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, buff,
                                           sizeof(buff));
    if (r > 0) data->serial_nr = reinterpret_cast<char*>(buff);
  }
}

static int TryOpen(int vendorId, int productId, usbdata* data = 0) {
  libusb_context* ctx = 0;
  int r = libusb_init(&ctx);
  if (r != 0) {
    auto e = static_cast<libusb_error>(r);
    WARNING_LOG << "Cannot initialize libusb: " << libusb_strerror(e);
    return LIBUSB_ERROR_NOT_SUPPORTED;
  }
  libusb_device** device_list;
  ssize_t size = libusb_get_device_list(ctx, &device_list);
  if (size < 0) {
    auto e = static_cast<libusb_error>(size);
    DEBUG_LOG << "Cannot get usb devices list: " << libusb_strerror(e);
    return LIBUSB_ERROR_NOT_SUPPORTED;
  }
  r = LIBUSB_ERROR_INVALID_PARAM;
  for (auto dev = device_list; *dev; dev++) {
    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(*dev, &desc);
    if (desc.idVendor != vendorId || desc.idProduct != productId) {
      continue;
    }
    libusb_device_handle* dev_handle;
    r = libusb_open(*dev, &dev_handle);
    if (r >= 0) {
      if (data) {
        ReadUsbdata(*dev, dev_handle, data);
      }
      libusb_close(dev_handle);
    }
    break;
  }
  libusb_free_device_list(device_list, 1);
  DEBUG_LOG << "Nothing found for " << vendorId << ":" << productId;
  libusb_exit(0);
  return r;
}

static int TryOpen(const std::string vendorId, const std::string productId,
                   usbdata* data = 0) {
  int v;
  int p;
  std::istringstream(vendorId) >> std::hex >> v;
  std::istringstream(productId) >> std::hex >> p;
  return TryOpen(v, p, data);
}

bool IsDonglePermissionsWrong() {
  int rc = TryOpen(DONGLE_VENDOR, DONGLE_PRODUCT);
  DEBUG_LOG << "Probing dongle permissions, result: " << rc;
  return rc == LIBUSB_ERROR_ACCESS;
}

bool IsDevicePermissionsOk(const char* path) {
  int r = access(path, R_OK | W_OK);
  if (r < 0) {
    INFO_LOG << "access(3) fails on: " << path << ": " << strerror(errno);
  }
  return r == 0;
}

/** Look for vendorId/ProductId in uevent file. */
static usbdata ParseUevent(std::istream& is) {
  std::string line;
  while (std::getline(is, line)) {
    if (line.find('=') == std::string::npos) {
      continue;
    }
    auto tokens = ocpn::split(line.c_str(), "=");
    if (tokens[0] != "PRODUCT") {
      continue;
    }
    if (line.find("/") == std::string::npos) {
      INFO_LOG << "invalid product line: " << line << "(ignored)";
      continue;
    }
    tokens = ocpn::split(tokens[1].c_str(), "/");
    std::stringstream ss1;
    ss1 << std::setfill('0') << std::setw(4) << tokens[0];
    std::stringstream ss2;
    ss2 << std::setfill('0') << std::setw(4) << tokens[1];
    return usbdata(ss1.str(), ss2.str());
  }
  return usbdata("", "");
}

static usbdata GetDeviceUsbdata(const char* path) {
  // Get real path for node in /sys corresponding to path in /dev
  struct stat st;
  int r = stat(path, &st);
  if (r < 0) {
    MESSAGE_LOG << "Cannot stat: " << path << ": " << strerror(errno);
    return usbdata(0, 0);
  }
  std::stringstream syspath("/sys/dev/char/");
  syspath << "/sys/dev/char/" << major(st.st_rdev) << ":" << minor(st.st_rdev);
  char buff[PATH_MAX];
  if (!realpath(syspath.str().c_str(), buff)) {
      wxLogDebug("Error resolving link %s: %s", syspath.str().c_str(),
                 strerror(errno));
  }
  std::string real_path(buff);

  // Get the uevent file in each parent dir and parse it.
  while (real_path.length() > 0) {
    auto uevent_path = real_path + "/uevent";
    if (access(uevent_path.c_str(), R_OK) >= 0) {
      std::ifstream is(uevent_path);
      auto data = ParseUevent(is);
      if (data.is_ok()) {
        // Add missing pieces (descriptions...) using libusb
        TryOpen(data.vendor_id, data.product_id, &data);
        return data;
      }
    }
    // Drop last part of filename
    size_t last_slash = real_path.rfind('/');
    last_slash = last_slash == std::string::npos ? 0 : last_slash;
    real_path = real_path.substr(0, last_slash);
  }
  return usbdata("", "");
}

static std::string TmpRulePath(const char* name) {
  std::string tmpdir =
      getenv("XDG_CACHE_HOME") ? getenv("XDG_CACHE_HOME") : "/tmp";
  tmpdir += "/udevXXXXXX";

  char dirpath[128] = {0};
  strncpy(dirpath, tmpdir.c_str(), sizeof(dirpath) - 1);
  if (!mkdtemp(dirpath)) {
    WARNING_LOG << "Cannot create tempdir: " << strerror(errno);
    MESSAGE_LOG << "Using /tmp";
    strcpy(dirpath, "/tmp");
  }
  std::string path(dirpath);
  path += "/";
  path += name;
  return path;
}

std::string MakeUdevLink() {
  for (char ch : {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'}) {
    std::stringstream ss;
    ss << "/etc/udev/rules.d/65-opencpn" << ch << ".rules";
    if (!ocpn::exists(ss.str())) {
      std::string path(ss.str());
      ocpn::replace(path, "/etc/udev/rules.d/65-", "");
      ocpn::replace(path, ".rules", "");
      return path;
    }
  }
  WARNING_LOG << "Too many opencpn device rules found (10). Giving up.";
  return "";
}

static std::string CreateTmpfile(const std::string& contents,
                                 const char* name) {
  auto path = TmpRulePath(name);
  std::ofstream of(path);
  of << contents;
  of.close();
  if (of.bad()) {
    WARNING_LOG << "Cannot write to temp file: " << path;
  }
  return path;
}

static std::string CreateUdevRule(const std::string& device, usbdata data,
                                  const char* symlink) {
  std::string rule(DEVICE_RULE);
  if (device.find("ttyS") != std::string::npos) {
    rule = std::string(DEVICE_RULE_TTYS);
    auto index(device.substr(device.find("ttyS") + strlen("ttyS")));
    ocpn::replace(rule, "@s_index@", index);
  } else {
    ocpn::replace(rule, "@vendor@", data.vendor_id);
    ocpn::replace(rule, "@product@", data.product_id);
  }
  ocpn::replace(rule, "@symlink@", symlink);
  std::string name(symlink);
  name.insert(0, "65-");
  name += ".rules";
  return CreateTmpfile(rule, name.c_str());
}

std::string GetDongleRule() {
  std::string rule(DONGLE_RULE);
  std::ostringstream oss;

  oss << std::setw(4) << std::setfill('0') << std::hex << DONGLE_VENDOR;
  ocpn::replace(rule, "@vendor@", oss.str());
  oss.str("");
  oss << std::setw(4) << std::setfill('0') << std::hex << DONGLE_PRODUCT;
  ocpn::replace(rule, "@product@", oss.str());
  return CreateTmpfile(rule, DONGLE_RULE_NAME);
}

std::string GetDeviceRule(const char* device, const char* symlink) {
  usbdata data = GetDeviceUsbdata(device);
  auto path = CreateUdevRule(device, data, symlink);
  return path;
}
