set(CMAKE_BUILD_TYPE Debug)

add_executable(test_pfx test_pfx.c)
target_link_libraries(test_pfx rtrlib)
add_executable(test_lpfst test_lpfst.c)
target_link_libraries(test_lpfst rtrlib)
add_executable(test_pfx_locks test_pfx_locks.c)
target_link_libraries(test_pfx_locks rtrlib)
