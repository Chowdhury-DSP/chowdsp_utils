add_executable(chowdsp_dsp_juce_test)
setup_juce_test(chowdsp_dsp_juce_test)

target_link_libraries(chowdsp_dsp_juce_test
    PRIVATE
        juce_dsp
        chowdsp_dsp_data_structures
        chowdsp_eq
        chowdsp_reverb
        chowdsp_sources
        chowdsp_plugin_base
)

target_sources(chowdsp_dsp_juce_test
    PRIVATE
        # Data Structures Tests
        data_structures_tests/COLAProcessorTest.cpp
        data_structures_tests/SmoothedBufferValueTest.cpp
        data_structures_tests/BufferViewTest.cpp
        data_structures_tests/BufferMathTest.cpp

        # Filters Tests
        FIRFilterTest.cpp

        # EQ Tests
        LinearPhaseEQTest.cpp

        # Reverb Tests
        DiffuserTest.cpp

        # Convolution Tests
        convolution_tests/ConvolutionTest.cpp
        convolution_tests/IRHelpersTest.cpp

        # Sources Tests
        source_tests/NoiseTest.cpp
        source_tests/RepitchedSourceTest.cpp

        # Resampling Tests
        resampling_tests/VariableOversamplingTest.cpp
)
