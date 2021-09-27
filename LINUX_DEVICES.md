OpenCPN devices under Linux
---------------------------

Opencpn runs at user privilege.  This means that in order to
read GPS input data and/or write autopilot output data, the serial
devices to be used must exhibit read and write permission for the
user in question.

Furthermore, typical device names like */dev/ttyUSB0* or */dev/ttyACM0*
are not stable over a suspend cycle. A device might change name to
*/dev/ttyUSB1* when the laptop lid is opened after being closed.

These aspects, permissions and dynamic names, is about udev which handles
the devices in all modern Linux distros.


## Device access.

Most distros have serial device access restricted to root or users in a
specific group. To check, list e. g. the /dev/ttyS0 device:

    $ ls -l /dev/ttyS0
    crw-rw---- 1 root dialout 4, 64 Jul 17 09:42 /dev/ttyS0

Here, the device is owned by root but can also be accessed by any user in
the dialout group. To get access to this device, add your user to the
group owner of the device, something like

    $ sudo usermod -aG dialout $USER

and logout and in again (or reboot). Test the permissions using

    $ stty -F /dev/ttyXXX ispeed 4800

which gives an error message if it doesn't work.

If the group permissions doesn't make the trick, a udev rule could be used
instead to give all users read/write access to the device. Create a file
called */etc/udev/rules.d/70-serial-opencpn.rules* like

    KERNEL=="ttyUSB*", MODE="0666"
    KERNEL=="ttyACM*", MODE="0666"
    KERNEL=="ttyS*", MODE="0666"

A reboot might be needed for it to take effect.  Normally, just one of the
three lines is required depending on device used, the rest could be removed.
Test the permissions (above).

_flatpak_ users need to install the udev rule, the group permissions can
not be used from the flatpaked opencpn.


## Fixed device names

If your device disappears after closing and opening the laptop lid you need
to arrange for a fixed device name. This is done with a udev rule. My rule
is named */etc/udev/rules.d/50-nmea.rules* and looks like:

    $ cat /etc/udev/rules.d/50-nmea.rules
    ATTRS{idVendor}=="067b", ATTRS{idProduct}=="2303", ACTION=="add",SYMLINK+="nmea"

which basically tells udev that any device from vendor 067b with product
id 2303 should have an additional device name */dev/nmea*, which is the
name used in opencpn. This name will always be the same.

The vendor and product id could be retrieved using lsusb:

    $ lsusb
    ...
    Bus 004 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub
    Bus 003 Device 003: ID 056a:033c Wacom Co., Ltd
    Bus 003 Device 002: ID 046d:c03d Logitech, Inc. M-BT96a Pilot Optical Mouse

which for example shows that the mouse has vendor 046d and product id c03d.
Another way is to use udevadm. It creates a lot of output, so do something
like:

    $ udevadm info attribute-walk /dev/ttyUSB0 > walk.log

    $ grep idProduct walk.log
    ATTRS{idProduct}=="0938"

    $ grep idVendor walk.log
    ATTRS{idVendor}=="1781"

Udev rules is a large topic; please refer to more general info for more.


## Testing

Test the port like:

      $ stty -F /dev/ttyXXX ispeed 4800
      $ cat </dev/ttyXXX

replacing ttyXXX with the filename of the port.  This will probably be
either /dev/ttyUSB0 or /dev/ttyS0.  When you run this command, you
should see text lines like:

    $GPGGA,092750.000,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M,,*76
    $GPGSA,A,3,10,07,05,02,29,04,08,13,,,,,1.72,1.03,1.38*0A
    $GPGSV,3,1,11,10,63,137,17,07,61,098,15,05,59,290,20,08,54,157,30*70
    $GPGSV,3,2,11,02,39,223,19,13,28,070,17,26,23,252,,04,14,186,14*79

If you don't see this, you may have OS-level problems with your serial
support, but more likely have the wrong device or permissions. Look again.
