add_executable(rtrclient rtrclient.c)
target_link_libraries(rtrclient rtrlib)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/rtrclient DESTINATION bin)

add_executable(cli-validator cli-validator.c)
target_link_libraries(cli-validator rtrlib)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cli-validator DESTINATION bin)

