add_executable(rtrclient rtrclient.c)
target_link_libraries(rtrclient rtrlib)
set_target_properties(rtrclient PROPERTIES SKIP_BUILD_RPATH TRUE)
install(TARGETS rtrclient DESTINATION bin)
install(FILES "rtrclient.1" DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

add_executable(rpki-rov rpki-rov.c)
target_link_libraries(rpki-rov rtrlib)
set_target_properties(rpki-rov PROPERTIES SKIP_BUILD_RPATH TRUE)
install(TARGETS rpki-rov DESTINATION bin)
install(FILES "rpki-rov.1" DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

