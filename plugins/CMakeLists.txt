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
MESSAGE (STATUS "*** Staging to build PlugIns ***")

# These variables are inherited by all plugins
SET(PARENT opencpn)
SET(PREFIX_DATA share)
SET(PREFIX_PLUGINS ${PREFIX_LIB}/${PARENT})

FILE(GLOB PLUGINS *_pi)

FOREACH(PLUGIN ${PLUGINS})
     add_subdirectory (${PLUGIN})
ENDFOREACH(PLUGIN)

# Allow chaining of any PlugIn package dependencies upstream to the global package
# PlugIns may then individually, as required, include the following:
# SET(PACKAGE_DEPS "${PACKAGE_DEPS}, dep1, dep2" PARENT_SCOPE) 
SET(PACKAGE_DEPS ${PACKAGE_DEPS} PARENT_SCOPE) 
