setup_example_plugin(ForwardingTestPlugin sRvb)

target_link_libraries(ForwardingTestPlugin
    PRIVATE
        juce::juce_dsp
        chowdsp_reverb
)

target_sources(ForwardingTestPlugin
    PRIVATE
        ForwardingTestPlugin.cpp
        PluginEditor.cpp

        ../SignalGenerator/SignalGeneratorPlugin.cpp
        ../SimpleReverb/SimpleReverbPlugin.cpp

        InternalPlugins/WernerFilterPlugin.cpp
)

target_compile_definitions(ForwardingTestPlugin
    PRIVATE
        CHOWDSP_BUILDING_FORWARDING_TEST_PLUGIN=1
)
