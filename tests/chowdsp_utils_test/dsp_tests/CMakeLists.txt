setup_juce_unit_test(chowdsp_dsp_unit_tests)

# Test with libsamplerate (if available)
find_package(samplerate)
if (SAMPLERATE_FOUND)
    message(STATUS "libsamplerate found")
    target_link_libraries(chowdsp_dsp_unit_tests PRIVATE samplerate::samplerate)
    target_compile_definitions(chowdsp_dsp_unit_tests PRIVATE CHOWDSP_USE_LIBSAMPLERATE=1)
endif ()

# Test with xsimd (if path is defined)
if (XSIMD_PATH)
    message(STATUS "Using XSIMD")
    target_include_directories(chowdsp_dsp_unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/${XSIMD_PATH}/include)
    target_compile_definitions(chowdsp_dsp_unit_tests PRIVATE CHOWDSP_USE_XSIMD=1)
endif ()

target_sources(chowdsp_dsp_unit_tests PRIVATE
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
