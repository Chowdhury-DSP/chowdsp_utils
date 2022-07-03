add_executable(chowdsp_presets_test)
setup_juce_test(chowdsp_presets_test)

if(CHOWDSP_BUILD_LIVE_GUI_TEST)
    juce_add_binary_data(chowdsp_presets_test_BinaryData SOURCES
        test_preset.preset
    )
endif()

target_link_libraries(chowdsp_presets_test PRIVATE
    chowdsp_plugin_base
    chowdsp_presets
)

target_sources(chowdsp_presets_test PRIVATE
    PresetTest.cpp
    PresetManagerTest.cpp

    TestPresetBinaryData.cpp
)
