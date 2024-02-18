setup_catch_lib_test(chowdsp_plugin_utils_test plugin_tests_lib)

target_sources(chowdsp_plugin_utils_test PRIVATE
    AudioFileSaveLoadHelperTest.cpp
    AudioUIBackgroundTaskTest.cpp
    DeferredActionTest.cpp
    FileListenerTest.cpp
    GlobalSettingsTest.cpp
    LNFAllocatorTest.cpp
    UIStateTest.cpp
    TweaksFileTest.cpp
)
