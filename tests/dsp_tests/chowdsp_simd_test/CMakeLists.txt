setup_catch_test(chowdsp_simd_test)

target_sources(chowdsp_simd_test
    PRIVATE
        SIMDSmoothedValueTest.cpp
        SIMDSpecialMathTest.cpp
)
