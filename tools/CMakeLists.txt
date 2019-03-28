add_executable(rtrclient rtrclient.c ${mustach} ${tommyds})
set_source_files_properties("${mustach}" PROPERTIES COMPILE_FLAGS "-DNO_EXTENSION_FOR_MUSTACH -DNO_OPEN_MEMSTREAM")
set_source_files_properties("${tommyds}" PROPERTIES COMPILE_FLAGS "-Dtommy_malloc=malloc -Dtommy_calloc=calloc -Dtommy_realloc=realloc -Dtommy_free=free -include stdlib.h")
target_link_libraries(rtrclient rtrlib)
install(TARGETS rtrclient DESTINATION bin)
install(FILES "rtrclient.1" DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

add_executable(rpki-rov rpki-rov.c)
target_link_libraries(rpki-rov rtrlib)
install(TARGETS rpki-rov DESTINATION bin)
install(FILES "rpki-rov.1" DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")


set(rtrclient_pfx_templates default csv csvwithheader json)

# Generate escaped string sequence for every template file
# This runs at configure time, changes are not picked up automatically
foreach(template_name IN LISTS rtrclient_pfx_templates)
    file(READ "templates/${template_name}" template_data HEX)
    string(LENGTH "${template_data}" template_length)
    math(EXPR template_length "${template_length} - 1")

    set(TEMPLATES "${TEMPLATES}{ .name = \"${template_name}\", .template = \"")

    foreach(iter RANGE 0 ${template_length} 2)
        string(SUBSTRING ${template_data} ${iter} 2 line)
        set(TEMPLATES "${TEMPLATES}\\x${line}")
    endforeach()
    set(TEMPLATES "${TEMPLATES} \"},\n")

endforeach()

string(STRIP ${TEMPLATES} TEMPLATES)

CONFIGURE_FILE("templates.h.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/templates.h")

