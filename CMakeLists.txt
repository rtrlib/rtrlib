project(rtrlib)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_C_FLAGS "-std=gnu99")

include_directories(.)
find_package (Threads REQUIRED)
find_library (RT_LIB rt)

set(RTRLIB_SRC rtrlib/lib/utils.c rtrlib/lib/ip.c rtrlib/lib/ipv4.c rtrlib/lib/ipv6.c rtrlib/lib/log.c
    rtrlib/pfx/lpfst/lpfst.c rtrlib/pfx/lpfst/lpfst-pfx.c rtrlib/transport/transport.c
    rtrlib/transport/tcp/tcp_transport.c rtrlib/rtr/rtr.c rtrlib/rtr/packets.c)
set(RTRLIB_LINK ${RT_LIB} ${CMAKE_THREAD_LIBS_INIT})

find_package(LibSSH 0.4.0)
if(LIBSSH_FOUND)
    set(RTRLIB_HAVE_LIBSSH 1)
    message(STATUS "libssh found, building librtr with SSH support")

    include_directories(LIBSSH_INCLUDE_DIRS)
    set(RTRLIB_SRC ${RTRLIB_SRC} rtrlib/transport/ssh/ssh_transport.c)
    set(RTRLIB_LINK ${RTRLIB_LINK} ${LIBSSH_LIBRARIES})
else()
    message(WARNING "libssh not found, building librtr without SSH support")
endif(LIBSSH_FOUND)

#doxygen target
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generating doxygen API documentation" VERBATIM)
endif(DOXYGEN_FOUND)

add_library(rtrlib SHARED ${RTRLIB_SRC})
target_link_libraries(rtrlib ${RTRLIB_LINK})

ADD_SUBDIRECTORY(tools)

ADD_SUBDIRECTORY(tests)
ENABLE_TESTING()
ADD_TEST(test_pfx tests/test_pfx)
ADD_TEST(test_lpfst tests/test_lpfst)

#install lib
set (RTRLIB_VERSION_MAJOR 0)
set (RTRLIB_VERSION_MINOR 1)
set (RTRLIB_VERSION_PATCH 0)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/rtrlib/rtrlib.h.cmake ${CMAKE_SOURCE_DIR}/rtrlib/rtrlib.h)
set(LIBRARY_VERSION ${RTRLIB_VERSION_MAJOR}.${RTRLIB_VERSION_MINOR}.${RTRLIB_VERSION_PATCH})
set(LIBRARY_SOVERSION ${RTRLIB_VERSION_MAJOR})
set_target_properties(rtrlib PROPERTIES SOVERSION ${LIBRARY_SOVERSION} VERSION ${LIBRARY_VERSION} OUTPUT_NAME rtr)
install(TARGETS rtrlib LIBRARY DESTINATION lib)

#install includes
install(DIRECTORY rtrlib/ DESTINATION include/rtrlib 
    FILES_MATCHING PATTERN "*.h"
    PATTERN rtrlib/lib/test.h EXCLUDE
    PATTERN rtrlib/rtr_mgr. EXCLUDE)
install(PROGRAMS tools/rtrclient DESTINATION bin)


#add uninstall target
configure_file(
    "cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

