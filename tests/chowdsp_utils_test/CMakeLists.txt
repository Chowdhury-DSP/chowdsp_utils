function(setup_juce_unit_test target)
    juce_add_console_app(${target})
    setup_juce_test(${target})

    target_sources(${target} PRIVATE ../TestRunner.cpp)
    target_include_directories(${target} PUBLIC ..)

    # set coverage flags if needed
    if(CODE_COVERAGE)
        enable_coverage_flags(${target})
    endif()
endfunction()

add_subdirectory(dsp_tests)
add_subdirectory(gui_tests)
add_subdirectory(utils_tests)
