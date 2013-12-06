#############################################################
cmake_minimum_required ( VERSION 2.8.10 )
project ( CSE5344-ECC )

#############################################################
### Version numbering
set (PROJECT_VERSION_MAJOR 1)
set (PROJECT_VERSION_MINOR 0)

### Project Package Config
find_package( PkgConfig )
pkg_check_modules( GLIB2 glib-2.0 )

#############################################################
## Configure a header file to pass some of the CMake settings
## to the source code

#configure_file (
# "${PROJECT_SOURCE_DIR}/config.h.in"
# "${PROJECT_BINARY_DIR}/config.h"
#)

##
## add the binary tree to the search path for include files
## so that we will find the config.h

#include_directories("${PROJECT_BINARY_DIR}")
#include_directories("${PROJECT_SOURCE_DIR}/include")

#############################################################
## Project binaries

#######################################
## Project Subdirectories
add_subdirectory ( src )

