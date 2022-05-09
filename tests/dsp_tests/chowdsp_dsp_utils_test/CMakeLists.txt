add_executable(chowdsp_dsp_utils_test)
setup_juce_test(chowdsp_dsp_utils_test)

target_link_libraries(chowdsp_dsp_utils_test PRIVATE
    juce::juce_dsp
    chowdsp_dsp_utils
    chowdsp_plugin_base
)

target_sources(chowdsp_dsp_utils_test PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_utils/TestRunner.cpp

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

    source_tests/NoiseTest.cpp
    source_tests/SineTest.cpp
    source_tests/SawtoothTest.cpp
    source_tests/SquareTest.cpp
    source_tests/RepitchedSourceTest.cpp

    modal_tests/ModalFilterTest.cpp
    modal_tests/ModalFilterBankTest.cpp

    resampling_tests/ResamplerTest.cpp
    resampling_tests/UpsampleDownsampleTest.cpp
    resampling_tests/VariableOversamplingTest.cpp
)
