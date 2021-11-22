target_sources(chowdsp_utils_tests PRIVATE
    BBDTest.cpp
    BypassTest.cpp
    ConvolutionTest.cpp
    GainTest.cpp
    PannerTest.cpp
    PitchShiftTest.cpp
    ResamplerTest.cpp

    source_tests/NoiseTest.cpp
    source_tests/SineTest.cpp

    filter_tests/ModalFilterTest.cpp
    filter_tests/NthOrderFilterTest.cpp
    filter_tests/ShelfFilterTest.cpp

    simd_tests/SIMDComplexTest.cpp
    simd_tests/SIMDSmoothedValueTest.cpp
    simd_tests/SIMDUtilsTest.cpp

    wdf_tests/OmegaTest.cpp
    wdf_tests/RTypeTest.cpp
    wdf_tests/SIMDWDFTest.cpp
    wdf_tests/StaticWDFTest.cpp
    wdf_tests/WDFTest.cpp
    wdf_tests/WDFPerf.cpp
)
