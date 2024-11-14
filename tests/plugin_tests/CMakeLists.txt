setup_juce_lib(plugin_tests_lib 
    juce::juce_dsp
    juce::juce_graphics
    chowdsp::chowdsp_parameters
    chowdsp::chowdsp_dsp_data_structures
    chowdsp::chowdsp_plugin_base
    chowdsp::chowdsp_rhythm
    chowdsp::chowdsp_gui
    chowdsp::chowdsp_plugin_state
    chowdsp::chowdsp_plugin_utils
    chowdsp::chowdsp_presets_v2
    chowdsp::chowdsp_fuzzy_search
    chowdsp::chowdsp_version
)

if(TARGET clap_juce_extensions)
    target_link_libraries(plugin_tests_lib
        PUBLIC
            clap_juce_extensions
            chowdsp::chowdsp_clap_extensions
    )
endif()

target_compile_definitions(plugin_tests_lib
    PUBLIC
        JucePlugin_Name="TestPlugin"
        JucePlugin_VersionString="9.9.9"
)

add_subdirectory(chowdsp_version_test)
add_subdirectory(chowdsp_parameters_test)
add_subdirectory(chowdsp_presets_test)
add_subdirectory(chowdsp_presets_v2_test)
add_subdirectory(chowdsp_plugin_base_test)
add_subdirectory(chowdsp_plugin_utils_test)
add_subdirectory(chowdsp_plugin_state_test)
add_subdirectory(chowdsp_fuzzy_search_test)
