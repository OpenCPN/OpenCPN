#-------------------------------------------------
#
# Project created by QtCreator 2014-03-27T23:08:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = opencpn
TEMPLATE = app

INCLUDEPATH += $$PWD/../include/
INCLUDEPATH += $$PWD/../src/nmea0183/

#wxQt_Base=/home/sean/build/wxWidgets/build-android/lib
wxQt_Base=$$system(which wx-config)
wxQt_Base=$$replace(wxQt_Base, "wx-config", "lib")
message("Using wxWidgets lib directory: " $$wxQt_Base)

INCLUDEPATH += $${wxQt_Base}/wx/include/arm-linux-androideabi-qt-unicode-static-3.1

LIBS += ./libopencpn.a
LIBS += ./libNMEA0183.a
LIBS += ./libGARMINHOST.a

LIBS += $${wxQt_Base}/libwx_qtu_html-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/libwx_baseu_xml-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/libwx_qtu_adv-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/libwx_baseu-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/libwx_qtu_core-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/libwx_qtu_qa-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/libwx_qtu_aui-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/libwxexpat-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/libwxregexu-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/libwxjpeg-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/libwxpng-3.1-arm-linux-androideabi.a




DEFINES += __WXQT__

HEADERS  +=

CONFIG += mobility
CONFIG += debug
MOBILITY =

ANDROID_EXTRA_LIBS = $$PWD/../buildandroid/libassetbridge.so

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
    android/AndroidManifest.xml

ui_deployment.files += $$PWD/../data/s57data/chartsymbols.xml
ui_deployment.files += $$PWD/../src/bitmaps/styles.xml
ui_deployment.files += $$PWD/../src/bitmaps/toolicons_traditional.png
ui_deployment.files += $$PWD/../src/bitmaps/toolicons_journeyman.png
ui_deployment.files += $$PWD/../src/bitmaps/toolicons_journeyman_flat.png
ui_deployment.path = /assets/uidata
INSTALLS += ui_deployment

gshhs_deployment.files += $$PWD/../data/gshhs/wdb_borders_c.b
gshhs_deployment.files += $$PWD/../data/gshhs/wdb_rivers_c.b
gshhs_deployment.files += $$PWD/../data/gshhs/poly-c-1.dat
gshhs_deployment.path = /assets/gshhs
INSTALLS += gshhs_deployment

