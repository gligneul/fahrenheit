cmake_minimum_required(VERSION 2.6)
project(fahrenheit_examples)

add_subdirectory(.. fahrenheit)

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

add_executable(hello hello.c)
target_link_libraries(hello fahrenheit)

add_executable(add add.c)
target_link_libraries(add fahrenheit)

add_executable(phi phi.c)
target_link_libraries(phi fahrenheit)

add_executable(array array.c)
target_link_libraries(array fahrenheit)

