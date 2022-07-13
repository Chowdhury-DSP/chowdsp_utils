setup_catch_test(chowdsp_simd_test MODULES chowdsp_simd)

target_sources(chowdsp_simd_test
    PRIVATE
        SIMDSmoothedValueTest.cpp
        SIMDSpecialMathTest.cpp
)
