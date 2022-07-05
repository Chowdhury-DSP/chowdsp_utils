add_executable(chowdsp_waveshapers_test)
setup_juce_test(chowdsp_waveshapers_test)

target_link_libraries(chowdsp_waveshapers_test
    PRIVATE
        chowdsp_waveshapers
)

target_sources(chowdsp_waveshapers_test
    PRIVATE
        SoftClipperTest.cpp
        ADAAHardClipperTest.cpp
        ADAATanhClipperTest.cpp
        ADAASoftClipperTest.cpp
)
