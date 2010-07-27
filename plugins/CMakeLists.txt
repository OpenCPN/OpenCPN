##---------------------------------------------------------------------------
## Author:      bdbcat aka. dsr (Dave Register)
## Copyright:   
## License:     wxWidgets License
## Update:      sethdart (Jean-Eudes Onfray)
##              with parts from balp (Anders Arnholm)
## DEB config from: antonm - Anton Martchukov <anton@martchukov.com>
##---------------------------------------------------------------------------

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)
MESSAGE (STATUS "*** Building PlugIns ***")
 
add_subdirectory (grib_pi/src)
add_subdirectory (demo_pi/src)
add_subdirectory (dashboard_pi/src)
add_subdirectory (celestial_navigation_pi/src)


