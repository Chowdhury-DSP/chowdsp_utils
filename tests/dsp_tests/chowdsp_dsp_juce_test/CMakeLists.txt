setup_catch_lib_test(chowdsp_dsp_juce_test dsp_juce_tests_lib)

target_sources(chowdsp_dsp_juce_test
    PRIVATE
        # Data Structures Tests
        data_structures_tests/COLAProcessorTest.cpp
        data_structures_tests/SmoothedBufferValueTest.cpp
        data_structures_tests/BufferMathTest.cpp

        # Convolution Tests
        convolution_tests/ConvolutionTest.cpp
        convolution_tests/IRHelpersTest.cpp

        # Sources Tests
        source_tests/NoiseTest.cpp
        source_tests/NoiseSynthTest.cpp
        source_tests/RepitchedSourceTest.cpp

        DiffuserTest.cpp
        FIRFilterTest.cpp
        LinearPhaseEQTest.cpp
        resampling_tests/VariableOversamplingTest.cpp
)
