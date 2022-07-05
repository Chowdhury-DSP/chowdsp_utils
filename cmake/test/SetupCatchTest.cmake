# setup_catch_test(<target-name>)
#
# Configures a Catch unit test
function(setup_catch_test target)
    add_executable(${target})
    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)

    setup_chowdsp_lib(${target}_lib ${ARGV})
    target_link_libraries(${target} PRIVATE ${target}_lib)

    target_sources(${target} PRIVATE ${CMAKE_SOURCE_DIR}/tests/test_utils/CatchTestRunner.cpp)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/tests/test_utils)
    target_link_libraries(${target} PRIVATE
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )

    # set coverage flags if needed
    if(CODE_COVERAGE)
        enable_coverage_flags(${target})
        enable_coverage_flags(${target}_lib)
    endif()

    add_custom_command(TARGET ${target}
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo "Copying $<TARGET_FILE:${target}> to test-binary"
        COMMAND ${CMAKE_COMMAND} -E make_directory test-binary
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${target}>" test-binary
    )
endfunction(setup_catch_test)
