setup_example_plugin(SignalGenerator Sggb)

target_link_libraries(SignalGenerator
    PRIVATE
        juce::juce_dsp
        chowdsp::chowdsp_dsp_utils
        chowdsp::chowdsp_sources
        chowdsp::chowdsp_waveshapers
        chowdsp::chowdsp_plugin_state
        chowdsp::chowdsp_gui
)

target_sources(SignalGenerator
    PRIVATE
        SignalGeneratorPlugin.cpp
)

include(AddDiagnosticInfo)
add_diagnostic_info(SignalGenerator)
