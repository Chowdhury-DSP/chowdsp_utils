setup_example_plugin(SignalGenerator Sggb)

target_link_libraries(SignalGenerator
    PRIVATE
        juce::juce_dsp
        chowdsp::chowdsp_dsp_utils
        chowdsp::chowdsp_sources
        chowdsp::chowdsp_waveshapers
)

target_sources(SignalGenerator
    PRIVATE
        SignalGeneratorPlugin.cpp
)
