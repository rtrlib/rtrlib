set(tommyds "${CMAKE_CURRENT_SOURCE_DIR}/tommyds/tommy.c" PARENT_SCOPE)

set(mustach "${CMAKE_CURRENT_SOURCE_DIR}/mustach/mustach.c" PARENT_SCOPE)
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/mustach/mustach.c" PROPERTIES COMPILE_FLAGS "-DNO_EXTENSION_FOR_MUSTACH")
