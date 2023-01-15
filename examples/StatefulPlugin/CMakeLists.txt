setup_example_plugin(StatefulPlugin S8fl)

target_link_libraries(StatefulPlugin
    PRIVATE
        chowdsp::chowdsp_plugin_state
        chowdsp::chowdsp_plugin_utils
        chowdsp::chowdsp_presets_v2
        chowdsp::chowdsp_gui
)

target_sources(StatefulPlugin
    PRIVATE
        StatefulPlugin.cpp
        PluginEditor.cpp
)

target_compile_definitions(StatefulPlugin
    PUBLIC
        JUCE_MODAL_LOOPS_PERMITTED=1
)
