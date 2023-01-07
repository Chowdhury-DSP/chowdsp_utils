setup_catch_juce_test(chowdsp_plugin_state_test chowdsp::chowdsp_plugin_base chowdsp::chowdsp_plugin_state)

target_sources(chowdsp_plugin_state_test
    PRIVATE
        StateSerializationTest.cpp
        StateListenersTest.cpp
        ParameterAttachmentsTest.cpp
        ParamHolderTest.cpp
        StatePluginInterfaceTest.cpp
        VersionStreamingTest.cpp
)

target_compile_definitions(chowdsp_plugin_state_test
    PUBLIC
        JucePlugin_VersionString="9.9.9"
)