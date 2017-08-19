cmake_minimum_required(VERSION 2.6)
project(fahrenheit_tests)

add_subdirectory(.. fahrenheit)

include(CTest)

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

# Create a test case given a generator
macro(fahrenheit_test name)
  set(gen ${CMAKE_CURRENT_SOURCE_DIR}/${name}.lua)
  set(exp ${CMAKE_CURRENT_SOURCE_DIR}/${name}.exp)
  set(src ${CMAKE_CURRENT_BINARY_DIR}/${name}.c)
  set(out ${name}.out)
  set(bin ${name})

  add_custom_command(
    OUTPUT ${src}
    COMMAND lua ${gen} > ${src}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${gen} ${CMAKE_CURRENT_SOURCE_DIR}/test.lua)

  add_executable(${bin} ${src})
  target_link_libraries(${bin} fahrenheit)

  add_custom_target(
    ${out} ALL
    COMMAND ./${bin} > ${out}
    DEPENDS ${bin})

  add_test(
    NAME ${name}
    COMMAND ${CMAKE_COMMAND} -E compare_files ${exp} ${out})
endmacro(fahrenheit_test)

# Test cases
fahrenheit_test(basic)
fahrenheit_test(getarg)
fahrenheit_test(mem)
fahrenheit_test(cast)
fahrenheit_test(binop)
fahrenheit_test(cmpjmp)
fahrenheit_test(util)
fahrenheit_test(call)
fahrenheit_test(phi)

