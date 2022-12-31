# setup_juce_test(<target-name>)
#
# Configures a JUCE unit test app
function(setup_juce_test target)
    target_compile_definitions(${target} PRIVATE
        JucePlugin_Name="DummyPlugin"
        JucePlugin_Manufacturer="chowdsp"
        JucePlugin_VersionString="9.9.9"
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_STANDALONE_APPLICATION=1
    )

    if (NOT ${target} STREQUAL "live_gui_test")
        target_sources(${target} PRIVATE ${CMAKE_SOURCE_DIR}/tests/test_utils/JUCETestRunner.cpp)
    endif()

    target_link_libraries(${target} PRIVATE
        juce::juce_events
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )

    target_include_directories(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/tests/test_utils
    )

    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)

    # set coverage flags if needed
    if(CODE_COVERAGE)
        enable_coverage_flags(${target})
    endif()

    # suppress warning from Foley's
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
endfunction(setup_juce_test)
