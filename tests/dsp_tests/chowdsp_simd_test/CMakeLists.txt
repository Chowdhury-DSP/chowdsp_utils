setup_catch_test(chowdsp_simd_test)

setup_chowdsp_lib(chowdsp_simd_test_lib MODULES chowdsp_core chowdsp_simd)
target_link_libraries(chowdsp_simd_test PRIVATE chowdsp_simd_test_lib)

target_sources(chowdsp_simd_test
    PRIVATE
        SIMDSmoothedValueTest.cpp
        SIMDSpecialMathTest.cpp
)
