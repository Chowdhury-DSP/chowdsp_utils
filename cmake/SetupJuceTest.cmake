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
    )

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
endfunction(setup_juce_test)
