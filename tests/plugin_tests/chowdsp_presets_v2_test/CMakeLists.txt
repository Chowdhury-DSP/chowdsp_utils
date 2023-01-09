setup_catch_juce_test(chowdsp_presets_v2_test juce::juce_graphics chowdsp::chowdsp_presets_v2)

target_sources(chowdsp_presets_v2_test
    PRIVATE
        PresetTest.cpp
        PresetTreeTest.cpp
        PresetManagerTest.cpp

        NextPreviousTest.cpp
        ProgramAdapterTest.cpp
        TextInterfaceTest.cpp

        TestPresetBinaryData.cpp
)

target_compile_definitions(chowdsp_presets_v2_test
    PRIVATE
        JucePlugin_Name="TestPlugin"
        JucePlugin_VersionString="9.9.9"
)
