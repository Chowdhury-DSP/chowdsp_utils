add_executable(chowdsp_plugin_utils_test)
setup_juce_test(chowdsp_plugin_utils_test)

target_link_libraries(chowdsp_plugin_utils_test PRIVATE
    juce_dsp
    chowdsp_plugin_utils
    chowdsp_gui
)

target_sources(chowdsp_plugin_utils_test PRIVATE
    AudioFileSaveLoadHelperTest.cpp
    AudioUIBackgroundTaskTest.cpp
    DeferredActionTest.cpp
    FileListenerTest.cpp
    GlobalSettingsTest.cpp
    PluginLoggerTest.cpp
    LNFAllocatorTest.cpp
    UIStateTest.cpp
    TweaksFileTest.cpp
)
