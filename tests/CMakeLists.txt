include(CTest)

add_executable(basic basic.c)
target_link_libraries(basic fahrenheit)
add_test(basic basic)

