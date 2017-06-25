cmake_minimum_required(VERSION 2.6)
project(fahrenheit)

set(FAHRENHEIT_VERSION_MAJOR 0)
set(FAHRENHEIT_VERSION_MINOR 1)
configure_file(
  "${PROJECT_SOURCE_DIR}/include/fahrenheit/version.h.in"
  "${PROJECT_BINARY_DIR}/include/fahrenheit/version.h")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
  -std=c++11 \
  -Wall \
  -Werror \
  -Wundef")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
  -std=c89 \
  -Wall \
  -Wextra \
  -Werror \
  -Wundef \
  -Wshadow \
  -Wcast-align \
  -Wstrict-prototypes \
  -Wmissing-prototypes \
  -pedantic")

add_library(fahrenheit
  src/backend_llvm.cpp
  src/ir.c
  src/verify.c)

# dependencies
find_package(LLVM 3.9 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs analysis core mcjit native)
target_link_libraries(fahrenheit ${llvm_libs})

# submodules
add_subdirectory(ext/stdplus)
target_link_libraries(fahrenheit stdplus)

SET(fahrenheit_headers
  ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
include_directories("${fahrenheit_headers}")

install(TARGETS fahrenheit DESTINATION lib)
install(DIRECTORY ${fahrenheit_headers} DESTINATION include)

if(ENABLE_FAHRENHEIT_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif(ENABLE_FAHRENHEIT_TESTS)

