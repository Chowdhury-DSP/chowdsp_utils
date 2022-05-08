setup_example_plugin(SimpleReverb sRvb)

target_link_libraries(SimpleReverb
    PRIVATE
        juce::juce_dsp
        chowdsp_reverb
)

target_sources(SimpleReverb
    PRIVATE
        SimpleReverbPlugin.cpp
)
