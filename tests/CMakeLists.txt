include(CTest)

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

