setup_catch_juce_test(chowdsp_presets_v2_test
    juce::juce_graphics chowdsp::chowdsp_presets_v2 chowdsp::chowdsp_plugin_utils
)

target_sources(chowdsp_presets_v2_test
    PRIVATE
        PresetTest.cpp
        PresetTreeTest.cpp
        PresetManagerTest.cpp

        NextPreviousTest.cpp
        ProgramAdapterTest.cpp
        TextInterfaceTest.cpp
        ClipboardInterfaceTest.cpp
        SettingsInterfaceTest.cpp
        MenuInterfaceTest.cpp

        TestPresetBinaryData.cpp
)

target_compile_definitions(chowdsp_presets_v2_test
    PRIVATE
        JucePlugin_Name="TestPlugin"
        JucePlugin_VersionString="9.9.9"
)
