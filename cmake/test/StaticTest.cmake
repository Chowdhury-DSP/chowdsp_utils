option(SHOULD_PASS "Should compilation succeed?" ON)

execute_process(
    COMMAND cmake --build build --parallel --target static_test_dummy_executable
    RESULT_VARIABLE res_var
)

if(SHOULD_PASS)
    if("${res_var}" STREQUAL "0")
        message(STATUS "Compiler succeeded as expected.")
    else()
        message(FATAL_ERROR "Compiler should have succeeded!")
    endif()
else()
    if("${res_var}" STREQUAL "0")
        message(FATAL_ERROR "Compiler should have failed!")
    else()
        message(STATUS "Compiler failed as expected.")
    endif()
endif()
