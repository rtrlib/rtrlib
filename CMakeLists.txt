project(rtrlib)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu99 -fstack-protector-all")
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wextra -Wfree-nonheap-object -Wformat-security -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Winit-self -Waggregate-return -Wmissing-format-attribute -Wundef -Wbad-function-cast -Wwrite-strings -Wformat=2")
else()
endif(CMAKE_BUILD_TYPE STREQUAL Debug)

include_directories(.)
find_package(Threads REQUIRED)
if(NOT APPLE)
    find_library(RT_LIB rt)
endif(NOT APPLE)
include(UseMultiArch) # needed for debian packaging

set(RTRLIB_SRC rtrlib/rtr_mgr.c rtrlib/lib/utils.c rtrlib/lib/ip.c rtrlib/lib/ipv4.c rtrlib/lib/ipv6.c rtrlib/lib/log.c
    rtrlib/pfx/lpfst/lpfst.c rtrlib/pfx/lpfst/lpfst-pfx.c rtrlib/transport/transport.c
    rtrlib/transport/tcp/tcp_transport.c rtrlib/rtr/rtr.c rtrlib/rtr/packets.c rtrlib/spki/hashtable/ht-spkitable.c rtrlib/spki/hashtable/tommyds-1.8/tommy.c)
set(RTRLIB_LINK ${RT_LIB} ${CMAKE_THREAD_LIBS_INIT})

find_package(LibSSH 0.6.0)
if(LIBSSH_FOUND)
    set(RTRLIB_HAVE_LIBSSH 1)
    message(STATUS "libssh found, building librtr with SSH support")

    include_directories(${LIBSSH_INCLUDE_DIRS})
    set(RTRLIB_SRC ${RTRLIB_SRC} rtrlib/transport/ssh/ssh_transport.c)
    set(RTRLIB_LINK ${RTRLIB_LINK} ${LIBSSH_LIBRARIES})
else()
    message(WARNING "libssh not found, building librtr without SSH support")
endif(LIBSSH_FOUND)

#doxygen target
find_package(Doxygen)
if(DOXYGEN_FOUND)
	add_custom_target(doc ALL ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doxygen/Doxyfile
	    COMMENT "Generating doxygen API documentation" VERBATIM)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs DESTINATION share/doc/rtrlib)
endif(DOXYGEN_FOUND)

add_library(rtrlib SHARED ${RTRLIB_SRC})
target_link_libraries(rtrlib ${RTRLIB_LINK})

ADD_SUBDIRECTORY(tools)

ADD_SUBDIRECTORY(doxygen/examples)

ADD_SUBDIRECTORY(tests)
ENABLE_TESTING()
ADD_TEST(test_pfx tests/test_pfx)
ADD_TEST(test_lpfst tests/test_lpfst)
ADD_TEST(test_pfx_locks tests/test_pfx_locks)

ADD_TEST(test_ht_spkitable tests/test_ht_spkitable)
ADD_TEST(test_ht_spkitable_locks tests/test_ht_spkitable_locks)


#install lib
set (RTRLIB_VERSION_MAJOR 0)
set (RTRLIB_VERSION_MINOR 3)
set (RTRLIB_VERSION_PATCH 6)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/rtrlib/rtrlib.h.cmake ${CMAKE_SOURCE_DIR}/rtrlib/rtrlib.h)
set(LIBRARY_VERSION ${RTRLIB_VERSION_MAJOR}.${RTRLIB_VERSION_MINOR}.${RTRLIB_VERSION_PATCH})
set(LIBRARY_SOVERSION ${RTRLIB_VERSION_MAJOR})
set_target_properties(rtrlib PROPERTIES SOVERSION ${LIBRARY_SOVERSION} VERSION ${LIBRARY_VERSION} OUTPUT_NAME rtr)
install(TARGETS rtrlib LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/)

#install includes
install(DIRECTORY rtrlib/ DESTINATION include/rtrlib
    FILES_MATCHING PATTERN "*.h"
    PATTERN rtrlib/rtr_mgr. EXCLUDE)

#pkgconfig file
if(LIBSSH_FOUND)
    set (PKG_CONFIG_REQUIRES "libssh >= 0.6.0")
endif(LIBSSH_FOUND)
set (PKG_CONFIG_LIBDIR     "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
# '#include <rtrlib/rtrlib.h>' includes the "rtrlib/"
set (PKG_CONFIG_INCLUDEDIR "\${prefix}/include")
set (PKG_CONFIG_LIBS       "-L\${libdir} -lrtr")
set (PKG_CONFIG_CFLAGS     "-I\${includedir}")

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/pkg-config.pc.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
)
install (
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
#add uninstall target
configure_file(
    "cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
