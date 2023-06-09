setup_catch_lib_test(chowdsp_compressor_test dsp_tests_lib)

target_sources(chowdsp_compressor_test
    PRIVATE
        GainComputerTest.cpp
        LevelDetectorTest.cpp
        MonoCompressorTest.cpp
)
