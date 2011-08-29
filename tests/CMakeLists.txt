add_library(rtrlib-test ${CMAKE_SOURCE_DIR}/rtrlib/lib/test.c)

add_executable(test_pfx test_pfx.c)
add_executable(test_lpfst test_lpfst.c)
target_link_libraries(test_pfx rtrlib rtrlib-test)
target_link_libraries(test_lpfst rtrlib rtrlib-test)
