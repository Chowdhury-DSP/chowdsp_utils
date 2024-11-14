setup_example_plugin(ForwardingTestPlugin sRvb)

target_link_libraries(ForwardingTestPlugin
    PRIVATE
        juce::juce_dsp
        chowdsp::chowdsp_reverb
        chowdsp::chowdsp_sources
        chowdsp::chowdsp_dsp_utils
        chowdsp::chowdsp_plugin_state
        chowdsp::chowdsp_gui
)

target_sources(ForwardingTestPlugin
    PRIVATE
        ForwardingTestPlugin.cpp
        PluginEditor.cpp

        ../SignalGenerator/SignalGeneratorPlugin.cpp
        ../SimpleReverb/SimpleReverbPlugin.cpp

        InternalPlugins/WernerFilterPlugin.cpp
        InternalPlugins/ARPFilterPlugin.cpp
        InternalPlugins/PolygonalOscPlugin.cpp
        InternalPlugins/BandSplitPlugin.cpp
        InternalPlugins/PlateReverb.cpp
        InternalPlugins/PolyphaseOversamplingPlugin.cpp
)

target_compile_definitions(ForwardingTestPlugin
    PRIVATE
        CHOWDSP_BUILDING_FORWARDING_TEST_PLUGIN=1
)
