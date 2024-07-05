setup_example_plugin(SimpleReverb sRvb)

target_link_libraries(SimpleReverb
    PRIVATE
        juce::juce_dsp
        chowdsp::chowdsp_reverb
        chowdsp::chowdsp_sources
        chowdsp::chowdsp_plugin_state
        chowdsp::chowdsp_gui
)

target_sources(SimpleReverb
    PRIVATE
        SimpleReverbPlugin.cpp
)
