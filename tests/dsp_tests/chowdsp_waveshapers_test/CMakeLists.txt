setup_catch_test(chowdsp_waveshapers_test MODULES chowdsp_waveshapers)

target_sources(chowdsp_waveshapers_test
    PRIVATE
        SoftClipperTest.cpp
        ADAAFullWaveRectifierTest.cpp
        ADAAHardClipperTest.cpp
        ADAATanhClipperTest.cpp
        ADAASoftClipperTest.cpp
        WestCoastFolderTest.cpp
)
