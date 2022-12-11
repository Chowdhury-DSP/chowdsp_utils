setup_example_plugin(StatefulPlugin S8fl)

target_link_libraries(StatefulPlugin
    PRIVATE
        chowdsp::chowdsp_plugin_state
        chowdsp::chowdsp_gui
)

target_sources(StatefulPlugin
    PRIVATE
        StatefulPlugin.cpp
        PluginEditor.cpp
)
