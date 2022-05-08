juce_add_console_app(chowdsp_presets_test)
setup_juce_test(chowdsp_presets_test)

juce_add_binary_data(chowdsp_presets_test_BinaryData SOURCES
    test_preset.preset
)

target_link_libraries(chowdsp_presets_test PRIVATE
    chowdsp_plugin_base
    chowdsp_presets
    chowdsp_presets_test_BinaryData
)

target_sources(chowdsp_presets_test PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/test_utils/TestRunner.cpp

    PresetTest.cpp
    PresetManagerTest.cpp
)
