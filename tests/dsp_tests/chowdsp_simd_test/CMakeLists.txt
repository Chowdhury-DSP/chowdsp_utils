add_executable(chowdsp_simd_test)
setup_juce_test(chowdsp_simd_test)

target_link_libraries(chowdsp_simd_test PRIVATE
    juce::juce_dsp
    chowdsp_simd
)

target_sources(chowdsp_simd_test PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_utils/TestRunner.cpp

    SIMDSmoothedValueTest.cpp
    SIMDSpecialMathTest.cpp
)
