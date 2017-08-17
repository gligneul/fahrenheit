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
macro(fahrenheit_test testname)
  set(generator ${CMAKE_CURRENT_SOURCE_DIR}/${testname}.lua)
  set(testsrc ${CMAKE_CURRENT_BINARY_DIR}/${testname}.c)
  set(testbin ${testname})
  add_custom_command(
    OUTPUT ${testsrc}
    COMMAND lua ${generator} > ${testsrc}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${generator} ${CMAKE_CURRENT_SOURCE_DIR}/test.lua)
  add_executable(${testbin} ${testsrc})
  target_link_libraries(${testbin} fahrenheit)
  add_test(${testname} ${testbin})
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

