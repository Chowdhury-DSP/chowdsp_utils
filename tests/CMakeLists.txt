target_sources(chowdsp_utils_tests PRIVATE
    test.cpp
    FoleysTest.cpp
    PluginBaseTest.cpp
    VersionUtilsTest.cpp
    dsp_tests/GainTest.cpp
    dsp_tests/ModalFilterTest.cpp
    dsp_tests/NoiseTest.cpp
    dsp_tests/NthOrderFilterTest.cpp
    dsp_tests/OmegaTest.cpp
    dsp_tests/ShelfFilterTest.cpp
    dsp_tests/SIMDComplexTest.cpp
    dsp_tests/SIMDSmoothedValueTest.cpp
    dsp_tests/SIMDUtilsTest.cpp
    dsp_tests/SineTest.cpp
    dsp_tests/PannerTest.cpp
    dsp_tests/PitchShiftTest.cpp
    dsp_tests/ResamplerTest.cpp
    dsp_tests/WDFTest.cpp
    dsp_tests/WDFPerf.cpp
)

target_sources(chowdsp_gui_test PRIVATE
    GUITest.cpp
)
