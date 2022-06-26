setup_example_plugin(ForwardingTestPlugin sRvb)

target_link_libraries(ForwardingTestPlugin
    PRIVATE
        chowdsp_reverb
)

target_sources(ForwardingTestPlugin
    PRIVATE
        ForwardingTestPlugin.cpp
        PluginEditor.cpp

        ../SignalGenerator/SignalGeneratorPlugin.cpp
        ../SimpleReverb/SimpleReverbPlugin.cpp
)

target_compile_definitions(ForwardingTestPlugin
    PRIVATE
        CHOWDSP_BUILDING_FORWARDING_TEST_PLUGIN=1
)
