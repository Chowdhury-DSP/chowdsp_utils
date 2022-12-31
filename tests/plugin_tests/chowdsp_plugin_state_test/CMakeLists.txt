setup_catch_juce_test(chowdsp_plugin_state_test chowdsp::chowdsp_plugin_base chowdsp::chowdsp_plugin_state)

target_sources(chowdsp_plugin_state_test
    PRIVATE
        StateSerializationTest.cpp
        StateListenersTest.cpp
        ParameterAttachmentsTest.cpp
        StatePluginInterfaceTest.cpp
)