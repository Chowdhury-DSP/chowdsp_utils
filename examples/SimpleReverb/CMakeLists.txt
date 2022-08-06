setup_example_plugin(SimpleReverb sRvb)

target_link_libraries(SimpleReverb
    PRIVATE
        juce::juce_dsp
        chowdsp::chowdsp_reverb
        chowdsp::chowdsp_sources
)

target_sources(SimpleReverb
    PRIVATE
        SimpleReverbPlugin.cpp
)
