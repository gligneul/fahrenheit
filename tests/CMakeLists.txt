include(CTest)

function(fahrenheit_test testname)
  add_executable(${testname} ${testname}.c)
  target_link_libraries(${testname} fahrenheit)
  add_test(${testname} ${testname})
endfunction(fahrenheit_test)

fahrenheit_test(basic)

