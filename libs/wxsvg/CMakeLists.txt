cmake_minimum_required(VERSION 3.1)

if (NOT CMAKE_MODULE_PATH)
  set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
endif ()

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

find_package(EXPAT)
if (NOT EXPAT_FOUND)
  SET(EXPAT_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/buildwin/expat-2.2.5/include)
  INCLUDE_DIRECTORIES(${EXPAT_INCLUDE_DIRS})
endif ()
include(FindEXIF)
if (EXIF_FOUND)
    message(STATUS "Building wxsvg with exif support.")
    set(SRC ${SRC} src/ExifHandler.cpp)
endif ()

add_library(WXSVG STATIC ${SRC})
set_property(TARGET WXSVG PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(WXSVG PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/wxSVG
    ${CMAKE_CURRENT_SOURCE_DIR}/include/wxSVGXML
    ${EXPAT_INCLUDE_DIRS}
    ${wxWidgets_INCLUDE_DIRS}
)
if (${bundled_expatdll_libs})
    target_link_libraries(WXSVG ${EXPAT_LIBS})      
endif()
if (EXIF_FOUND)
    target_include_directories(WXSVG PRIVATE ${EXIF_INCLUDE_DIR})
    target_compile_definitions(WXSVG PRIVATE wxsvgUSE_EXIF)
    target_link_libraries(WXSVG ${EXIF_LIBRARIES} ${EXPAT_LIBRARIES})      
endif ()
