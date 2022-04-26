target_sources(chowdsp_utils_tests PRIVATE
    AudioFileSaveLoadHelperTest.cpp
    AudioUIBackgroundTaskTest.cpp
    FileListenerTest.cpp
    GlobalSettingsTest.cpp
    JSONTest.cpp
    PluginBaseTest.cpp
    PluginLoggerTest.cpp
    LNFAllocatorTest.cpp
    VersionUtilsTest.cpp

    parameter_tests/ForwardingParameterTest.cpp
    parameter_tests/ParamHelpersTest.cpp
    parameter_tests/ParamStringsTest.cpp
    parameter_tests/RhythmParameterTest.cpp

    presets_tests/PresetManagerTest.cpp
    presets_tests/PresetTest.cpp

    music_theory_tests/RhythmUtilsTest.cpp
)

juce_add_binary_data(UnitTestData SOURCES
    presets_tests/test_preset.preset
)

target_link_libraries(chowdsp_utils_tests PRIVATE UnitTestData)
