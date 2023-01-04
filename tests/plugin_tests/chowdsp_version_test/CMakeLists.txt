setup_catch_juce_test(chowdsp_version_test chowdsp::chowdsp_version)

target_sources(chowdsp_version_test
    PRIVATE
        VersionUtilsTest.cpp
)
