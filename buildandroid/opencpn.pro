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
s57_deployment.path = /assets/s57data
INSTALLS += s57_deployment

ui_deployment.files += $$PWD/../src/bitmaps/styles.xml
ui_deployment.files += $$PWD/../src/bitmaps/toolicons_traditional.png
ui_deployment.files += $$PWD/../src/bitmaps/toolicons_journeyman.png
ui_deployment.files += $$PWD/../src/bitmaps/toolicons_journeyman_flat.png
ui_deployment.path = /assets/uidata
INSTALLS += ui_deployment

gshhs_deployment.files += $$PWD/../data/gshhs/wdb_borders_i.b
gshhs_deployment.files += $$PWD/../data/gshhs/wdb_rivers_i.b
gshhs_deployment.files += $$PWD/../data/gshhs/poly-i-1.dat
gshhs_deployment.path = /assets/gshhs
INSTALLS += gshhs_deployment

styles_deployment.files += $$PWD/../data/styles/qtstylesheet.qss
styles_deployment.path = /assets/styles
INSTALLS += styles_deployment

tc_deployment.files += $$PWD/../data/tcdata/HARMONIC.IDX
tc_deployment.files += $$PWD/../data/tcdata/HARMONIC
tc_deployment.path = /assets/tcdata
INSTALLS += tc_deployment

license_deployment.files += $$PWD/../data/license.txt
license_deployment.path = /assets
INSTALLS += license_deployment

sound_deployment.files += $$PWD/../data/sounds/1bells.wav
sound_deployment.files += $$PWD/../data/sounds/2bells.wav
sound_deployment.path = /assets/sounds
INSTALLS += sound_deployment

doc_deployment.files += $$PWD/../data/doc
doc_deployment.path = /assets/doc
INSTALLS += doc_deployment

# The built-in PlugIns

# Chart Downloader-------------------------------------------------------------------
#data
dldr_plugin_deployment.files += $$PWD/../plugins/chartdldr_pi/data/chart_sources.xml
dldr_plugin_deployment.files += $$PWD/../plugins/chartdldr_pi/data/folder215.png
dldr_plugin_deployment.files += $$PWD/../plugins/chartdldr_pi/data/open182.png
dldr_plugin_deployment.path = /assets/plugins/chartdldr_pi/data
INSTALLS += dldr_plugin_deployment

#library
so_dldr_plugin_deployment.files +=$${OCPN_Base}/$${OCPN_Build}/plugins/chartdldr_pi/libchartdldr_pi.so
so_dldr_plugin_deployment.path = /assets/plugins
INSTALLS += so_dldr_plugin_deployment
#------------------------------------------------------------------------------------

# World Magnetic Model---------------------------------------------------------------
#library
#so_wmm_plugin_deployment.files +=$${OCPN_Base}/$${OCPN_Build}/plugins/wmm_pi/libwmm_pi.so
#so_wmm_plugin_deployment.path = /assets/plugins
#INSTALLS += so_wmm_plugin_deployment

#data
#wmm_plugin_deployment.files += $$PWD/../plugins/wmm_pi/data/WMM.COF
#wmm_plugin_deployment.path = /assets/plugins/wmm_pi/data
INSTALLS += wmm_plugin_deployment
#------------------------------------------------------------------------------------


# GRIB---------------------------------------------------------------
#library
so_grib_plugin_deployment.files +=$${OCPN_Base}/$${OCPN_Build}/plugins/grib_pi/libgrib_pi.so
so_grib_plugin_deployment.path = /assets/plugins
INSTALLS += so_grib_plugin_deployment
#------------------------------------------------------------------------------------


ANDROID_EXTRA_LIBS = \
        /home/dsr/Projects/opencpn_android/buildandroid/../buildandroid/assetbridge/libs/armeabi/libassetbridge.so


#ANDROID_EXTRA_LIBS += /home/dsr/Projects/opencpn_sf/opencpn/build_android_53/plugins/dashboard_pi/libdashboard_pi.so
#ANDROID_EXTRA_LIBS += /home/dsr/Projects/opencpn_sf/opencpn/build_android_53/plugins/grib_pi/libgrib_pi.so








