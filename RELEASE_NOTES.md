OpenCPN 5.14 Release Notes
==========================

Major user-facing updates in this version:
------------------------------------------
  - Allow NMEA0183 sentences without Checksum. If present it must though be
    correct.
  - Correct fault in GPX file import with file name containing UTF-8 chars,
    Android.
  - DataMonitor: Filter selection bugfix and several feature updates, various
    ui clean up.
  - DataMonitor: Add capability to replay DM recordings using the VDR plugin
  - Improve ENC sounding rendering performance.
  - Correct auto-follow behavior and other handling on touch screen interface.
  - Enable MMSI Properties dialog in Options | Ships for Android.
  - Refine the selection method for quilted charts.
  - Handle invalid time cases in new route.
  - Add N0183 sentence $xxTHS for true heading.
  - Fix RangeRingColours for Isolated Marks.
  - Improve tide station window.
    Tides are updated for reliable real time presentation.
  - Enable selection of tide/current graph by double-click.
  - Add tidal currents for the Baltic and North Sea.
  - Add Ticon tidal database for extended tide station support, including
    Mediterranean Sea.
  - Correct AIS ATON icon rendering logic.
  - Add a search ship name function to the AIS target list.
  - Fix input of negative latitudes / longitudes.
  - Restrict Notification button "Bell" to primary canvas.
  - Implement delayed AIS alert dialog and sound effect to reduce disruptive
    false alerts.
  - Improve AIS Alert deferral logic on repeated alerts
  - Streamline the rebuilding of the chart database
  - Improve deferred chart update progress dialog
  - Remove deprecated option for COG/SOG estimation from position fixes
  - Correct AIS Query handling of message 14 Safety Broadcast Message
  - Improved performance for chart database update process.


Internal code updates:
----------------------
Improves developer accessibility and maintenance efficiency.

  - Introduce and implement Plugin API 1.21 for multi functional plugin
    handling.
  - Extensive internal code quality reviews, adding a large amount of
    descriptive commentary to the code base.
  - A fairly extensive revision of instructions for multiplexes and
    communications.
  - Refactoring in order to decrease internal couplings.
  - Refactor/simplify auto daily track restart logic
  - Refactor tooltips for the chart canvas.

Known issues:
------------

- Defining two signalK drivers using the same address/port might
  lead to a crash -  see #5119


Issues closed since 5.12.4:
---------------------------

#5148 Connections New/Edit "SocketCAN" disables "OK" button for all connections after it's used<br/>
#5147 Deleting the last waypoint in a route list causes exception<br/>
#5134 Default world map location seems to be confused<br/>
#5132 No position recieved using signalk data<br/>
#5130 The mouse pointer changes very early to a green arrow<br/>
#5123 Chart downloader can't type some language special letters made by Windows<br/>
#5122 AIS, MOB, EPIRB target query formatting problem<br/>
#5119 segmentation fault linux signalK<br/>
#5117 Options dialog changes second canvas chart type<br/>
#5107 OpenCPN fails to terminate, top-left toolbar not shown...<br/>
#5104 OpenCPN freezes after 10 times opening and closing the optiondialog<br/>
#5096 GDAL library is not "thread-safe"<br/>
#5094 opencpn-glutil: linux: Wrong installation directory<br/>
#5092 Humidity in Nmea0183 MDA<br/>
#5091 tests fails in debian pbuilder<br/>
#5088 Not able to filter out NMEA0183 "ZDA"<br/>
#5086 OpenCPN send N2k Message in a Format that is not self acceptance by itself<br/>
#5084 data/copyright is outdated<br/>
#5083 Spurious  console output<br/>
#5080 SART Alarm Not Properly Acknowledged or Silenced (Repeating Popup)<br/>
#5075 demo plugin: Incompatible license<br/>
#5074 Dashboard Sunrise/Sunset instrument has UTC time even if it says LCL<br/>
#5073 tests: failures<br/>
#5070 Data monitor: polish needed<br/>
#5068 OCPN crash while creating a route. (Mouse point related?)<br/>
#5067 Weather Routing Bug - Save Route Options<br/>
#5066 _build-flatpak-x86-2506_ and also _build-flatpak-arm64-2506_ FTB<br/>
#5064 Data Monitor (DM) : blank log lines<br/>
#5063 When closing window with alt + F4 after closing polar plugin then OpenCPN thinks it is crashing<br/>
