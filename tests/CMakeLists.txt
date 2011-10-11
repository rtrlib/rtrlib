set(CMAKE_BUILD_TYPE Debug)

add_library(rtrlib-test ${CMAKE_SOURCE_DIR}/rtrlib/lib/test.c)

add_executable(test_pfx test_pfx.c)
target_link_libraries(test_pfx rtrlib rtrlib-test)
add_executable(test_lpfst test_lpfst.c)
target_link_libraries(test_lpfst rtrlib rtrlib-test)
add_executable(test_pfx_locks test_pfx_locks.c)
target_link_libraries(test_pfx_locks rtrlib rtrlib-test)
