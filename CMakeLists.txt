project(rtrlib C)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu99 -fstack-protector-all")
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wextra -Wformat-security -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement -Winit-self -Waggregate-return -Wmissing-format-attribute -Wundef -Wbad-function-cast -Wwrite-strings -Wformat=2")
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if(GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wfree-nonheap-object")
    endif()
else()
endif(CMAKE_BUILD_TYPE STREQUAL Debug)

include_directories(.)

find_package(codecov)

find_package(Threads REQUIRED)
if(NOT APPLE)
    find_library(RT_LIB rt)
endif(NOT APPLE)
include(UseMultiArch) # needed for debian packaging

set(RTRLIB_SRC rtrlib/rtr_mgr.c rtrlib/lib/utils.c rtrlib/lib/alloc_utils.c rtrlib/lib/convert_byte_order.c
    rtrlib/lib/ip.c rtrlib/lib/ipv4.c rtrlib/lib/ipv6.c rtrlib/lib/log.c
    rtrlib/pfx/trie/trie.c rtrlib/pfx/trie/trie-pfx.c rtrlib/transport/transport.c
    rtrlib/transport/tcp/tcp_transport.c rtrlib/rtr/rtr.c rtrlib/rtr/packets.c
    rtrlib/spki/hashtable/ht-spkitable.c rtrlib/spki/hashtable/tommyds-1.8/tommy.c)
set(RTRLIB_LINK ${RT_LIB} ${CMAKE_THREAD_LIBS_INIT})

find_package(LibSSH 0.6.0 QUIET)
if(LIBSSH_FOUND AND NOT NOSSH)
    set(LIBSSH_060 1)
    message(STATUS "libssh found, building librtr with SSH 0.6.0 support")
else()
    find_package(LibSSH 0.5.0)
    if(LIBSSH_FOUND AND NOT NOSSH)
        set(LIBSSH_050 1)
        message(STATUS "libssh found, building librtr with SSH 0.5.0 support")
    endif(LIBSSH_FOUND AND NOT NOSSH)
endif(LIBSSH_FOUND AND NOT NOSSH)

if(LIBSSH_050 OR LIBSSH_060)
        set(RTRLIB_HAVE_LIBSSH 1)
        include_directories(${LIBSSH_INCLUDE_DIRS})
        set(RTRLIB_SRC ${RTRLIB_SRC} rtrlib/transport/ssh/ssh_transport.c)
        set(RTRLIB_LINK ${RTRLIB_LINK} ${LIBSSH_LIBRARIES})
else()
        message(WARNING "libssh not found, building librtr without SSH support")
endif(LIBSSH_050 OR LIBSSH_060)

#doxygen target
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doc ALL ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doxygen/Doxyfile
        COMMENT "Generating doxygen API documentation" VERBATIM)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs DESTINATION share/doc/rtrlib)
endif(DOXYGEN_FOUND)

add_library(rtrlib SHARED ${RTRLIB_SRC})
add_coverage(rtrlib)
target_link_libraries(rtrlib ${RTRLIB_LINK})

if(UNIT_TESTING)
    add_library(rtrlib_static STATIC ${RTRLIB_SRC})
    add_coverage(rtrlib_static)
    target_link_libraries(rtrlib_static ${RTRLIB_LINK})
endif(UNIT_TESTING)

ADD_SUBDIRECTORY(tools)

ADD_SUBDIRECTORY(doxygen/examples)

include(AddTest)
ADD_SUBDIRECTORY(tests)
ENABLE_TESTING()
ADD_TEST(test_pfx tests/test_pfx)
ADD_TEST(test_trie tests/test_trie)
#ADD_TEST(test_pfx_locks tests/test_pfx_locks)

ADD_TEST(test_ht_spkitable tests/test_ht_spkitable)
ADD_TEST(test_ht_spkitable_locks tests/test_ht_spkitable_locks)

ADD_TEST(test_live_validation tests/test_live_validation)

ADD_TEST(test_ipaddr tests/test_ipaddr)

ADD_TEST(test_getbits tests/test_getbits)

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
    set (PKG_CONFIG_REQUIRES "libssh >= 0.5.0")
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

coverage_evaluate()
