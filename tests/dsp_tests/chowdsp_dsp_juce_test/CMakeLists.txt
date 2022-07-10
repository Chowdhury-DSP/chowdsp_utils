add_executable(chowdsp_dsp_juce_test)
setup_juce_test(chowdsp_dsp_juce_test)

target_link_libraries(chowdsp_dsp_juce_test
    PRIVATE
        juce_dsp
        chowdsp_dsp_data_structures
        chowdsp_eq
        chowdsp_reverb
        chowdsp_plugin_base
)

target_sources(chowdsp_dsp_juce_test
    PRIVATE
        # Data Structures Tests
        COLAProcessorTest.cpp
        SmoothedBufferValueTest.cpp
        BufferViewTest.cpp

        # Filters Tests
        FIRFilterTest.cpp

        # EQ Tests
        LinearPhaseEQTest.cpp

        # Reverb Tests
        DiffuserTest.cpp
)
