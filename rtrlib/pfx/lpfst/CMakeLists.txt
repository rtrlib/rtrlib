cmake_minimum_required (VERSION 2.6)
project (pfx_test)
set(CMAKE_C_FLAGS "-std=gnu99 ")
set(CMAKE_BUILD_TYPE Debug)
#add_executable(test test.c ../../lib/ip.c ../../lib/ipv4.c ../../lib/ipv6.c ../../lib/log.c)
add_executable(test_lpfst test_lpfst.c ../../lib/ip.c ../../lib/ipv4.c ../../lib/ipv6.c ../../lib/log.c ../../lib/test.c)
add_executable(test_pfx test_pfx.c lpfst.c lpfst-pfx.c ../../lib/ip.c ../../lib/ipv4.c ../../lib/ipv6.c ../../lib/log.c ../../lib/test.c )
#ENABLE_TESTING()

