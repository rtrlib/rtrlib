set(CMAKE_BUILD_TYPE Debug)

add_executable(test_pfx test_pfx.c)
target_link_libraries(test_pfx rtrlib_static)
add_coverage(test_pfx)

add_executable(test_trie test_trie.c)
target_link_libraries(test_trie rtrlib_static)
add_coverage(test_trie)

add_executable(test_pfx_locks test_pfx_locks.c)
target_link_libraries(test_pfx_locks rtrlib_static)
add_coverage(test_pfx_locks)

add_executable(test_ht_spkitable test_ht_spkitable.c)
target_link_libraries(test_ht_spkitable rtrlib_static)
add_coverage(test_ht_spkitable)

add_executable(test_ht_spkitable_locks test_ht_spkitable_locks.c)
target_link_libraries(test_ht_spkitable_locks rtrlib_static)
add_coverage(test_ht_spkitable_locks)

add_executable(test_live_validation test_live_validation.c)
target_link_libraries(test_live_validation rtrlib_static)
add_coverage(test_live_validation)

add_executable(test_live_fetching test_live_fetching.c)
target_link_libraries(test_live_fetching rtrlib_static)
add_coverage(test_live_fetching)

add_executable(test_ipaddr test_ipaddr.c)
target_link_libraries(test_ipaddr rtrlib_static)
add_coverage(test_ipaddr)

add_executable(test_getbits test_getbits.c)
target_link_libraries(test_getbits rtrlib_static)
add_coverage(test_getbits)

add_executable(test_dynamic_groups test_dynamic_groups.c)
target_link_libraries(test_dynamic_groups rtrlib_static)
add_coverage(test_dynamic_groups)

add_executable(test_aspa test_aspa.c)
target_link_libraries(test_aspa rtrlib_static)
add_coverage(test_aspa)

add_executable(test_aspa_array test_aspa_array.c)
target_link_libraries(test_aspa_array rtrlib_static)
add_coverage(test_aspa_array)

add_executable(test_as_path_verification test_as_path_verification.c)
target_link_libraries(test_as_path_verification rtrlib_static)
add_coverage(test_as_path_verification)


if(RTRLIB_BGPSEC_ENABLED)
    add_executable(test_bgpsec test_bgpsec.c)
    target_link_libraries(test_bgpsec rtrlib_static)
    add_coverage(test_bgpsec)
endif(RTRLIB_BGPSEC_ENABLED)


if(UNIT_TESTING AND NOT APPLE)
    find_package(CMocka REQUIRED)
    add_subdirectory(unittests)
endif(UNIT_TESTING AND NOT APPLE)
