setup_example_plugin(SimpleEQ linP)

target_link_libraries(SimpleEQ
    PRIVATE
        juce::juce_dsp
        chowdsp::chowdsp_serialization
        chowdsp::chowdsp_eq
        chowdsp::chowdsp_plugin_state
)

target_sources(SimpleEQ
    PRIVATE
        PrototypeEQ.cpp
        SimpleEQPlugin.cpp
        PluginEditor.cpp
)
