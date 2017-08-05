cmake_minimum_required(VERSION 2.6)
project(fahrenheit)

set(FAHRENHEIT_VERSION_MAJOR 0)
set(FAHRENHEIT_VERSION_MINOR 1)
configure_file(
  "${PROJECT_SOURCE_DIR}/include/fahrenheit/version.h.in"
  "${PROJECT_BINARY_DIR}/include/fahrenheit/version.h")

add_library(fahrenheit
  src/backend_llvm.cpp
  src/instructions.c
  src/ir.c
  src/printer.c
  src/verify.c)

# dependencies
message(STATUS "LLVM_DIR: ${LLVM_DIR}")
find_package(LLVM REQUIRED)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs analysis core mcjit native)
target_link_libraries(fahrenheit ${llvm_libs})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

# submodules
add_subdirectory(ext/stdplus)
target_link_libraries(fahrenheit stdplus)

SET(fahrenheit_headers
  ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)
target_include_directories(fahrenheit PUBLIC ${fahrenheit_headers})
include_directories("${fahrenheit_headers}")

install(TARGETS fahrenheit DESTINATION lib)
install(DIRECTORY ${fahrenheit_headers} DESTINATION include)

if(FAHRENHEIT_DEV)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -std=c++11 \
    -Wall \
    -Werror \
    -Wshadow \
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

  enable_testing()
  add_subdirectory(tests)
endif(FAHRENHEIT_DEV)

