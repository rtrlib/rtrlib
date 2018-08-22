add_executable(rtrclient rtrclient.c)
target_link_libraries(rtrclient rtrlib)
install(TARGETS rtrclient DESTINATION bin)
install(FILES "rtrclient.1" DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

add_executable(rpki-rov rpki-rov.c)
target_link_libraries(rpki-rov rtrlib)
install(TARGETS rpki-rov DESTINATION bin)
install(FILES "rpki-rov.1" DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

