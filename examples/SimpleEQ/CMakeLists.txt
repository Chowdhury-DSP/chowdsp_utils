setup_example_plugin(SimpleEQ linP)

target_link_libraries(SimpleEQ
    PRIVATE
        juce::juce_dsp
        chowdsp::chowdsp_serialization
        chowdsp::chowdsp_eq
        chowdsp::chowdsp_plugin_state
        chowdsp::chowdsp_visualizers
    )

target_sources(SimpleEQ
    PRIVATE
        SimpleEQPlugin.cpp
        PluginEditor.cpp
        FilterPlots.cpp
)
target_compile_definitions(SimpleEQ
    PRIVATE
        CHOWDSP_BUFFER_MAX_NUM_CHANNELS=2
)
