add_executable(rtrclient rtrclient.c)
target_link_libraries(rtrclient rtrlib)
install(TARGETS rtrclient DESTINATION bin)

add_executable(rpki-rov rpki-rov.c)
target_link_libraries(rpki-rov rtrlib)
install(TARGETS rpki-rov DESTINATION bin)

