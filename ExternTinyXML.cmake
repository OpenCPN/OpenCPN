message(STATUS "Extern: TinyXML 2.6.2")

set(tinyxml_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/local")
set(tinyxml_INCLUDE_DIR "${tinyxml_INSTALL_DIR}/include")
set(tinyxml_LIBRARY_DIR "${tinyxml_INSTALL_DIR}/lib")

ExternalProject_Add(extern_tinyxml
	PREFIX "${CMAKE_CURRENT_BINARY_DIR}/tinyxml"
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src_extern/tinyxml-2.6.2"
	# configure
	CMAKE_ARGS
		-DCMAKE_INSTALL_PREFIX=${tinyxml_INSTALL_DIR}
		-DUSE_STL=ON
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
		-DCMAKE_C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}
		-DCMAKE_C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}
		-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
		-DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
		-DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
	# install
	INSTALL_DIR ${tinyxml_INSTALL_DIR}
	)

add_library(tinyxml STATIC IMPORTED)
set_target_properties(tinyxml
	PROPERTIES
		IMPORTED_LOCATION
			${tinyxml_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}tinyxml${CMAKE_STATIC_LIBRARY_SUFFIX}
	)
add_dependencies(tinyxml extern_tinyxml)
include_directories(${tinyxml_INCLUDE_DIR})
add_definitions("-DTIXML_USE_STL")

