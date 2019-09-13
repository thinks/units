# Copyright (C) Tommy Hinks <tommy.hinks@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

set(_LIB_NAME "thinks_units")
add_library(${_LIB_NAME} INTERFACE)
target_include_directories(${_LIB_NAME}
  INTERFACE
    ${PROJECT_SOURCE_DIR}
    #${THINKS_COMMON_INCLUDE_DIRS}
)
add_library(thinks::units ALIAS ${_LIB_NAME})

if (BUILD_TESTING) 
  set(_TEST_NAME "thinks_units_test")
  add_executable(${_TEST_NAME} "")
  target_sources(${_TEST_NAME} 
    PRIVATE 
      "units_test.cc"
  )
  target_link_libraries(${_TEST_NAME}
    PRIVATE 
      thinks::units
  )
  
  set_property(TARGET ${_TEST_NAME} PROPERTY CXX_STANDARD ${THINKS_UNITS_CXX_STANDARD})
  set_property(TARGET ${_TEST_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

  add_test(NAME ${_TEST_NAME} COMMAND ${_TEST_NAME})
endif()