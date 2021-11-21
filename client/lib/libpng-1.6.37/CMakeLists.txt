# CMakeLists.txt

# Copyright (c) 2018-2020 Cosmin Truta
# Copyright (c) 2007,2009-2018 Glenn Randers-Pehrson
# Written by Christian Ehrlicher, 2007
# Revised by Roger Lowman, 2009-2010
# Revised by Clifford Yapp, 2011-2012,2017
# Revised by Roger Leigh, 2016
# Revised by Andreas Franek, 2016
# Revised by Sam Serrels, 2017
# Revised by Vadim Barkov, 2017
# Revised by Vicky Pfau, 2018
# Revised by Cameron Cawley, 2018
# Revised by Kyle Bentley, 2018
# Revised by David Callu, 2020
# Revised by Steve Robinson, 2020
# Revised by Simon Hausmann, 2020
# Revised by Alex Gaynor, 2020

# This code is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h

cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)

project(libpng C)
enable_testing()

set(PNGLIB_MAJOR 1)
set(PNGLIB_MINOR 6)
set(PNGLIB_RELEASE 38)
set(PNGLIB_NAME libpng${PNGLIB_MAJOR}${PNGLIB_MINOR})
set(PNGLIB_VERSION ${PNGLIB_MAJOR}.${PNGLIB_MINOR}.${PNGLIB_RELEASE})

include(GNUInstallDirs)

# Many more configuration options could be added here.
option(PNG_DEBUG "Build with debug output" OFF)

set(PNG_PREFIX "" CACHE STRING "Prefix to add to the API function names")
set(DFA_XTRA "" CACHE FILEPATH "File containing extra configuration settings")

# Set PNG_LIB_NAME.
set(PNG_LIB_NAME png${PNGLIB_MAJOR}${PNGLIB_MINOR})

# Distinguish between debug and release builds.
set(CMAKE_DEBUG_POSTFIX "d")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/pnglibconf.h.prebuilt
               ${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h)

# List the source code files.
set(libpng_public_hdrs png.h pngconf.h "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h")
set(libpng_private_hdrs pngpriv.h pngdebug.h pnginfo.h pngstruct.h)
set(libpng_sources ${libpng_public_hdrs} ${libpng_private_hdrs} png.c pngerror.c pngget.c pngmem.c pngpread.c pngread.c pngrio.c pngrtran.c pngrutil.c pngset.c pngtrans.c pngwio.c pngwrite.c pngwtran.c pngwutil.c)

if(PNG_DEBUG)
  add_definitions(-DPNG_DEBUG)
endif()

# Now build our target.
add_library(png_static STATIC ${libpng_sources})
target_link_libraries(png_static PRIVATE zlibstatic m)
target_include_directories(png_static PUBLIC ${CMAKE_CURRENT_BINARY_DIR} .)
