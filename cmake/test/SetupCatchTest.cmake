# setup_catch_test(<target-name>)
#
# Configures a Catch unit test
function(setup_catch_test target)
    add_executable(${target})

    target_sources(${target} PRIVATE ${CMAKE_SOURCE_DIR}/tests/test_utils/CatchTestRunner.cpp)

    target_link_libraries(${target} PRIVATE
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )

    target_include_directories(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/tests/test_utils
        ${CMAKE_SOURCE_DIR}/modules/common
        ${CMAKE_SOURCE_DIR}/modules/dsp
    )

    target_compile_definitions(${target}
        PUBLIC
            $<IF:$<CONFIG:DEBUG>,DEBUG=1 _DEBUG=1,NDEBUG=1 _NDEBUG=1>
    )

    if(APPLE)
        target_link_libraries(${target} PUBLIC "-framework Accelerate")
    endif()

    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)

    # set coverage flags if needed
    if(CODE_COVERAGE)
        enable_coverage_flags(${target})
    endif()

    # supress warning from Foley's
    if ((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
        target_compile_options(${target} PUBLIC /wd4458)
    endif ()

    add_custom_command(TARGET ${target}
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo "Copying $<TARGET_FILE:${target}> to test-binary"
        COMMAND ${CMAKE_COMMAND} -E make_directory test-binary
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${target}>" test-binary
    )

    include(AddDiagnosticInfo)
    add_diagnostic_info(${target})
endfunction(setup_catch_test)
