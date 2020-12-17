target_sources(chowdsp_utils_tests PRIVATE
    test.cpp
    FoleysTest.cpp
    GainTest.cpp
    ModalFilterTest.cpp
    NoiseTest.cpp
    OmegaTest.cpp
    ShelfFilterTest.cpp
    SineTest.cpp
    PitchShiftTest.cpp
    PluginBaseTest.cpp
    WDFTest.cpp
)

target_sources(chowdsp_gui_test PRIVATE
    GUITest.cpp
)
