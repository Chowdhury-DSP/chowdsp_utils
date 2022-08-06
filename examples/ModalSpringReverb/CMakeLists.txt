setup_example_plugin(ModalSpringReverb msRv)

target_link_libraries(ModalSpringReverb
    PRIVATE
        juce::juce_dsp
        chowdsp::chowdsp_dsp_utils
        chowdsp::chowdsp_sources
)

target_sources(ModalSpringReverb
    PRIVATE
        ModalReverbPlugin.cpp
)
