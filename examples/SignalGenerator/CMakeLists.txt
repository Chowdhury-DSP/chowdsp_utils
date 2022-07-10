setup_example_plugin(SignalGenerator Sggb)

target_link_libraries(SignalGenerator
    PRIVATE
        juce::juce_dsp
        chowdsp::chowdsp_waveshapers
        chowdsp::chowdsp_dsp_utils
)

target_sources(SignalGenerator
    PRIVATE
        SignalGeneratorPlugin.cpp
)
