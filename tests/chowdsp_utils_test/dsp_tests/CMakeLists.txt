target_sources(chowdsp_utils_tests PRIVATE
    AudioTimerTest.cpp
    BBDTest.cpp
    BypassTest.cpp
    GainTest.cpp
    PannerTest.cpp
    PitchShiftTest.cpp
    SoftClipperTest.cpp
    TunerTest.cpp

    convolution_tests/ConvolutionTest.cpp
    convolution_tests/IRHelpersTest.cpp

    math_tests/CombinatoricsTest.cpp
    math_tests/FloatVectorOperationsTest.cpp
    math_tests/PolynomialsTest.cpp
    math_tests/PowerTest.cpp

    data_structure_tests/DoubleBufferTest.cpp
    data_structure_tests/LookupTableTest.cpp
    data_structure_tests/RebufferProcessorTest.cpp
    data_structure_tests/SmoothedBufferValueTest.cpp
    data_structure_tests/ScopedValueTest.cpp

    source_tests/NoiseTest.cpp
    source_tests/SineTest.cpp
    source_tests/SawtoothTest.cpp
    source_tests/SquareTest.cpp

    filter_tests/FirstOrderFiltersTest.cpp
    filter_tests/ModalFilterTest.cpp
    filter_tests/NthOrderFilterTest.cpp
    filter_tests/SecondOrderFiltersTest.cpp
    filter_tests/ShelfFilterTest.cpp

    resampling_tests/ResamplerTest.cpp
    resampling_tests/UpsampleDownsampleTest.cpp
    resampling_tests/VariableOversamplingTest.cpp

    simd_tests/SIMDArithmeticTest.cpp
    simd_tests/SIMDComplexTest.cpp
    simd_tests/SIMDFastMathTest.cpp
    simd_tests/SIMDLogicTest.cpp
    simd_tests/SIMDSmoothedValueTest.cpp
    simd_tests/SIMDSpecialMathTest.cpp
    simd_tests/SIMDUtilsTest.cpp

    wdf_tests/OmegaTest.cpp
    wdf_tests/RTypeTest.cpp
    wdf_tests/SIMDWDFTest.cpp
    wdf_tests/StaticWDFTest.cpp
    wdf_tests/WDFTest.cpp
)
