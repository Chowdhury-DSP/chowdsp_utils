setup_example_plugin(SignalGenerator Sggb)

target_link_libraries(SignalGenerator
    PRIVATE
        chowdsp_waveshapers
        chowdsp_dsp_utils
)

target_sources(SignalGenerator
    PRIVATE
        SignalGeneratorPlugin.cpp
)
