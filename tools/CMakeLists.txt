add_executable(rtrclient rtrclient.c)
target_link_libraries(rtrclient rtrlib)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/rtrclient DESTINATION bin)

add_executable(rpki-rov rpki-rov.c)
target_link_libraries(rpki-rov rtrlib)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/rpki-rov DESTINATION bin)

