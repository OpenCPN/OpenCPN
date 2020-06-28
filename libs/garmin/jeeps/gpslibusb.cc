/*
    Physical/OS USB layer to talk to libusb.

    Copyright (C) 2004, 2005, 2006, 2007, 2008  Robert Lipe, robertlipe@usa.net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */


#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#if HAVE_CONFIG_H
#include "config.h"
#endif
#if HAVE_LIBUSB_1_0
#  if __APPLE__
// We use our own libusb.
#    include "mac/libusb/libusb.h"
#  else
#    include <libusb-1.0/libusb.h>
#  endif
#include "../defs.h"
#include "garminusb.h"
#include "gpsdevice.h"
#include "gpsusbcommon.h"
#include "../garmin_device_xml.h"

#define GARMIN_VID 0x91e

/* This is very sensitive to timing; libusb and/or the unit is kind of
 * sloppy about not obeying packet boundaries.  If this is too high, the
 * multiple packets responding to the device inquiry will be glommed into
 * one packet and we'll misparse them.  If it's too low, we'll get partially
 * satisfied reads.  It turns out this isn't terrible because we still end
 * up with DLE boundings and the upper layers (which are used to doing frame
 * coalescion into packets anyway because of their serial background) will
 * compensate.
 */
#define TMOUT_I 5000 /*  Milliseconds to timeout intr pipe access. */
#define TMOUT_B 5000 /*  Milliseconds to timeout bulk pipe access. */

typedef struct {
  unsigned product_id;
} libusb_unit_data;

/*
 * TODO: this should all be moved into libusbdata in gpslibusb.h,
 * allocated once here in gusb_start, and deallocated at the end.
 */
static unsigned char gusb_intr_in_ep;
static unsigned char gusb_bulk_out_ep;
static unsigned char gusb_bulk_in_ep;

static bool libusb_successfully_initialized{false};
static libusb_device_handle* udev{nullptr};
static int garmin_usb_scan(libusb_unit_data*, int);
static const gdx_info* gdx;

static int gusb_libusb_get(garmin_usb_packet* ibuf, size_t sz);
static int gusb_libusb_get_bulk(garmin_usb_packet* ibuf, size_t sz);
static int gusb_teardown(gpsdevh* dh, bool exit_lib);
static int gusb_libusb_send(const garmin_usb_packet* opkt, size_t sz);

static gusb_llops_t libusb_llops = {
  gusb_libusb_get,
  gusb_libusb_get_bulk,
  gusb_libusb_send,
  gusb_teardown,
  0
};

#if __linux__
static
char** os_get_garmin_mountpoints()
{
  // Hacked for testing.
  return nullptr;
}
#elif __APPLE__
// In fantasy land, we'd query iokit for enumerated devices of the Garmin
// vendor ID and match that against the mounted device table.  In practical
// matters, that's crazy complex and this is where the devices seems to always
// get mounted...
char** os_get_garmin_mountpoints()
{
  char** dlist = (char**) xcalloc(2, sizeof *dlist);
  dlist[0] = xstrdup("/Volumes/GARMIN");
  dlist[1] = nullptr;
  return dlist;
}
#else
char** os_get_garmin_mountpoints()
{
  return nullptr;
}
#endif


static int
gusb_libusb_send(const garmin_usb_packet* opkt, size_t sz)
{
  auto buf = const_cast<unsigned char*>(&opkt->dbuf[0]);
  int transferred;

  int ret = libusb_bulk_transfer(udev, gusb_bulk_out_ep, buf, sz,
                                 &transferred, TMOUT_B);
  if (ret != LIBUSB_SUCCESS) {
    fatal("libusb_bulk_transfer failed. '%s'\n",
          libusb_strerror(static_cast<enum libusb_error>(ret)));
  }
  if (transferred != (int) sz) {
    warning("Bad cmdsend transferred %d sz %lud\n", transferred,
            (unsigned long) sz);
  }

  return transferred;
}

static int
gusb_libusb_get(garmin_usb_packet* ibuf, size_t sz)
{
  unsigned char* buf = &ibuf->dbuf[0];
  int transferred;

  int ret = libusb_interrupt_transfer(udev, gusb_intr_in_ep, buf, sz,
                                      &transferred, TMOUT_I);
// UGGH: expected return is error code < 0 or # of bytes transferred.
// assume libusb return values are all <= 0, transferred >= 0.
  if (ret != LIBUSB_SUCCESS) {
    assert(ret < 0);
    return ret;
  }
  return transferred;
}

static int
gusb_libusb_get_bulk(garmin_usb_packet* ibuf, size_t sz)
{
  unsigned char* buf = &ibuf->dbuf[0];
  int transferred;

  int ret = libusb_bulk_transfer(udev, gusb_bulk_in_ep, buf, sz,
                                 &transferred, TMOUT_B);
// UGGH: expected return is error code < 0 or # of bytes transferred.
// assume libusb return values are all <= 0, transferred >= 0.
  if (ret != LIBUSB_SUCCESS) {
    assert(ret < 0);
    return ret;
  }
  return transferred;
}


static int
gusb_teardown(gpsdevh* dh, bool exit_lib)
{
  if (udev != nullptr) {
    int ret = libusb_release_interface(udev, 0);
    if (ret != LIBUSB_SUCCESS) {
      warning("libusb_release_interface failed: %s\n",
              libusb_strerror(static_cast<enum libusb_error>(ret)));
    }
    libusb_close(udev);
    /* In the worst case, we leak a little bit of memory
     * when called via the atexit handler.  That's not too
     * terrible.
     */
    if (nullptr != dh) {
      xfree(dh);
    }
    udev = nullptr;
  }
  if (exit_lib && libusb_successfully_initialized) {
    libusb_exit(nullptr);
    libusb_successfully_initialized = false;
  }
  return 0;
}

static void
gusb_atexit_teardown()
{
  gusb_teardown(nullptr, true);
}


/*
 * This was a function of great joy to discover...and even greater to maintain.
 *
 * It turns out that as of 5/2006, every Garmin USB product has a problem
 * where the device does not reset the data toggles after a configuration
 * set.   After a reset, the toggles both match.  So we tear through the
 * conversation and life is good.  Unfortunately, the second time through,
 * if we had an odd number of transactions in the previous conversation,
 * we send a configuration set and reset the toggle on the HCI but the
 * toggle on the device's end of the pipe is now out of whack which means
 * that the subsequent transaction will hang.
 *
 * This isn't a problem in Windows since the configuration set is done only
 * once there.
 *
 * This code has been tested in loops of 1000 cycles on Linux and OS/X and
 * it seems to cure this at a mere cost of complexity and startup time.  I'll
 * be delighted when all the firmware gets revved and updated and we can
 * remove this.
 *
 * 9/2008 But wait, there's more.   The very toggle reset that we *had* to
 * implement in 2006 to make non-Windows OSes work actually locks up version
 * 2.70 of the Venture HC.   On that model, the second product request
 * (you know, the one that we *use*, locks that device's protocol stack
 * after the RET2INTR that immediately follows the REQBLK (and why is it
 * telling us to go into bulk mode followed by an immediate EOF, anyway?)
 * that follows the request for product ID.   100% reproducible on Mac and
 * Linux.    Of course, we don't see this on the Windows system because
 * we don't have to jump through hoops to clear the spec-violating out
 * of state toggles there because those systems see only one configuration
 * set ever.
 *
 * Grrrr!
 */
unsigned
gusb_reset_toggles()
{
  static const unsigned char  oinit[12] =
  {0, 0, 0, 0, GUSB_SESSION_START, 0, 0, 0, 0, 0, 0, 0};
  static const unsigned char  oid[12] =
  {20, 0, 0, 0, 0xfe, 0, 0, 0, 0, 0, 0, 0};
  garmin_usb_packet iresp;
  int t;
  unsigned rv = 0;

  /* Start off with three session starts.
   * #1 resets the bulk out toggle.  It may not make it to the device.
   * #2 resets the the intr in toggle.  It will make it to the device
   *	since #1 reset the the bulk out toggle.   The device will
   *      respond on the intr in pipe which will clear its toggle.
   * #3 actually starts the session now that the above are both clear.
   */

  gusb_cmd_send((const garmin_usb_packet*) oinit, sizeof(oinit));
  gusb_cmd_send((const garmin_usb_packet*) oinit, sizeof(oinit));
  gusb_cmd_send((const garmin_usb_packet*) oinit, sizeof(oinit));

  t = 10;
  while (true) {
    le_write16(&iresp.gusb_pkt.pkt_id, 0);
    le_write32(&iresp.gusb_pkt.datasz, 0);
    le_write32(&iresp.gusb_pkt.databuf, 0);

    gusb_cmd_get(&iresp, sizeof(iresp));

    if ((le_read16(iresp.gusb_pkt.pkt_id) == GUSB_SESSION_ACK) &&
        (le_read32(iresp.gusb_pkt.datasz) == 4)) {
      break;
    }
    if (t-- <= 0) {
      fatal("Could not start session in a reasonable number of tries.\n");
    }
  }

  /*
   * Now that the bulk out and intr in packets are good, we send
   * a product ID.    On devices that respond totally on the intr
   * pipe, this does nothing interesting, but on devices that respond
   * on the bulk pipe this will reset the toggles on the bulk in.
   */

  t = 10;
  gusb_cmd_send((const garmin_usb_packet*) oid, sizeof(oid));
  while (true) {
    le_write16(&iresp.gusb_pkt.pkt_id, 0);
    le_write32(&iresp.gusb_pkt.datasz, 0);
    le_write32(&iresp.gusb_pkt.databuf, 0);

    gusb_cmd_get(&iresp, sizeof(iresp));

    if (le_read16(iresp.gusb_pkt.pkt_id) == 0xff) {
      rv = le_read16(iresp.gusb_pkt.databuf+0);
    }

    if (le_read16(iresp.gusb_pkt.pkt_id) == 0xfd) {
      return rv;
    }
    if (t-- <= 0) {
      fatal("Could not start session in a reasonable number of tries.\n");
    }
  }
  return 0;
}

void
garmin_usb_start(struct libusb_device* dev,
                 struct libusb_device_descriptor* desc, libusb_unit_data* lud)
{
  int ret;

  if (udev != nullptr) {
    return;
  }
  ret  = libusb_open(dev, &udev);
  if (ret != LIBUSB_SUCCESS) {
    fatal("libusb_open failed: %s\n",
          libusb_strerror(static_cast<enum libusb_error>(ret)));
  }
  assert(udev != nullptr);
  /*
   * Hrmph.  No iManufacturer or iProduct headers....
   */


#if __APPLE__
  // On Leopard, if we don't do an explicit set_configuration, some
  // devices will work only the first time after a reset.
  ret = libusb_set_configuration(udev, 1);
  if (ret != LIBUSB_SUCCESS) {
    fatal("libusb_set_configuration failed: %s\n",
          libusb_strerror(static_cast<enum libusb_error>(ret)));
  };
#endif

#if 0
  if (usb_set_configuration(udev, 1) < 0) {
#if __linux__
    char drvnm[128];
    drvnm[0] = 0;
    /*
     * Most Linux distributions ship a slightly broken
     * kernel driver that bonds with the hardware.
     */
    usb_get_driver_np(udev, 0, drvnm, sizeof(drvnm)-1);
    fatal("usb_set_configuration failed, probably because kernel driver '%s'\n is blocking our access to the USB device.\n"
          "For more information see https://www.gpsbabel.org/os/Linux_Hotplug.html\n", drvnm);
#else

    fatal("usb_set_configuration failed: %s\n", usb_strerror());
#endif
  }
#endif
  ret = libusb_claim_interface(udev, 0);
  if (ret != LIBUSB_SUCCESS) {
    fatal("libusb_claim_interface failed: %s\n",
          libusb_strerror(static_cast<enum libusb_error>(ret)));
  }

  /*
   * FIXME: Shouldn't this be wMaxPacketSize from the
   * endpoint descriptor for the bulk out endpoint?
   *
   */
  libusb_llops.max_tx_size = desc->bMaxPacketSize0;

  /*
   * About 5% of the time on OS/X (Observed on 10.5.4 on Intel Imac
   * with Venture HC) we get a dev with a valid vendor ID, but no
   * associated configuration.  I was unable to see a single instance
   * of this on a 276, a 60CS, a 60CSx, an SP310, or an Edge 305, leading
   * me to think this is some kind of bug in the Venture HC.
   *
   * Rather than crash, we at least print
   * a nastygram.  Experiments with retrying various USB ops brought
   * no joy, so just call fatal and move on.
   */
  struct libusb_config_descriptor* config;
  ret = libusb_get_active_config_descriptor(dev, &config);
  if (ret != LIBUSB_SUCCESS) {
    fatal("libusb_get_active_config_descriptor failed: %s\n",
          libusb_strerror(static_cast<enum libusb_error>(ret)));
  }

  if (config == nullptr) {
    fatal("Found USB device with no configuration.\n");
  }

  for (int i = 0; i < config->bNumInterfaces; ++i) {
    const struct libusb_interface* interface = &config->interface[i];
    for (int j = 0; j < interface->num_altsetting; ++j) {
      const struct libusb_interface_descriptor* altsetting = &interface->altsetting[j];
      /*
       * FIXME: Since we never use libusb_set_interface_alt_setting()
       * shouldn't we only look at the default interface descriptor, i.e. 
       * the one that has a bAlternateSetting of 0 and/or the one
       * that has index 0?
       * From the USB spec:
       * "The default setting for an interface is always alternate setting zero."
       */
      for (int k = 0; k < altsetting->bNumEndpoints; ++k) {
        const struct libusb_endpoint_descriptor* ep = &altsetting->endpoint[k];

        switch (ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) {
#define EA(x) x & LIBUSB_ENDPOINT_ADDRESS_MASK
        case LIBUSB_TRANSFER_TYPE_BULK:
          if (ep->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) {
            gusb_bulk_in_ep = (EA(ep->bEndpointAddress)) | LIBUSB_ENDPOINT_IN;
          } else {
            gusb_bulk_out_ep = EA(ep->bEndpointAddress);
          }
          break;
        case LIBUSB_TRANSFER_TYPE_INTERRUPT:
          if (ep->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) {
            gusb_intr_in_ep = (EA(ep->bEndpointAddress)) | LIBUSB_ENDPOINT_IN;
          }
          break;
        }

      }
    }
  }

  libusb_free_config_descriptor(config);

  /*
   *  Zero is the configuration endpoint, so if we made it through
   * that loop without non-zero values for all three, we're hosed.
   */
  if (gusb_intr_in_ep && gusb_bulk_in_ep && gusb_bulk_out_ep) {
    lud->product_id = gusb_reset_toggles();
    switch (lud->product_id) {
    // Search for "Venture HC" for more on this silliness..
    // It's a case instead of an 'if' because I have a
    // feeling there are more affected models either
    // on the market or on the way.
    case 695:
      break;   // Venture HC
    case 941:
      break;   // Venture HC, Japanese version.
    case 957:
      break;   // Legend H
    case 285:
      break;   // GPSMap 276C/4.80
    case 402:
      break;   // GPSMap 396C/4.50
    case 1095:
      break;  // GPS72H/2.30
    default:
      gusb_syncup();
    }
    return;
  }

  fatal("Could not identify endpoints on USB device.\n"
        "Found endpoints Intr In 0x%x Bulk Out 0x%x Bulk In %0xx\n",
        gusb_intr_in_ep, gusb_bulk_out_ep, gusb_bulk_in_ep);
}

static
int garmin_usb_scan(libusb_unit_data* lud, int req_unit_number)
{
  int found_devices = 0;

  libusb_device** devs;
  ssize_t cnt = libusb_get_device_list(nullptr, &devs);

  for (int i = 0; i < cnt; ++i) {
    /*
     * Exclude Mass Storage devices (CO, OR, Nuvi, etc.)
     * from this scan.
     * At least on Mac, bDeviceClass isn't
     * USB_MASS_STORAGE as it should be (perhaps because
     * the storage driver has already bound to it?) so
     * we fondle only the proprietary class devices.
     */
    struct libusb_device_descriptor desc;
    int ret = libusb_get_device_descriptor(devs[i], &desc);
    if (ret != LIBUSB_SUCCESS) {
      fatal("libusb_get_device_descriptor failed: %s\n",
            libusb_strerror(static_cast<enum libusb_error>(ret)));
    }
    if ((desc.idVendor == GARMIN_VID) &&
        (desc.bNumConfigurations > 0) &&
        (desc.bDeviceClass == LIBUSB_CLASS_VENDOR_SPEC)) {
      if (req_unit_number < 0) {
        garmin_usb_start(devs[i], &desc, lud);
        /*
         * It's important to call _close
         * here since the bulk/intr models
         * may have a "dangling" packet that
         * needs to be drained.
         */
        gusb_close(nullptr, false);
      } else if (req_unit_number == found_devices) {
        garmin_usb_start(devs[i], &desc, lud);
      }
      found_devices++;
    }
  }
  libusb_free_device_list(devs, 1);

  if (req_unit_number < 0) {
    gusb_list_units();
    exit(0);
  }

  if (0 == found_devices) {
    /* It's time for Plan B.  The user told us to use
     * Garmin Protocol in device "usb:" but it's possible
     * that they're talking to one of the dozens of models
     * that is wants to read and write GPX files on a
     * mounted drive.  Try that now.
     */
    char** dlist = os_get_garmin_mountpoints();
    gdx = gdx_find_file(dlist);
    if (gdx != nullptr) {
      return 1;
    }
    /* Plan C. */
    fatal("Found no Garmin USB devices.\n");
  } else if (req_unit_number >= found_devices) {
    fatal("usb unit number(%d) too high.\n"
          "The unit number must be either\n"
          "1) nonnegative and less than the number of garmin devices found(%d), or\n"
          "2) negative to list the garmin devices found.\n",
          req_unit_number, found_devices);
  } else {
    return 1;
  }
}


int
gusb_init(const char* portname, gpsdevh** dh)
{
  int req_unit_number = 0;
  auto lud = (libusb_unit_data*) xcalloc(sizeof(libusb_unit_data), 1);

  *dh = (gpsdevh*) lud;

//  To enable debug logging
//  set the LIBUSB_DEBUG environment variable to the log level, or
//  libusb_set_option(nullptr, LIBUSB_OPTION_LOG_LEVEL, 99);
  int ret = libusb_init(nullptr);
  if (ret != LIBUSB_SUCCESS) {
    fatal("libusb_init failed: %s\n",
          libusb_strerror(static_cast<enum libusb_error>(ret)));
  }
  libusb_successfully_initialized = true;
  /* you can't unregister an exit handler, and */
  /* they are called once for ever time they are registered. */
  static bool exit_handler_registered{false};
  if (!exit_handler_registered) {
    atexit(gusb_atexit_teardown);
    exit_handler_registered = true;
  }
  gusb_register_ll(&libusb_llops);

  /* if "usb:N", read "N" to be the unit number. */
  if (strlen(portname) > 4) {
    if (0 == strcmp(portname+4, "list")) {
      req_unit_number = -1;
    } else {
      req_unit_number = atoi(portname + 4);
    }
  }
  return garmin_usb_scan(lud, req_unit_number);
}

#endif /* HAVE_LIBUSB_1_0 */
