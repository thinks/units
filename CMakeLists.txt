# Copyright (C) Tommy Hinks <tommy.hinks@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

cmake_minimum_required(VERSION 3.5)
project(thinks_units CXX)

option(THINKS_UNITS_RUN_TESTS "If ON, thinks::units tests will be run." OFF)

if (${THINKS_UNITS_RUN_TESTS})
  # Enable CTest.
  # This will set BUILD_TESTING to ON unless otherwise specified
  # on the command line.
  include(CTest)
  enable_testing()
  message(STATUS "thinks::units: BUILD_TESTING = ${BUILD_TESTING}")
endif()

# Include current path.
list(APPEND THINKS_UNITS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

# Check if CXX_STANDARD was specified, otherwise choose default.
if (NOT "${CMAKE_CXX_STANDARD}")
  message(STATUS "thinks::units: No CMAKE_CXX_STANDARD set, assuming 14")
  set(THINKS_UNITS_CXX_STANDARD 14)
else()
  set(THINKS_UNITS_CXX_STANDARD "${CMAKE_CXX_STANDARD}")
endif()

add_subdirectory(thinks)
