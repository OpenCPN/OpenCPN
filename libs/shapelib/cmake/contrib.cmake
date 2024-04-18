if(BUILD_SHAPELIB_CONTRIB)
  add_executable(csv2shp ${PROJECT_SOURCE_DIR}/contrib/csv2shp.c)
  target_link_libraries(csv2shp shp)
  set_target_properties(csv2shp PROPERTIES FOLDER "contrib")

  add_executable(dbfcat ${PROJECT_SOURCE_DIR}/contrib/dbfcat.c)
  target_link_libraries(dbfcat shp)
  set_target_properties(dbfcat PROPERTIES FOLDER "contrib")

  add_executable(dbfinfo ${PROJECT_SOURCE_DIR}/contrib/dbfinfo.c)
  target_link_libraries(dbfinfo shp)
  set_target_properties(dbfinfo PROPERTIES FOLDER "contrib")

  add_executable(shpcat ${PROJECT_SOURCE_DIR}/contrib/shpcat.c)
  target_link_libraries(shpcat shp)
  set_target_properties(shpcat PROPERTIES FOLDER "contrib")

  add_executable(shpdxf ${PROJECT_SOURCE_DIR}/contrib/shpdxf.c)
  target_link_libraries(shpdxf shp)
  set_target_properties(shpdxf PROPERTIES FOLDER "contrib")

  add_executable(shpfix ${PROJECT_SOURCE_DIR}/contrib/shpfix.c)
  target_link_libraries(shpfix shp)
  set_target_properties(shpfix PROPERTIES FOLDER "contrib")

  add_executable(shpsort ${PROJECT_SOURCE_DIR}/contrib/shpsort.c)
  target_link_libraries(shpsort shp)
  set_target_properties(shpsort PROPERTIES FOLDER "contrib")

  add_executable(Shape_PointInPoly ${PROJECT_SOURCE_DIR}/contrib/Shape_PointInPoly.cpp)
  target_link_libraries(Shape_PointInPoly shp)
  set_target_properties(Shape_PointInPoly PROPERTIES FOLDER "contrib" LINKER_LANGUAGE CXX)

  add_executable(shpcentrd ${PROJECT_SOURCE_DIR}/contrib/shpcentrd.c ${PROJECT_SOURCE_DIR}/contrib/shpgeo.c ${PROJECT_SOURCE_DIR}/contrib/shpgeo.h)
  target_link_libraries(shpcentrd shp)
  set_target_properties(shpcentrd PROPERTIES FOLDER "contrib")

  add_executable(shpdata ${PROJECT_SOURCE_DIR}/contrib/shpcentrd.c ${PROJECT_SOURCE_DIR}/contrib/shpgeo.c ${PROJECT_SOURCE_DIR}/contrib/shpgeo.h)
  target_link_libraries(shpdata shp)
  set_target_properties(shpdata PROPERTIES FOLDER "contrib")

  add_executable(shpinfo ${PROJECT_SOURCE_DIR}/contrib/shpcentrd.c ${PROJECT_SOURCE_DIR}/contrib/shpgeo.c ${PROJECT_SOURCE_DIR}/contrib/shpgeo.h)
  target_link_libraries(shpinfo shp)
  set_target_properties(shpinfo PROPERTIES FOLDER "contrib")

  add_executable(shpwkb ${PROJECT_SOURCE_DIR}/contrib/shpcentrd.c ${PROJECT_SOURCE_DIR}/contrib/shpgeo.c ${PROJECT_SOURCE_DIR}/contrib/shpgeo.h)
  target_link_libraries(shpwkb shp)
  set_target_properties(shpwkb PROPERTIES FOLDER "contrib")

  install(
    TARGETS
      csv2shp
      dbfcat
      dbfinfo
      shpcat
      shpdxf
      shpfix
      shpsort
      Shape_PointInPoly
      shpcentrd
      shpdata
      shpinfo
      shpwkb
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
endif()
