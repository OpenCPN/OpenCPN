cmake_minimum_required(VERSION 3.1)

if (TARGET ocpn::wxsvg)
    return ()
endif ()

#
# Build wxsvg library. Primarely intended to be invoked fror the main
# CMakeLists. The alternative uses a simplified configuration
# requiring proper pkg-config setup and just using standard wxWidgets
# options. It has only been tested on linux.

if (NOT CMAKE_MODULE_PATH)
  set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
endif ()

if(UNIX AND NOT APPLE)
  include(FindGlib2)
endif(UNIX AND NOT APPLE)


set(SRC
    src/CSSStyleDeclaration.cpp
    src/CSSValue.cpp
    src/Elements_CopyConstructors.cpp
    src/Elements_GetAttribute.cpp
    src/Elements_GetAttributes.cpp
    src/Elements_HasAttribute.cpp
    src/Elements_SetAttribute.cpp
    src/GetSVGDocument.cpp
    src/NodeList.cpp
    src/SVGAngle.cpp
    src/SVGAnimateTransformElement.cpp
    src/SVGAnimateMotionElement.cpp
    src/SVGAnimatedType.cpp
    src/SVGAnimationElement.cpp
    src/SVGCanvas.cpp
    src/SVGCanvasItem.cpp
    src/SVGCircleElement.cpp
    src/SVGColor.cpp
    src/SVGDocument.cpp
    src/SVGElementInstanceList.cpp
    src/SVGEllipseElement.cpp
    src/SVGFEGaussianBlurElement.cpp
    src/SVGFilterElement.cpp
    src/SVGFitToViewBox.cpp
    src/SVGImageElement.cpp
    src/SVGLength.cpp
    src/SVGLengthList.cpp
    src/SVGLineElement.cpp
    src/SVGLocatable.cpp
    src/SVGMarkerElement.cpp
    src/SVGMatrix.cpp
    src/SVGNumberList.cpp
    src/SVGPaint.cpp
    src/SVGPathElement.cpp
    src/SVGPathSegList.cpp
    src/SVGPoint.cpp
    src/SVGPointList.cpp
    src/SVGPolygonElement.cpp
    src/SVGPolylineElement.cpp
    src/SVGPreserveAspectRatio.cpp
    src/SVGRadialGradientElement.cpp
    src/SVGRect.cpp
    src/SVGRectElement.cpp
    src/SVGSVGElement.cpp
    src/SVGStringList.cpp
    src/SVGStylable.cpp
    src/SVGTests.cpp
    src/SVGTextContentElement.cpp
    src/SVGTextElement.cpp
    src/SVGTransform.cpp
    src/SVGTransformList.cpp
    src/SVGTransformable.cpp
    src/SVGUseElement.cpp
    src/SVGVideoElement.cpp
    src/imagsvg.cpp
#   src/mediadec_ffmpeg.cpp
    src/svgctrl.cpp
    src/cairo/SVGCanvasCairo.cpp
    src/cairo/SVGCanvasCairo.h
    src/cairo/SVGCanvasImageCairo.cpp
    src/cairo/SVGCanvasImageCairo.h
    src/cairo/SVGCanvasPathCairo.cpp
    src/cairo/SVGCanvasPathCairo.h
    src/cairo/SVGCanvasTextCairo.cpp
    src/cairo/SVGCanvasTextCairo.h
    src/svgxml/svgxml.cpp
    src/svgxml/svgxmlhelpr.cpp
)

add_library(WXSVG STATIC ${SRC})
add_library(ocpn::wxsvg ALIAS WXSVG)
set_property(TARGET WXSVG PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")

     
   # CMake has trouble finding libgobject-2.0.so, as required for static wxsvg library.
   # We look for and add it explicitely
if (NOT WIN32 AND NOT APPLE AND NOT QT_ANDROID)

        # Assume pkg-config is available.
        pkg_search_module(GOBJECT REQUIRED gobject-2.0)
        #MESSAGE (STATUS "    GOBJECT: " ${GOBJECT_LDFLAGS})
        target_link_libraries(WXSVG PRIVATE ${GOBJECT_LDFLAGS})
        
        find_library(PANGOCAIRO_LIBRARY NAMES pangocairo-1.0 PATHS ${LINUX_LIB_PATHS})
        #MESSAGE (STATUS "    PANGOCAIRO: " ${PANGOCAIRO_LIBRARY})
        target_link_libraries(WXSVG PRIVATE ${PANGOCAIRO_LIBRARY})
        
endif (NOT WIN32 AND NOT APPLE AND NOT QT_ANDROID)

include(FindEXIF)
if (EXIF_FOUND)
    target_sources(WXSVG PRIVATE src/ExifHandler.cpp)
    target_compile_definitions(WXSVG PRIVATE -DwxsvgUSE_EXIF)
    target_include_directories(WXSVG PRIVATE ${EXIF_INCLUDE_DIRS})
    target_link_libraries(WXSVG PRIVATE ${EXIF_LIBRARIES})
endif ()

target_compile_definitions(WXSVG PRIVATE -DUSE_RENDER_CAIRO)

if (NOT wxWidgets_INCLUDE_DIRS)
    include(SimpleWxConfig)
    SimpleWxConfig()

    include(FindPkgConfig)
    pkg_search_module(GLIB2 glib-2.0)
    pkg_search_module(CAIRO cairo libcairo)
    find_package(EXPAT REQUIRED)
    find_package(PANGO REQUIRED)
    target_link_libraries(WXSVG_CAIRO PRIVATE ocpn::pango)
endif (NOT wxWidgets_INCLUDE_DIRS)
add_library(WXSVG_CAIRO INTERFACE)


target_include_directories(WXSVG_CAIRO INTERFACE
    ${CAIRO_INCLUDE_DIRS}
    ${EXPAT_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
)
target_link_libraries(WXSVG_CAIRO INTERFACE
    ${CAIRO_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${GLIB2_LIBRARIES}
)
if(UNIX AND NOT APPLE)
    target_include_directories(WXSVG_CAIRO INTERFACE ${GLib_INCLUDE_DIRS})
    target_link_libraries(WXSVG_CAIRO INTERFACE ${GLib_LIBRARY})
endif(UNIX AND NOT APPLE)

add_library(wxsvg::cairo ALIAS WXSVG_CAIRO)

target_include_directories(WXSVG
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/wxSVG
        ${CMAKE_CURRENT_SOURCE_DIR}/include/wxSVGXML
        ${wxWidgets_INCLUDE_DIRS}
)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang")
    target_compile_options(WXSVG PRIVATE
        "-Wno-varargs" "-Wno-unknown-pragmas" "-Wno-overloaded-virtual"
    )
endif()
target_link_libraries(WXSVG PRIVATE ocpn::expat wxsvg::cairo)
if (TARGET ocpn::pango)
    target_link_libraries(WXSVG PRIVATE ocpn::pango)
endif ()
if (TARGET ocpn::gtk)
  target_link_libraries(WXSVG PRIVATE ocpn::gtk)
endif ()
