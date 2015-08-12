#-------------------------------------------------
#
# Project created by QtCreator 2014-03-27T23:08:24
#
#-------------------------------------------------

QT       += core gui opengl widgets

#QT += androidextras
LIBS += -lEGL
LIBS += -lz

TARGET = opencpn

TEMPLATE = app

CONFIG(SHARED){
# Qt target

wxQt_Base=/home/dsr/Projects/wxqt/wxWidgets
wxQt_Build=build_android_so

# OCPN target
OCPN_Base=/home/dsr/Projects/opencpn_sf/opencpn
OCPN_Build=build_android_so
}
else{
# Qt target
wxQt_Base=/home/dsr/Projects/wxqt/wxWidgets
wxQt_Build=build_android_53

# OCPN target
OCPN_Base=/home/dsr/Projects/opencpn_sf/opencpn
OCPN_Build=build_android_53
}


INCLUDEPATH += $${wxQt_Base}/include/

INCLUDEPATH += $${OCPN_Base}/include/
INCLUDEPATH += $${OCPN_Base}/src/nmea0183

#LIBS += $${OCPN_Base}/$${OCPN_Build}/libopencpn.a
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libgorp.a
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libGARMINHOST.a
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libNMEA0183.a
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libS57ENC.a
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libSYMBOLS.a
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libTEXCMP.a
#LIBS += $${OCPN_Base}/$${OCPN_Build}/lib/libGLUES.a


LIBS += -L$${wxQt_Base}/$${wxQt_Build}/lib
LIBS += -L$${OCPN_Base}/$${OCPN_Build}

CONFIG(SHARED){

TARGETDEPS += $${OCPN_Base}/$${OCPN_Build}/libgorp.a

LIBS += $${OCPN_Base}/$${OCPN_Build}/libgorp.a
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libGARMINHOST.so
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libNMEA0183.so
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libS57ENC.so
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libSYMBOLS.so
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libTEXCMP.so
#LIBS += $${OCPN_Base}/$${OCPN_Build}/lib/libGLUES.so

INCLUDEPATH += $${wxQt_Base}/$${wxQt_Build}/lib/wx/include/arm-linux-androideabi-qt-unicode-3.1

LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_html-3.1.so
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_baseu_xml-3.1.so
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_qa-3.1.so
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_adv-3.1.so
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_core-3.1.so
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_baseu-3.1.so
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_aui-3.1.so
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwxexpat-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwxregexu-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwxjpeg-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwxpng-3.1-arm-linux-androideabi.a
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_baseu_net-3.1.so
LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_gl-3.1-arm-linux-androideabi.so
LIBS += /home/dsr/Projects/opencpn_sf/opencpn/build_android_so/lib/libGL.a
LIBS += /home/dsr/Projects/opencpn_sf/opencpn/build_android_so/lib/libGLU.a


}
else{

LIBS += $${OCPN_Base}/$${OCPN_Build}/libgorp.a
LIBS += $${OCPN_Base}/$${OCPN_Build}/libGARMINHOST.a
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libNMEA0183.a
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libS57ENC.a
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libSYMBOLS.a
#LIBS += $${OCPN_Base}/$${OCPN_Build}/libTEXCMP.a
LIBS += $${OCPN_Base}/$${OCPN_Build}/lib/libGLUES.a

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

TARGETDEPS += $${OCPN_Base}/$${OCPN_Build}/libgorp.a
#TARGETDEPS += $${OCPN_Base}/$${OCPN_Build}/libS57ENC.a

TARGETDEPS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_baseu-3.1-arm-linux-androideabi.a
TARGETDEPS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_core-3.1-arm-linux-androideabi.a
TARGETDEPS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_gl-3.1-arm-linux-androideabi.a

}

LIBS += /home/dsr/Qt/5.3/android_armv7/lib/libQt5AndroidExtras.so


DEFINES += __WXQT__

SOURCES += $$PWD/../buildandroid/ocpn_wrapper.cpp


CONFIG += mobility
CONFIG += debug
MOBILITY =

ANDROID_EXTRA_LIBS = $$PWD/../buildandroid/assetbridge/libs/armeabi/libassetbridge.so

# To execute the assetbridge runtime code, we make a custom modification to the android Activity method.
# so we include the sources for this patched version here

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

dldr_plugin_deployment.files += $$PWD/../plugins/chartdldr_pi/data/chart_sources.xml
dldr_plugin_deployment.files += $$PWD/../plugins/chartdldr_pi/data/folder215.png
dldr_plugin_deployment.files += $$PWD/../plugins/chartdldr_pi/data/open182.png
dldr_plugin_deployment.files +=$${OCPN_Base}/$${OCPN_Build}/plugins/chartdldr_pi/libchartdldr_pi.so
dldr_plugin_deployment.path = /assets/plugins/chartdldr_pi/data
#dldr_plugin_deployment.path = /assets/plugins
INSTALLS += dldr_plugin_deployment


#contains(ANDROID_TARGET_ARCH,armeabi-v7a)
 {
    ANDROID_EXTRA_LIBS = \
        /home/dsr/Projects/opencpn_android/buildandroid/../buildandroid/assetbridge/libs/armeabi/libassetbridge.so

ANDROID_EXTRA_LIBS += /home/dsr/Qt/5.3/android_armv7/lib/libQt5Test.so

#ANDROID_EXTRA_LIBS += /home/dsr/Projects/opencpn_sf/opencpn/build_android_53/plugins/dashboard_pi/libdashboard_pi.so
#ANDROID_EXTRA_LIBS += /home/dsr/Projects/opencpn_sf/opencpn/build_android_53/plugins/grib_pi/libgrib_pi.so


CONFIG(SHARED){

ANDROID_EXTRA_LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_baseu-3.1.so
ANDROID_EXTRA_LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_core-3.1.so
ANDROID_EXTRA_LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_html-3.1.so
ANDROID_EXTRA_LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_baseu_xml-3.1.so
ANDROID_EXTRA_LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_qa-3.1.so
ANDROID_EXTRA_LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_adv-3.1.so
ANDROID_EXTRA_LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_aui-3.1.so
ANDROID_EXTRA_LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_baseu_net-3.1.so
ANDROID_EXTRA_LIBS += $${wxQt_Base}/$${wxQt_Build}/lib/libwx_qtu_gl-3.1.so


}

}




