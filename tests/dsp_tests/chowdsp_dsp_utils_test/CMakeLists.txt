setup_catch_lib_test(chowdsp_dsp_utils_test dsp_tests_lib)

target_sources(chowdsp_dsp_utils_test
    PRIVATE
        AudioTimerTest.cpp
        BypassTest.cpp
        GainTest.cpp
        PannerTest.cpp
        TunerTest.cpp
        LevelDetectorTest.cpp
        OvershootLimiterTest.cpp
        WidthPannerTest.cpp

        BBDTest.cpp
        PitchShiftTest.cpp

        resampling_tests/UpsampleDownsampleTest.cpp
        resampling_tests/ResamplerTest.cpp
)
