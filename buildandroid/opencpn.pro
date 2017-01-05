#-------------------------------------------------
#
# Project created by QtCreator 2014-03-27T23:08:24
#
#-------------------------------------------------

QT       += core gui opengl widgets androidextras
LIBS += -lEGL
LIBS += -lz

TARGET = opencpn

TEMPLATE = app


#  THese definitions now made in Projects selector
# Qt target
#wxQt_Base=/home/dsr/Projects/wxqt/wxWidgets
#wxQt_Build=build_android_55

# OCPN target
#OCPN_Base=/home/dsr/Projects/opencpn_sf/opencpn
#OCPN_Build=build_android_55



INCLUDEPATH += $${wxQt_Base}/include/

INCLUDEPATH += $${OCPN_Base}/include/
INCLUDEPATH += $${OCPN_Base}/src/nmea0183


LIBS += -L$${wxQt_Base}/$${wxQt_Build}/lib
LIBS += -L$${OCPN_Base}/$${OCPN_Build}


LIBS += $${OCPN_Base}/$${OCPN_Build}/libgorp.a

INCLUDEPATH += $${wxQt_Base}/$${wxQt_Build}/lib/wx/include/arm-linux-androideabi-qt-unicode-static-3.1

LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_html-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_baseu_xml-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_qa-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_adv-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_core-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_baseu-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_aui-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwxexpat-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwxregexu-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwxjpeg-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwxpng-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_gl-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_baseu_net-3.1-arm-linux-androideabi.a
LIBS += $${OCPN_Base}/$${OCPN_Build}/lib/libGL.a

#LIBS += $${OCPN_Base}/$${OCPN_Build}/lib/libGLU.a
#LIBS += $${OCPN_Base}/$${OCPN_Build}/lib/libGLUES.a

#  I dunno why this does not work right....
contains(wxQt_Build,55)
{
LIBS += $${OCPN_Base}/$${OCPN_Build}/lib/libGLU.a
}

contains(wxQt_Build,53)
{
#LIBS += $${OCPN_Base}/$${OCPN_Build}/lib/libGLUES.a
}


TARGETDEPS += $${OCPN_Base}/$${OCPN_Build}/libgorp.a

TARGETDEPS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_baseu-3.1-arm-linux-androideabi.a
TARGETDEPS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_core-3.1-arm-linux-androideabi.a
TARGETDEPS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_gl-3.1-arm-linux-androideabi.a



DEFINES += __WXQT__

SOURCES += $$PWD/../buildandroid/ocpn_wrapper.cpp


CONFIG += mobility
CONFIG += debug
MOBILITY =

ANDROID_EXTRA_LIBS = $$PWD/../buildandroid/assetbridge/libs/armeabi/libassetbridge.so

# To execute the assetbridge runtime code, we make a custom modification to the android Activity method.

ANDROID_PACKAGE_SOURCE_DIR = $${OCPN_Base}/buildandroid/android
OTHER_FILES += $${OCPN_Base}/buildandroid/android/AndroidManifest.xml

s57_deployment.files += $$PWD/../data/s57data/chartsymbols.xml
s57_deployment.files += $$PWD/../data/s57data/attdecode.csv
s57_deployment.files += $$PWD/../data/s57data/rastersymbols-dark.png
s57_deployment.files += $$PWD/../data/s57data/rastersymbols-day.png
s57_deployment.files += $$PWD/../data/s57data/rastersymbols-dusk.png
s57_deployment.files += $$PWD/../data/s57data/s57attributes.csv
s57_deployment.files += $$PWD/../data/s57data/s57expectedinput.csv
s57_deployment.files += $$PWD/../data/s57data/s57objectclasses.csv
s57_deployment.path = /assets/files/s57data
INSTALLS += s57_deployment

ui_deployment.files += $$PWD/../src/bitmaps/styles.xml
ui_deployment.files += $$PWD/../src/bitmaps/toolicons_traditional.png
ui_deployment.files += $$PWD/../src/bitmaps/toolicons_journeyman.png
ui_deployment.files += $$PWD/../src/bitmaps/toolicons_journeyman_flat.png
ui_deployment.files += $$PWD/../data/svg/traditional/settings.svg
ui_deployment.path = /assets/files/uidata
INSTALLS += ui_deployment

svg_deployment.files += $$PWD/../data/svg/traditional/AIS_AlertGeneral_Active_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/AIS_AlertGeneral_Active.svg
svg_deployment.files += $$PWD/../data/svg/traditional/AIS_AlertGeneral_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/AIS_AlertGeneral.svg
svg_deployment.files += $$PWD/../data/svg/traditional/AIS_Disabled_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/AIS_Disabled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/AIS_Normal_Active_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/AIS_Normal_Active.svg
svg_deployment.files += $$PWD/../data/svg/traditional/AIS_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/AIS_Suppressed_Active_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/AIS_Suppressed_Active.svg
svg_deployment.files += $$PWD/../data/svg/traditional/AIS_Suppressed_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/AIS_Suppressed.svg
svg_deployment.files += $$PWD/../data/svg/traditional/AIS.svg
svg_deployment.files += $$PWD/../data/svg/traditional/colorscheme_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/colorscheme.svg
svg_deployment.files += $$PWD/../data/svg/traditional/CompassRoseBlue.svg
svg_deployment.files += $$PWD/../data/svg/traditional/CompassRose.svg
svg_deployment.files += $$PWD/../data/svg/traditional/current_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/current_rollover_toggled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/current.svg
svg_deployment.files += $$PWD/../data/svg/traditional/current_toggled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/down.svg
svg_deployment.files += $$PWD/../data/svg/traditional/follow_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/follow_rollover_toggled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/follow.svg
svg_deployment.files += $$PWD/../data/svg/traditional/follow_toggled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/gps1Bar.svg
svg_deployment.files += $$PWD/../data/svg/traditional/gps2Bar.svg
svg_deployment.files += $$PWD/../data/svg/traditional/gps3Bar.svg
svg_deployment.files += $$PWD/../data/svg/traditional/gpsGrn.svg
svg_deployment.files += $$PWD/../data/svg/traditional/gpsGry.svg
svg_deployment.files += $$PWD/../data/svg/traditional/gpsRed.svg
svg_deployment.files += $$PWD/../data/svg/traditional/grabber_ext.svg
svg_deployment.files += $$PWD/../data/svg/traditional/grabber_hi.svg
svg_deployment.files += $$PWD/../data/svg/traditional/grabber.svg
svg_deployment.files += $$PWD/../data/svg/traditional/help_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/help.svg
svg_deployment.files += $$PWD/../data/svg/traditional/left.svg
svg_deployment.files += $$PWD/../data/svg/traditional/mob_btn_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/mob_btn.svg
svg_deployment.files += $$PWD/../data/svg/traditional/mob.svg
svg_deployment.files += $$PWD/../data/svg/traditional/pencil.svg
svg_deployment.files += $$PWD/../data/svg/traditional/print_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/print.svg
svg_deployment.files += $$PWD/../data/svg/traditional/right.svg
svg_deployment.files += $$PWD/../data/svg/traditional/route_manager_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/route_manager.svg
svg_deployment.files += $$PWD/../data/svg/traditional/route_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/route_rollover_toggled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/route.svg
svg_deployment.files += $$PWD/../data/svg/traditional/route_toggled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/scin_disabled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/scin_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/scin.svg
svg_deployment.files += $$PWD/../data/svg/traditional/scout_disabled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/scout_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/scout.svg
svg_deployment.files += $$PWD/../data/svg/traditional/settings_disabled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/settings_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/settings.svg
svg_deployment.files += $$PWD/../data/svg/traditional/text_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/text_rollover_toggled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/text.svg
svg_deployment.files += $$PWD/../data/svg/traditional/text_toggled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/tide_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/tide_rollover_toggled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/tide.svg
svg_deployment.files += $$PWD/../data/svg/traditional/tide_toggled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/track_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/track_rollover_toggled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/track.svg
svg_deployment.files += $$PWD/../data/svg/traditional/track_toggled.svg
svg_deployment.files += $$PWD/../data/svg/traditional/up.svg
svg_deployment.files += $$PWD/../data/svg/traditional/zoomin_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/zoomin.svg
svg_deployment.files += $$PWD/../data/svg/traditional/zoomout_rollover.svg
svg_deployment.files += $$PWD/../data/svg/traditional/zoomout.svg
svg_deployment.path = /assets/files/uidata/traditional
INSTALLS += svg_deployment

gshhs_deployment.files += $$PWD/../data/gshhs/wdb_borders_i.b
gshhs_deployment.files += $$PWD/../data/gshhs/wdb_rivers_i.b
gshhs_deployment.files += $$PWD/../data/gshhs/poly-i-1.dat
gshhs_deployment.path = /assets/files/gshhs
INSTALLS += gshhs_deployment

styles_deployment.files += $$PWD/../data/styles/qtstylesheet.qss
styles_deployment.files += $$PWD/../data/styles/chek_full.png
styles_deployment.files += $$PWD/../data/styles/chek_empty.png
styles_deployment.files += $$PWD/../data/styles/tabbar_button_right.png
styles_deployment.files += $$PWD/../data/styles/tabbar_button_left.png
styles_deployment.path = /assets/files/styles
INSTALLS += styles_deployment


tc_deployment.files += $$PWD/../data/tcdata/HARMONIC.IDX
tc_deployment.files += $$PWD/../data/tcdata/HARMONIC
tc_deployment.path = /assets/files/tcdata
INSTALLS += tc_deployment

license_deployment.files += $$PWD/../data/license.txt
license_deployment.path = /assets/files
INSTALLS += license_deployment

sound_deployment.files += $$PWD/../data/sounds/1bells.wav
sound_deployment.files += $$PWD/../data/sounds/2bells.wav
sound_deployment.path = /assets/files/sounds
INSTALLS += sound_deployment

doc_deployment.files += $$PWD/../data/doc
doc_deployment.path = /assets/files/doc
INSTALLS += doc_deployment

# The built-in PlugIns

# Chart Downloader-------------------------------------------------------------------
#data
dldr_plugin_deployment.files += $$PWD/../plugins/chartdldr_pi/data/chart_sources.xml
dldr_plugin_deployment.files += $$PWD/../plugins/chartdldr_pi/data/folder.png
dldr_plugin_deployment.files += $$PWD/../plugins/chartdldr_pi/data/file.png
dldr_plugin_deployment.files += $$PWD/../plugins/chartdldr_pi/data/button_right.png
dldr_plugin_deployment.files += $$PWD/../plugins/chartdldr_pi/data/button_down.png
dldr_plugin_deployment.path = /assets/files/plugins/chartdldr_pi/data
INSTALLS += dldr_plugin_deployment

#locale
fr_FR_mo_dldr_deployment.files += $${OCPN_Base}/$${OCPN_Build}/plugins/chartdldr_pi/Resources/fr_FR.lproj/opencpn-chartdldr_pi.mo
fr_FR_mo_dldr_deployment.path = /assets/files/locale/fr_FR/LC_MESSAGES
INSTALLS += fr_FR_mo_dldr_deployment

#library
so_dldr_plugin_deployment.files +=$${OCPN_Base}/$${OCPN_Build}/plugins/chartdldr_pi/libchartdldr_pi.so
so_dldr_plugin_deployment.path = /assets/files/plugins
INSTALLS += so_dldr_plugin_deployment
#------------------------------------------------------------------------------------

# World Magnetic Model---------------------------------------------------------------
#library
so_wmm_plugin_deployment.files +=$${OCPN_Base}/$${OCPN_Build}/plugins/wmm_pi/libwmm_pi.so
so_wmm_plugin_deployment.path = /assets/files/plugins
INSTALLS += so_wmm_plugin_deployment

#data
wmm_plugin_deployment.files += $$PWD/../plugins/wmm_pi/data/WMM.COF
wmm_plugin_deployment.path = /assets/files/plugins/wmm_pi/data
INSTALLS += wmm_plugin_deployment

#SVG
svg_wmm_plugin_deployment.files +=$$PWD/../plugins/wmm_pi/data/wmm_live.svg
svg_wmm_plugin_deployment.files +=$$PWD/../plugins/wmm_pi/data/wmm_pi.svg
svg_wmm_plugin_deployment.path = /assets/files/plugins/wmm_pi/data
INSTALLS += svg_wmm_plugin_deployment

#locale
fr_FR_mo_wmm_deployment.files += $${OCPN_Base}/$${OCPN_Build}/plugins/wmm_pi/Resources/fr_FR.lproj/opencpn-wmm_pi.mo
fr_FR_mo_wmm_deployment.path = /assets/files/locale/fr_FR/LC_MESSAGES
INSTALLS += fr_FR_mo_wmm_deployment

#------------------------------------------------------------------------------------

# GRIB---------------------------------------------------------------
#library
so_grib_plugin_deployment.files +=$${OCPN_Base}/$${OCPN_Build}/plugins/grib_pi/libgrib_pi.so
so_grib_plugin_deployment.path = /assets/files/plugins
INSTALLS += so_grib_plugin_deployment

#locale
fr_FR_mo_grib_deployment.files += $${OCPN_Base}/$${OCPN_Build}/plugins/grib_pi/Resources/fr_FR.lproj/opencpn-grib_pi.mo
fr_FR_mo_grib_deployment.path = /assets/files/locale/fr_FR/LC_MESSAGES
INSTALLS += fr_FR_mo_grib_deployment

#SVG
svg_grib_plugin_deployment.path = /assets/files/plugins/grib_pi/data
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/altitude.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/curdata.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/grib.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/grib_rollover.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/grib_toggled.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/ncurdata.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/next.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/now.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/openfile.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/play.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/prev.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/request.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/request_end.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/selzone.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/setting.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/slider.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/stop.svg
svg_grib_plugin_deployment.files +=$$PWD/../plugins/grib_pi/data/zoomto.svg
INSTALLS += svg_grib_plugin_deployment
#------------------------------------------------------------------------------------


# Dashboard---------------------------------------------------------------
#library
so_dashboard_plugin_deployment.files +=$${OCPN_Base}/$${OCPN_Build}/plugins/dashboard_pi/libdashboard_pi.so
so_dashboard_plugin_deployment.path = /assets/files/plugins
INSTALLS += so_dashboard_plugin_deployment

#SVG
svg_dashboard_plugin_deployment.files +=$$PWD/../plugins/dashboard_pi/data/Dashboard.svg
svg_dashboard_plugin_deployment.files +=$$PWD/../plugins/dashboard_pi/data/Dashboard_toggled.svg
svg_dashboard_plugin_deployment.path = /assets/files/plugins/dashboard_pi/data
INSTALLS += svg_dashboard_plugin_deployment

#locale
fr_FR_mo_dashboard_deployment.files += $${OCPN_Base}/$${OCPN_Build}/plugins/dashboard_pi/Resources/fr_FR.lproj/opencpn-dashboard_pi.mo
fr_FR_mo_dashboard_deployment.path = /assets/files/locale/fr_FR/LC_MESSAGES
INSTALLS += fr_FR_mo_dashboard_deployment
#------------------------------------------------------------------------------------

# Squiddio---------------------------------------------------------------
#library
so_squiddio_plugin_deployment.files +=$${OCPN_Base}/$${OCPN_Build}/plugins/squiddio_pi/libsquiddio_pi.so
so_squiddio_plugin_deployment.path = /assets/files/plugins
INSTALLS += so_squiddio_plugin_deployment

#locale
fr_FR_mo_squiddio_deployment.files += $${OCPN_Base}/$${OCPN_Build}/plugins/squiddio_pi/Resources/fr_FR.lproj/opencpn-squiddio_pi.mo
fr_FR_mo_squiddio_deployment.path = /assets/files/locale/fr_FR/LC_MESSAGES
INSTALLS += fr_FR_mo_squiddio_deployment

#------------------------------------------------------------------------------------


fr_FR_mo_deployment.files +=$${OCPN_Base}/$${OCPN_Build}/Resources/opencpn_fr.lproj/opencpn.mo
fr_FR_mo_deployment.path = /assets/files/locale/fr_FR/LC_MESSAGES
INSTALLS += fr_FR_mo_deployment

ANDROID_EXTRA_LIBS = \
        /home/dsr/Projects/opencpn_android/buildandroid/../buildandroid/assetbridge/libs/armeabi/libassetbridge.so

DISTFILES += \
    android/src/org/opencpn/PortContainer.java









