cmake_minimum_required(VERSION 3.5.0)

if (TARGET ocpn::gdal)
    return ()
endif ()

set(SRC
    include/gdal/cpl_config.h
    include/gdal/gdal_priv.h
    include/gdal/cpl_conv.h
    include/gdal/cpl_csv.h
    include/gdal/ogr_api.h
    include/gdal/cpl_error.h
    include/gdal/ogr_attrind.h
    include/gdal/cpl_list.h
    include/gdal/cpl_minixml.h
    include/gdal/ogr_feature.h
    include/gdal/cpl_multiproc.h
    include/gdal/ogr_geometry.h
    include/gdal/cpl_odbc.h
    include/gdal/ogr_p.h
    include/gdal/ogr_core.h
    include/gdal/cpl_port.h
    include/gdal/cpl_string.h
    include/gdal/ogr_spatialref.h
    include/gdal/cpl_vsi.h
    include/gdal/ogrsf_frmts.h
    include/gdal/gdal.h
    include/gdal/gdal_frmts.h
    src/cpl_conv.cpp
    src/cpl_csv.cpp
    src/cpl_error.cpp
    src/cpl_findfile.cpp
    src/cpl_minixml.cpp
    src/cpl_path.cpp
    src/cpl_string.cpp
    src/cpl_vsisimple.cpp
    src/cplgetsymbol.cpp
    src/ograssemblepolygon.cpp
    src/ogrcurve.cpp
    src/ogrfeature.cpp
    src/ogrfeaturedefn.cpp
    src/ogrfielddefn.cpp
    src/ogrgeometry.cpp
    src/ogrgeometrycollection.cpp
    src/ogrgeometryfactory.cpp
    src/ogrlinearring.cpp
    src/ogrlinestring.cpp
    src/ogrmultilinestring.cpp
    src/ogrmultipoint.cpp
    src/ogrmultipolygon.cpp
    src/ogrpoint.cpp
    src/ogrpolygon.cpp
    src/ogrlayer.cpp
    src/ogrutils.cpp
    src/gdal_misc.cpp
)

add_library(GDAL STATIC ${SRC})
add_library(ocpn::gdal ALIAS GDAL)

set_property(TARGET GDAL PROPERTY COMPILE_FLAGS "${OBJ_VISIBILITY}")
target_include_directories(GDAL BEFORE
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/gdal
)
