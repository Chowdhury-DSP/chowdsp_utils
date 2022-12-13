setup_example_plugin(ModalSpringReverb msRv)

target_link_libraries(ModalSpringReverb
    PRIVATE
        juce::juce_dsp
        chowdsp::chowdsp_modal_dsp
        chowdsp::chowdsp_sources
        chowdsp::chowdsp_plugin_state
)

target_sources(ModalSpringReverb
    PRIVATE
        ModalReverbPlugin.cpp
)
