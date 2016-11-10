enable_testing()
include(CTest)

function (ADD_RTR_TEST _testName _testSource)
    add_executable(${_testName} ${_testSource})
    target_link_libraries(${_testName} ${ARGN})
    add_test(${_testName} ${CMAKE_CURRENT_BINARY_DIR}/${_testName})
endfunction (ADD_RTR_TEST)


function (ADD_RTR_UNIT_TEST _testName _testSource)
    add_rtr_test(${_testName} ${_testSource} ${ARGN})
    add_coverage(${_testName})
endfunction (ADD_RTR_UNIT_TEST)


function (WRAP_FUNCTIONS _testName)
    set(template " -Wl,--wrap=")
    set(linkopts "")
    foreach(f ${ARGN})
        string(CONCAT linkopts ${linkopts} ${template} ${f})
    endforeach()

    get_target_property(temp ${_testName} COMPILE_FLAGS)
    if (temp)
        string(CONCAT linkopts ${temp} ${linkopts})
    endif()
    set_target_properties(${_testName}
                      PROPERTIES
                      LINK_FLAGS ${linkopts})
endfunction (WRAP_FUNCTIONS)
