add_executable(chowdsp_simd_test)
setup_juce_test(chowdsp_simd_test)

target_link_libraries(chowdsp_simd_test PRIVATE
    chowdsp_simd
)

target_sources(chowdsp_simd_test PRIVATE
    SIMDSmoothedValueTest.cpp
    SIMDSpecialMathTest.cpp
)
