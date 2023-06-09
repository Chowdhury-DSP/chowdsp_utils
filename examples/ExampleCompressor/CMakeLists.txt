setup_example_plugin(ExampleCompressor EC0m)

target_link_libraries(ExampleCompressor
    PRIVATE
        juce::juce_dsp
        chowdsp::chowdsp_compressor
        chowdsp::chowdsp_plugin_utils
        chowdsp::chowdsp_plugin_state
        chowdsp::chowdsp_gui
        chowdsp::chowdsp_visualizers
)

target_sources(ExampleCompressor
    PRIVATE
        ExampleCompressor.cpp
        PluginEditor.cpp
)