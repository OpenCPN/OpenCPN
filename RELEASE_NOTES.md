OpenCPN 5.14 Release Notes
==========================

Major updates in this version:
------------------------------

- The Data Monitor (DM) has got some polish. Together with work on the VDR
  plugin it is now possible to replay data recorded by the DM using the VDR
  plugin.

- Improved performance for chart database update process.

- Extensive internal code quality reviews, adding a large amount of
  descriptive commentary to the code base. Refactoring in order to
  decrease internal couplings.  Improves developer accessibility
  and maintenance efficiency.

Known issues:
------------

- Defining two signalK drivers using the same address/port might
  lead to a crash -  see #5119


Issues closed since 5.12.4:
---------------------------

#5134 Default world map location seems to be confused
#5132 No position recieved using signalk data
#5130 The mouse pointer changes very early to a green arrow
#5123 Chart downloader can't type some language special letters made by Windows
#5122 AIS, MOB, EPIRB target query formatting problem
#5119 segmentation fault linux signalK
#5117 Options dialog changes second canvas chart type
#5107 OpenCPN fails to terminate, top-left toolbar not shown...
#5104 OpenCPN freezes after 10 times opening and closing the optiondialog
#5096 GDAL library is not "thread-safe"
#5094 opencpn-glutil: linux: Wrong installation directory
#5092 Humidity in Nmea0183 MDA
#5091 tests fails in debian pbuilder
#5088 Not able to filter out NMEA0183 "ZDA"
#5086 OpenCPN send N2k Message in a Format that is not self acceptance by itself
#5084 data/copyright is outdated
#5083 Spurious  console output
#5080 SART Alarm Not Properly Acknowledged or Silenced (Repeating Popup)
#5075 demo plugin: Incompatible license
#5074 Dashboard Sunrise/Sunset instrument has UTC time even if it says LCL
#5073 tests: failures
#5070 Data monitor: polish needed
#5068 OCPN crash while creating a route. (Mouse point related?)
#5067 Weather Routing Bug - Save Route Options
#5066 _build-flatpak-x86-2506_ and also _build-flatpak-arm64-2506_ FTB
#5064 Data Monitor (DM) : blank log lines
#5063 When closing window with alt + F4 after closing polar plugin then OpenCPN thinks it is crashing
#5062 Rename local names from "OpenCPN *version number* *some other number*" to just "OpenCPN"
#5061 Datamonitor : N2K PGN is not shown when it comes alone in a long distance
#5060 linux/flatpak: metainfo: Missing info links
