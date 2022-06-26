setup_example_plugin(ModalSpringReverb msRv)

target_link_libraries(ModalSpringReverb
    PRIVATE
        chowdsp_dsp_utils
)

target_sources(ModalSpringReverb
    PRIVATE
        ModalReverbPlugin.cpp
)
