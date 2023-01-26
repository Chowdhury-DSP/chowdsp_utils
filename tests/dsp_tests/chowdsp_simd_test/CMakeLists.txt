setup_catch_lib_test(chowdsp_simd_test dsp_tests_lib)

target_sources(chowdsp_simd_test
    PRIVATE
        SIMDSmoothedValueTest.cpp
        SIMDSpecialMathTest.cpp
        SIMDAlignmentHelpersTest.cpp
)
