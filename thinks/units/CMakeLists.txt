# Copyright (C) Tommy Hinks <tommy.hinks@gmail.com>
# This file is subject to the license terms in the LICENSE file
# found in the top-level directory of this distribution.

# Create library target and alias.
set(_LIB_NAME "thinks_units")
add_library(${_LIB_NAME} INTERFACE)
target_include_directories(${_LIB_NAME}
  INTERFACE
    ${THINKS_UNITS_INCLUDE_DIRS}
)
add_library(thinks::units ALIAS ${_LIB_NAME})

# Create test target if applicable.
if (${THINKS_UNITS_RUN_TESTS}) 
  set(_TEST_NAME "thinks_units_test")
  add_executable(${_TEST_NAME} "")
  target_sources(${_TEST_NAME} 
    PRIVATE 
      "units_test.cc"
  )
  target_compile_options(${_TEST_NAME}
    PRIVATE 
      # Need additional compiler flag to properly initialize 
      # the C++ version macro in MSVC.
      "$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>"
  )  
  target_link_libraries(${_TEST_NAME}
    PRIVATE 
      thinks::units
  )  
  
  set_property(TARGET ${_TEST_NAME} PROPERTY CXX_STANDARD ${THINKS_UNITS_CXX_STANDARD})
  set_property(TARGET ${_TEST_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

  add_test(NAME ${_TEST_NAME} COMMAND ${_TEST_NAME})
endif()
