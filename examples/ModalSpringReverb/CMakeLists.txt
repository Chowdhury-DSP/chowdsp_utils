message(STATUS "Configuring example: ModalSpringReverb")

setup_example_plugin(ModalSpringReverb msRv)

target_sources(ModalSpringReverb
    PRIVATE
        ModalReverbPlugin.cpp
)
