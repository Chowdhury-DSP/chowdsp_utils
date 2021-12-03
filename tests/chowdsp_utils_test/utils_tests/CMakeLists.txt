target_sources(chowdsp_utils_tests PRIVATE
    ParamStringsTest.cpp
    PluginBaseTest.cpp
    VersionUtilsTest.cpp
    PresetTest.cpp
    PresetManagerTest.cpp
    PluginLoggerTest.cpp
    ForwardingParameterTest.cpp
    LNFAllocatorTest.cpp
)

juce_add_binary_data(UnitTestData SOURCES
    test_preset.preset
)

target_link_libraries(chowdsp_utils_tests PRIVATE UnitTestData)
