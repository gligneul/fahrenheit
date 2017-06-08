cmake_minimum_required(VERSION 2.6)
project(fahrenheit)

set(FAHRENHEIT_VERSION_MAJOR 0)
set(FAHRENHEIT_VERSION_MINOR 1)
configure_file(
  "${PROJECT_SOURCE_DIR}/fahrenheit/version.h.in"
  "${PROJECT_BINARY_DIR}/fahrenheit/version.h")

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_BINARY_DIR}")

add_definitions(
  -std=c89
  -Wall
  -Wextra
  -Werror
  -Wundef
  -Wshadow
  -Wcast-align
  -Wstrict-prototypes
  -Wmissing-prototypes
  -pedantic)

add_library(fahrenheit
  fahrenheit/ir.c)

SET(HEADERS
  fahrenheit/ir.h
  "${PROJECT_BINARY_DIR}/fahrenheit/version.h")

install(TARGETS fahrenheit DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include/fahrenheit)

include(CTest)
enable_testing()
add_custom_target(build-tests)
function(addcustomtest testname)
  add_executable(${testname} EXCLUDE_FROM_ALL tests/${testname}.c)
  add_dependencies(build-tests ${testname})
  target_link_libraries(${testname} fahrenheit)
  add_test(${testname} ${testname})
endfunction(addcustomtest)
addcustomtest(basic)

