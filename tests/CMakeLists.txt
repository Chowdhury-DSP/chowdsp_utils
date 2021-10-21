target_sources(chowdsp_utils_tests PRIVATE
    test.cpp

    dsp_tests/BBDTest.cpp
    dsp_tests/BypassTest.cpp
    dsp_tests/ConvolutionTest.cpp
    dsp_tests/GainTest.cpp
    dsp_tests/PannerTest.cpp
    dsp_tests/PitchShiftTest.cpp
    dsp_tests/ResamplerTest.cpp

    dsp_tests/source_tests/NoiseTest.cpp
    dsp_tests/source_tests/SineTest.cpp
    
    dsp_tests/filter_tests/ModalFilterTest.cpp
    dsp_tests/filter_tests/NthOrderFilterTest.cpp
    dsp_tests/filter_tests/ShelfFilterTest.cpp
    
    dsp_tests/simd_tests/SIMDComplexTest.cpp
    dsp_tests/simd_tests/SIMDSmoothedValueTest.cpp
    dsp_tests/simd_tests/SIMDUtilsTest.cpp
    
    dsp_tests/wdf_tests/OmegaTest.cpp
    dsp_tests/wdf_tests/RTypeTest.cpp
    dsp_tests/wdf_tests/SIMDWDFTest.cpp
    dsp_tests/wdf_tests/StaticWDFTest.cpp
    dsp_tests/wdf_tests/WDFTest.cpp
    dsp_tests/wdf_tests/WDFPerf.cpp

    gui_tests/FoleysTest.cpp
    gui_tests/LNFAllocatorTest.cpp
    gui_tests/PresetsCompTest.cpp

    utils_tests/ParamStringsTest.cpp
    utils_tests/PluginBaseTest.cpp
    utils_tests/VersionUtilsTest.cpp
    utils_tests/PresetTest.cpp
    utils_tests/PresetManagerTest.cpp
)

target_sources(chowdsp_gui_test PRIVATE
    gui_tests/GUITest.cpp
)

target_sources(wdf_test PRIVATE
    wdf_standalone_test.cpp
)

juce_add_binary_data(UnitTestData SOURCES
    utils_tests/test_preset.preset
)
