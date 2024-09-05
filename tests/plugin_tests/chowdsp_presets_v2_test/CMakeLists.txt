setup_catch_lib_test(chowdsp_presets_v2_test plugin_tests_lib)

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
        FileInterfaceTest.cpp

        CLAPPresetsDiscoveryTest.cpp

        TestPresetBinaryData.cpp
)
