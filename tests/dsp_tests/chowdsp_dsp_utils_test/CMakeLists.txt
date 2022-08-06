setup_catch_test(chowdsp_dsp_utils_test MODULES chowdsp_dsp_utils)

target_sources(chowdsp_dsp_utils_test
    PRIVATE
        AudioTimerTest.cpp
        BypassTest.cpp
        GainTest.cpp
        PannerTest.cpp
        TunerTest.cpp

        BBDTest.cpp
        PitchShiftTest.cpp

        modal_tests/ModalFilterTest.cpp
        modal_tests/ModalFilterBankTest.cpp

        resampling_tests/UpsampleDownsampleTest.cpp
        resampling_tests/ResamplerTest.cpp
)
