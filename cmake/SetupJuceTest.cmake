add_library(juce_test_modules STATIC)

target_link_libraries(juce_test_modules
    PUBLIC
        juce::juce_audio_utils
        juce::juce_dsp
        foleys_gui_magic
        chowdsp_dsp
        chowdsp_gui
        chowdsp_plugin_utils
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(juce_test_modules
    PUBLIC
        JucePlugin_Name="DummyPlugin"
        JucePlugin_Manufacturer="chowdsp"
        JucePlugin_VersionString="9.9.9"
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_STANDALONE_APPLICATION=1
    INTERFACE
        $<TARGET_PROPERTY:juce_test_modules,COMPILE_DEFINITIONS>
)

target_include_directories(juce_test_modules
    INTERFACE
        $<TARGET_PROPERTY:juce_test_modules,INCLUDE_DIRECTORIES>
)

set_target_properties(juce_test_modules PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    VISIBILITY_INLINES_HIDDEN TRUE
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

# setup_juce_test(<target-name>)
#
# Configures a JUCE unit test app
function(setup_juce_test target)
    juce_generate_juce_header(${target})
    target_link_libraries(${target} PRIVATE juce_test_modules)

    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)

    # supress warning from Foley's
    if ((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
        target_compile_options(${target} PUBLIC /wd4458)
    endif ()
endfunction(setup_juce_test)