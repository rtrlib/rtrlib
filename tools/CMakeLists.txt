add_executable(rtrclient rtrclient.c)
target_link_libraries(rtrclient rtrlib)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/rtrclient DESTINATION bin)
