target_sources(chowdsp_utils_tests PRIVATE
    FileListenerTest.cpp
    ForwardingParameterTest.cpp
    GlobalSettingsTest.cpp
    ParamStringsTest.cpp
    PluginBaseTest.cpp
    PluginLoggerTest.cpp
    PresetManagerTest.cpp
    PresetTest.cpp
    LNFAllocatorTest.cpp
    VersionUtilsTest.cpp
)

juce_add_binary_data(UnitTestData SOURCES
    test_preset.preset
)

target_link_libraries(chowdsp_utils_tests PRIVATE UnitTestData)
