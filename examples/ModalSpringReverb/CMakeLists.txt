setup_example_plugin(ModalSpringReverb msRv)

target_link_libraries(ModalSpringReverb
    PRIVATE
        juce::juce_dsp
        chowdsp_dsp_utils
)

target_sources(ModalSpringReverb
    PRIVATE
        ModalReverbPlugin.cpp
)
