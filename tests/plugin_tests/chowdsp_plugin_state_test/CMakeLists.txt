add_executable(chowdsp_plugin_state_test)
setup_juce_test(chowdsp_plugin_state_test)

target_link_libraries(chowdsp_plugin_state_test
    PRIVATE
        chowdsp_plugin_state
)

target_sources(chowdsp_plugin_state_test
    PRIVATE
        StateSerializationTest.cpp
        StateListenersTest.cpp
        ParameterAttachmentsTest.cpp
        ParameterPathTest.cpp
)