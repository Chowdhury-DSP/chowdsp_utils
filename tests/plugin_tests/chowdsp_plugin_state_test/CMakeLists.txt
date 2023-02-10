setup_catch_lib_test(chowdsp_plugin_state_test plugin_tests_lib)

target_sources(chowdsp_plugin_state_test
    PRIVATE
        StateSerializationTest.cpp
        StateListenersTest.cpp
        ParameterAttachmentsTest.cpp
        ParamHolderTest.cpp
        StatePluginInterfaceTest.cpp
        VersionStreamingTest.cpp
)
