setup_example_plugin(SimpleEQ linP)

target_link_libraries(SimpleEQ
    PRIVATE
        juce::juce_dsp
        chowdsp_serialization
        chowdsp_eq
)

target_sources(SimpleEQ
    PRIVATE
        PrototypeEQ.cpp
        SimpleEQPlugin.cpp
        PluginEditor.cpp
)
