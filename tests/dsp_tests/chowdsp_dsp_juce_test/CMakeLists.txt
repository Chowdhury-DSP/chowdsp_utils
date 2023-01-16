setup_catch_juce_test(chowdsp_dsp_juce_test
    juce::juce_dsp
    chowdsp::chowdsp_dsp_data_structures
    chowdsp::chowdsp_eq
    chowdsp::chowdsp_reverb
    chowdsp::chowdsp_sources
    chowdsp::chowdsp_plugin_base
)

target_sources(chowdsp_dsp_juce_test
    PRIVATE
        # Data Structures Tests
        data_structures_tests/COLAProcessorTest.cpp
        data_structures_tests/SmoothedBufferValueTest.cpp
        data_structures_tests/BufferViewTest.cpp
        data_structures_tests/BufferMathTest.cpp
        data_structures_tests/BufferConversionTest.cpp

        # Convolution Tests
        convolution_tests/ConvolutionTest.cpp
        convolution_tests/IRHelpersTest.cpp

        # Sources Tests
        source_tests/NoiseTest.cpp
        source_tests/RepitchedSourceTest.cpp

        DiffuserTest.cpp
        FIRFilterTest.cpp
        LinearPhaseEQTest.cpp
        resampling_tests/VariableOversamplingTest.cpp
)
