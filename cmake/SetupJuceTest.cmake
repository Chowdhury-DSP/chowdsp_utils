# setup_juce_test(<target-name>)
#
# Configures a JUCE unit test app
function(setup_juce_test target)
    juce_generate_juce_header(${target})

    target_compile_definitions(${target} PRIVATE
        JucePlugin_Name="DummyPlugin"
        JucePlugin_Manufacturer="chowdsp"
        JucePlugin_VersionString="9.9.9"
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_MODAL_LOOPS_PERMITTED=1
    )

    target_link_libraries(${target} PRIVATE
        juce::juce_dsp
        chowdsp_dsp
        chowdsp_gui
        chowdsp_plugin_utils
        foleys_gui_magic
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )

    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)

    # supress warning from Foley's
    if ((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
        target_compile_options(${target} PUBLIC /wd4458)
    endif ()
endfunction(setup_juce_test)