

add_rtr_unit_test(test_packets_static test_packets_static.c rtrlib_static cmocka)
wrap_functions(test_packets_static lrtr_get_monotonic_time tr_send_all)

add_rtr_unit_test(test_packets test_packets.c rtrlib_static cmocka)
wrap_functions(test_packets rtr_change_socket_state tr_send_all)

if(RTRLIB_BGPSEC_ENABLED)
    add_rtr_unit_test(test_bgpsec_utils test_bgpsec_utils.c rtrlib_static cmocka)

    add_rtr_unit_test(test_bgpsec_validation test_bgpsec_validation.c rtrlib_static cmocka)
    wrap_functions(test_bgpsec_validation check_router_keys req_stream_size align_byte_sequence hash_byte_sequence validate_signature spki_table_search_by_ski)

    add_rtr_unit_test(test_bgpsec_signing test_bgpsec_signing.c rtrlib_static cmocka)
    wrap_functions(test_bgpsec_signing load_private_key req_stream_size align_byte_sequence hash_byte_sequence ECDSA_size sign_byte_sequence)
endif(RTRLIB_BGPSEC_ENABLED)
